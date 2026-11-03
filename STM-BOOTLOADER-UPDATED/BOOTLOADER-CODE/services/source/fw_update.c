/*
 * fw_update.c
 *
 *  Created on: 21 Eki 2026
 *      Author: ozan
 */

#include "fw_update.h"
#include "drv_flash.h"
#include "image.h"
#include "memmap.h"
#include "metadata.h"

#include "image_sig.h"

/* Indirilen imaj once "staging" alani olan Slot B'ye yazilir */
#define STAGE_START          MEMMAP_SLOT_B_START
#define STAGE_SIZE           MEMMAP_SLOT_B_SIZE
#define STAGE_FIRST_SECTOR   MEMMAP_SLOT_B_FIRST_SECTOR
#define STAGE_SECTOR_COUNT   MEMMAP_SLOT_B_SECTOR_COUNT
#define STAGE_SECTOR_SIZE    MEMMAP_SLOT_B_SECTOR_SIZE

static uint32_t s_erased_bytes;   /* slotun basindan itibaren silinmis miktar */

void fw_update_begin(void)
{
    /* Onceki bir "kurulmayi bekliyor" kaydi varsa gecersiz say: B yeniden yazilacak.
     * meta_store flash kilidini kendi acip kapatir, o yuzden unlock'tan ONCE cagrilir. */
    meta_record_t m;
    meta_load(&m);
    if (m.state == META_STATE_PENDING) {
        m.state = META_STATE_CONFIRMED;
        (void)meta_store(&m);
    }

    s_erased_bytes = 0U;
    drv_flash_unlock();
}

fw_update_status_t fw_update_write(uint32_t offset, const uint8_t *data, uint32_t len)
{
    if ((len == 0U) || (offset > STAGE_SIZE) || (len > (STAGE_SIZE - offset))) {
        return FW_UPDATE_ERR_RANGE;
    }

    /* Yazilacak alan henuz silinmediyse, gereken sektorleri sirayla sil */
    while ((offset + len) > s_erased_bytes) {
        uint32_t idx = s_erased_bytes / STAGE_SECTOR_SIZE;
        if (idx >= STAGE_SECTOR_COUNT) {
            return FW_UPDATE_ERR_RANGE;
        }
        if (drv_flash_erase_sector((uint8_t)(STAGE_FIRST_SECTOR + idx)) != DRV_FLASH_OK) {
            return FW_UPDATE_ERR_ERASE;
        }
        s_erased_bytes += STAGE_SECTOR_SIZE;
    }

    uint32_t addr = STAGE_START + offset;
    uint32_t full = len & ~3U;

    if ((full > 0U) && (drv_flash_write(addr, data, full) != DRV_FLASH_OK)) {
        return FW_UPDATE_ERR_WRITE;
    }
    if ((len & 3U) != 0U) {                       /* 4'un kati olmayan kuyruk: 0xFF ile doldur */
        uint8_t tail[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
        for (uint32_t i = 0U; i < (len & 3U); i++) {
            tail[i] = data[full + i];
        }
        if (drv_flash_write(addr + full, tail, 4U) != DRV_FLASH_OK) {
            return FW_UPDATE_ERR_WRITE;
        }
    }

    /* Geri oku ve karsilastir */
    const uint8_t *p = (const uint8_t *)addr;
    for (uint32_t i = 0U; i < len; i++) {
        if (p[i] != data[i]) {
            return FW_UPDATE_ERR_WRITE;
        }
    }
    return FW_UPDATE_OK;
}

fw_update_status_t fw_update_finish(void)
{
    drv_flash_lock();

    if (!image_is_trusted(STAGE_START, STAGE_SIZE)) {
        return FW_UPDATE_ERR_VERIFY;
    }

    /* Surum dusurme korumasi: onaylanmis surumden eski imaj kabul edilmez */
    const image_header_t *hdr = (const image_header_t *)STAGE_START;
    if (!meta_version_allowed(hdr->fw_version)) {
        return FW_UPDATE_ERR_VERIFY;
    }

    /* Imaj saglam, imzali ve yeterince yeni: "kurulmayi bekliyor" olarak isaretle */
    meta_record_t m;
    meta_load(&m);
    m.state = META_STATE_PENDING;
    return (meta_store(&m) == META_OK) ? FW_UPDATE_OK : FW_UPDATE_ERR_META;
}

void fw_update_abort(void)
{
    drv_flash_lock();
}
