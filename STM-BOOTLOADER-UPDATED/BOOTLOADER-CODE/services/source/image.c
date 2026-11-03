/*
 * image.c
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */

#include "image.h"
#include "crc32.h"
#include "memmap.h"

image_status_t image_verify(uint32_t slot_addr, uint32_t slot_size)
{
    const image_header_t *hdr  = (const image_header_t *)slot_addr;
    const uint8_t        *body = (const uint8_t *)(slot_addr + MEMMAP_IMAGE_HEADER_SIZE);
    uint32_t max_body = slot_size - MEMMAP_IMAGE_HEADER_SIZE;

    if (hdr->magic != IMAGE_HEADER_MAGIC) {
        return IMAGE_ERR_MAGIC;
    }
    if (hdr->header_version != IMAGE_HEADER_VERSION) {
        return IMAGE_ERR_HEADER_VERSION;
    }
    if ((hdr->image_size == 0U) ||
        (hdr->image_size > max_body) ||
        ((hdr->image_size & 3U) != 0U)) {
        return IMAGE_ERR_SIZE;
    }
    if (crc32_calc(body, hdr->image_size) != hdr->image_crc32) {
        return IMAGE_ERR_CRC;
    }
    return IMAGE_OK;
}
