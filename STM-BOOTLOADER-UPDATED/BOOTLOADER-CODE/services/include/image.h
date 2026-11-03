/*
 * image.h
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_IMAGE_H_
#define INCLUDE_IMAGE_H_

#include <stdint.h>

#define IMAGE_HEADER_MAGIC     0x544F4F42UL   /* bellekte "BOOT" */
#define IMAGE_HEADER_VERSION   1U

typedef struct {
    uint32_t magic;
    uint32_t header_version;
    uint32_t image_size;     /* govde boyutu (byte), 4'un kati */
    uint32_t image_crc32;    /* govdenin CRC-32'si */
    uint32_t fw_version;
    uint32_t reserved[3];
} image_header_t;            /* toplam 32 byte */

typedef enum {
    IMAGE_OK = 0,
    IMAGE_ERR_MAGIC,
    IMAGE_ERR_HEADER_VERSION,
    IMAGE_ERR_SIZE,
    IMAGE_ERR_CRC
} image_status_t;

/* slot_addr: slotun basi (baslik burada), slot_size: slotun toplam boyutu */
image_status_t image_verify(uint32_t slot_addr, uint32_t slot_size);

#endif /* INCLUDE_IMAGE_H_ */
