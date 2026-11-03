/*
 * metadata.c
 *
 *  Created on: 22 Eki 2026
 *      Author: ozan
 */

#include "metadata.h"
#include <string.h>
#include "crc32.h"
#include "drv_flash.h"
#include "memmap.h"

#define META_MAGIC         0x4154454DUL   /* bellekte "META" */
#define META_RECORD_SIZE   32U
#define META_SLOT_COUNT    (MEMMAP_META_SIZE / META_RECORD_SIZE)
#define META_CRC_LEN       (META_RECORD_SIZE - 4U)

static const meta_record_t *slot_ptr(uint32_t i)
{
    return (const meta_record_t *)(MEMMAP_META_START + (i * META_RECORD_SIZE));
}

static int slot_is_erased(const meta_record_t *r)
{
    const uint32_t *w = (const uint32_t *)r;
    for (uint32_t i = 0U; i < (META_RECORD_SIZE / 4U); i++) {
        if (w[i] != 0xFFFFFFFFUL) {
            return 0;
        }
    }
    return 1;
}

static int slot_is_valid(const meta_record_t *r)
{
    return (r->magic == META_MAGIC) &&
           (crc32_calc((const uint8_t *)r, META_CRC_LEN) == r->crc32);
}

/* En son gecerli kaydi ve ilk bos slotu bulur */
static void scan(const meta_record_t **last, uint32_t *free_idx)
{
    *last     = 0;
    *free_idx = META_SLOT_COUNT;              /* bos yer yok varsayimi */

    for (uint32_t i = 0U; i < META_SLOT_COUNT; i++) {
        const meta_record_t *r = slot_ptr(i);

        if (slot_is_erased(r)) {
            *free_idx = i;
            return;
        }
        if (slot_is_valid(r)) {
            *last = r;                         /* yarim/bozuk kayitlar atlanir */
        }
    }
}

void meta_load(meta_record_t *out)
{
    const meta_record_t *last;
    uint32_t free_idx;

    scan(&last, &free_idx);

    if (last != 0) {
        memcpy(out, last, sizeof(*out));
    } else {
        memset(out, 0, sizeof(*out));
        out->state = META_STATE_NONE;
    }
}

meta_status_t meta_store(const meta_record_t *fields)
{
    const meta_record_t *last;
    uint32_t free_idx;

    scan(&last, &free_idx);

    meta_record_t rec;
    memcpy(&rec, fields, sizeof(rec));
    rec.magic = META_MAGIC;
    rec.seq   = (last != 0) ? (last->seq + 1U) : 1U;
    rec.crc32 = crc32_calc((const uint8_t *)&rec, META_CRC_LEN);

    drv_flash_unlock();

    if (free_idx >= META_SLOT_COUNT) {         /* sektor dolu: sil, bastan basla */
        if (drv_flash_erase_sector(MEMMAP_META_SECTOR) != DRV_FLASH_OK) {
            drv_flash_lock();
            return META_ERR_FLASH;
        }
        free_idx = 0U;
    }

    drv_flash_status_t st = drv_flash_write(
        MEMMAP_META_START + (free_idx * META_RECORD_SIZE),
        (const uint8_t *)&rec, META_RECORD_SIZE);

    drv_flash_lock();
    return (st == DRV_FLASH_OK) ? META_OK : META_ERR_FLASH;
}

uint32_t meta_min_version(const meta_record_t *m)
{
    /* Eski kayıtlarda bu alan 0xFFFFFFFF kalmış olabilir: "ayarlanmamış" say */
    return (m->min_version == 0xFFFFFFFFUL) ? 0U : m->min_version;
}

uint8_t meta_version_allowed(uint32_t fw_version)
{
    meta_record_t m;
    meta_load(&m);
    return (fw_version >= meta_min_version(&m)) ? 1U : 0U;
}


