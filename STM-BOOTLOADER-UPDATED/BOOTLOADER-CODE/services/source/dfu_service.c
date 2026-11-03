/*
 * dfu_service.c
 *
 *  Created on: 21 Eki 2026
 *      Author: ozan
 */

#include "dfu_service.h"
#include <string.h>
#include "board.h"
#include "drv_usb.h"
#include "drv_systick.h"
#include "fw_update.h"
#include "memmap.h"
#include "tusb.h"

#include "stm32f4xx.h"      /* NVIC_SystemReset */

/* ---- Donanim baglantilari ---- */
void OTG_FS_IRQHandler(void)
{
    tud_int_handler(0);
}

uint32_t tusb_time_millis_api(void)
{
    return drv_systick_millis();
}

/* ---- Indirme durumu ---- */
static bool     s_pending;                       /* islenmeyi bekleyen blok var mi */
static uint16_t s_block_num;
static uint16_t s_length;
static uint8_t  s_buf[CFG_TUD_DFU_XFER_BUFSIZE];
static uint32_t s_next_block;                    /* sirada bekledigimiz blok no */

#define RESET_DELAY_MS   500U
static bool     s_reset_pending;
static uint32_t s_reset_at;


static uint8_t map_status(fw_update_status_t st)
{
    switch (st) {
    case FW_UPDATE_OK:         return DFU_STATUS_OK;
    case FW_UPDATE_ERR_RANGE:  return DFU_STATUS_ERR_ADDRESS;
    case FW_UPDATE_ERR_ERASE:  return DFU_STATUS_ERR_ERASE;
    case FW_UPDATE_ERR_WRITE:  return DFU_STATUS_ERR_WRITE;
    case FW_UPDATE_ERR_VERIFY: return DFU_STATUS_ERR_VERIFY;
    default:                   return DFU_STATUS_ERR_UNKNOWN;
    }
}

static void process_pending_block(void)
{
    if (!s_pending) {
        return;
    }

    if (s_block_num == 0U) {                     /* yeni oturum */
        board_led_set(BOARD_LED_RED, 0U);
        fw_update_begin();
    }

    fw_update_status_t st = fw_update_write(
        (uint32_t)s_block_num * CFG_TUD_DFU_XFER_BUFSIZE, s_buf, s_length);

    if (st == FW_UPDATE_OK) {
        s_next_block = (uint32_t)s_block_num + 1U;
        board_led_toggle(BOARD_LED_ORANGE);      /* her blokta kirpisir = ilerleme */
    } else {
        fw_update_abort();
        board_led_set(BOARD_LED_RED, 1U);
    }

    s_pending = false;
    tud_dfu_finish_flashing(map_status(st));
}

/* ---- Yasam dongusu ---- */
void dfu_service_init(void)
{
    drv_usb_init();

    tusb_rhport_init_t dev_init = { .role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
}

void dfu_service_task(void)
{
    tud_task();
    process_pending_block();

    if (s_reset_pending &&
        ((int32_t)(drv_systick_millis() - s_reset_at) >= 0)) {
        NVIC_SystemReset();
    }
}

/* ---- USB durum geri cagrilari: yesil LED = PC cihazi taniyip yapilandirdi ---- */
void tud_mount_cb(void)   { board_led_set(BOARD_LED_GREEN, 1U); }
void tud_umount_cb(void)  { board_led_set(BOARD_LED_GREEN, 0U); }

void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;
    board_led_set(BOARD_LED_GREEN, 0U);
}

void tud_resume_cb(void)
{
    board_led_set(BOARD_LED_GREEN, tud_mounted() ? 1U : 0U);
}

/* ---- DFU geri cagrilari ---- */

/* Host'a "bu kadar ms sonra tekrar sor" deriz. Sektor basinda silme olacagi icin uzun. */
uint32_t tud_dfu_get_timeout_cb(uint8_t alt, uint8_t state)
{
    (void)alt;

    if (state == DFU_DNBUSY) {
        uint32_t offset = s_next_block * CFG_TUD_DFU_XFER_BUFSIZE;
        return ((offset % MEMMAP_SLOT_A_SECTOR_SIZE) == 0U) ? 3000U : 5U;
    }
    return 0U;
}

void tud_dfu_download_cb(uint8_t alt, uint16_t block_num,
                         const uint8_t *data, uint16_t length)
{
    (void)alt;

    if ((length > sizeof(s_buf)) || s_pending) {
        tud_dfu_finish_flashing(DFU_STATUS_ERR_UNKNOWN);
        return;
    }

    memcpy(s_buf, data, length);                 /* flash islemi ana donguye */
    s_block_num = block_num;
    s_length    = length;
    s_pending   = true;
}

void tud_dfu_manifest_cb(uint8_t alt)
{
    (void)alt;

    fw_update_status_t st = fw_update_finish();  /* imaj CRC dogrulamasi */
    s_next_block = 0U;
    board_led_set(BOARD_LED_ORANGE, 0U);

    if (st == FW_UPDATE_OK) {
        s_reset_pending = true;                  /* basarili: resetle ve yeni imaji calistir */
        s_reset_at      = drv_systick_millis() + RESET_DELAY_MS;
    } else {
        board_led_set(BOARD_LED_RED, 1U);
    }
    tud_dfu_finish_flashing(map_status(st));
}

uint16_t tud_dfu_upload_cb(uint8_t alt, uint16_t block_num,
                           uint8_t *data, uint16_t length)
{
    (void)alt; (void)block_num; (void)data; (void)length;
    return 0;                                    /* geri okuma kapali */
}

void tud_dfu_abort_cb(uint8_t alt)
{
    (void)alt;
    fw_update_abort();
    s_pending    = false;
    s_next_block = 0U;
    board_led_set(BOARD_LED_ORANGE, 0U);
}

void tud_dfu_detach_cb(void)
{
}
