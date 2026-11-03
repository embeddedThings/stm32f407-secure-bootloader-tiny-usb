/*
 * slot_swap.c
 *
 *  Created on: 23 Eki 2026
 *      Author: ozan
 */

#include "slot_swap.h"
#include <string.h>
#include "board.h"
#include "drv_flash.h"
#include "image.h"
#include "memmap.h"
#include "metadata.h"

#include "image_sig.h"

_Static_assert(MEMMAP_SLOT_A_SECTOR_COUNT == MEMMAP_SLOT_B_SECTOR_COUNT,
               "Slot A ve B ayni sektor sayisinda olmali");

#define SECTOR_SIZE     MEMMAP_SLOT_A_SECTOR_SIZE
#define SWAP_SECTORS    MEMMAP_SLOT_A_SECTOR_COUNT
#define PHASES          3U
#define STEPS_TOTAL     (SWAP_SECTORS * PHASES)
#define COPY_CHUNK      1024U

static uint8_t s_chunk[COPY_CHUNK];

/* dst_sector'u siler, src_addr'daki bir sektoru dst_addr'a kopyalar.
 * Her parcayi yazdiktan sonra geri okuyup karsilastirir. 1 = basarili. */
static int erase_and_copy(uint8_t dst_sector, uint32_t dst_addr, uint32_t src_addr)
{
    int ok = 0;

    drv_flash_unlock();

    if (drv_flash_erase_sector(dst_sector) == DRV_FLASH_OK) {
        ok = 1;
        for (uint32_t off = 0U; off < SECTOR_SIZE; off += COPY_CHUNK) {
            memcpy(s_chunk, (const void *)(src_addr + off), COPY_CHUNK);

            if (drv_flash_write(dst_addr + off, s_chunk, COPY_CHUNK) != DRV_FLASH_OK) {
                ok = 0;
                break;
            }
            if (memcmp((const void *)(dst_addr + off), s_chunk, COPY_CHUNK) != 0) {
                ok = 0;
                break;
            }
        }
    }

    drv_flash_lock();
    return ok;
}

static int run_step(uint32_t step)
{
    uint32_t i     = step / PHASES;          /* hangi sektor cifti */
    uint32_t phase = step % PHASES;
    uint32_t off   = i * SECTOR_SIZE;

    switch (phase) {
    case 0:   /* eski A -> scratch */
        return erase_and_copy(MEMMAP_SCRATCH_SECTOR, MEMMAP_SCRATCH_START,
                              MEMMAP_SLOT_A_START + off);
    case 1:   /* B (yeni imaj) -> A */
        return erase_and_copy((uint8_t)(MEMMAP_SLOT_A_FIRST_SECTOR + i),
                              MEMMAP_SLOT_A_START + off,
                              MEMMAP_SLOT_B_START + off);
    default:  /* scratch (eski A) -> B */
        return erase_and_copy((uint8_t)(MEMMAP_SLOT_B_FIRST_SECTOR + i),
                              MEMMAP_SLOT_B_START + off,
                              MEMMAP_SCRATCH_START);
    }
}

swap_status_t slot_swap_run_if_pending(void)
{
    meta_record_t m;
    meta_load(&m);

    if (m.state == META_STATE_PENDING) {
        const image_header_t *hdr = (const image_header_t *)MEMMAP_SLOT_B_START;

        /* Yeni imaj (B) saglam, imzali ve yeterince yeni mi? */
        if ((!image_is_trusted(MEMMAP_SLOT_B_START, MEMMAP_SLOT_B_SIZE)) ||
            (hdr->fw_version < meta_min_version(&m))) {
            m.state = META_STATE_CONFIRMED;
            (void)meta_store(&m);
            return SWAP_ERR_IMAGE;
        }
        m.state     = META_STATE_SWAPPING;
        m.swap_step = 0U;
        if (meta_store(&m) != META_OK) {
            return SWAP_ERR_FLASH;
        }
    }

    if ((m.state != META_STATE_SWAPPING) && (m.state != META_STATE_REVERTING)) {
        return SWAP_NOT_NEEDED;
    }

    /* Normal takas bitince yeni imaj denenir (TESTING), geri donus bitince is kapanir (CONFIRMED) */
    uint32_t final_state = (m.state == META_STATE_REVERTING) ? META_STATE_CONFIRMED
                                                             : META_STATE_TESTING;

    while (m.swap_step < STEPS_TOTAL) {
        if (!run_step(m.swap_step)) {
            return SWAP_ERR_FLASH;
        }
        m.swap_step++;
        if (meta_store(&m) != META_OK) {
            return SWAP_ERR_FLASH;
        }
        board_led_toggle(BOARD_LED_ORANGE);   /* her adimda LED degisir = ilerleme */
    }

    m.state     = final_state;
    m.swap_step = 0U;
    m.trial     = 0U;
    board_led_set(BOARD_LED_ORANGE, 0U);
    return (meta_store(&m) == META_OK) ? SWAP_DONE : SWAP_ERR_FLASH;
}

int slot_swap_request_revert(void)
{
    if (!image_is_trusted(MEMMAP_SLOT_B_START, MEMMAP_SLOT_B_SIZE)) {
        return 0;
    }
    meta_record_t m;
    meta_load(&m);
    m.state     = META_STATE_REVERTING;
    m.swap_step = 0U;
    return (meta_store(&m) == META_OK);
}
