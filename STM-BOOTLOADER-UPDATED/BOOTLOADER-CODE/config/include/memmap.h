/*
 * memmap.h
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */

#ifndef MEMMAP_H_
#define MEMMAP_H_

/* ---- Flash bolgeleri ---- */
#define MEMMAP_BOOT_START     0x08000000UL   /* sektor 0-3  :  64 KB */
#define MEMMAP_BOOT_SIZE      0x00010000UL

#define MEMMAP_META_START     0x08010000UL   /* sektor 4    :  64 KB */
#define MEMMAP_META_SIZE      0x00010000UL

#define MEMMAP_SLOT_A_START   0x08020000UL   /* sektor 5-7  : 384 KB */
#define MEMMAP_SLOT_A_SIZE    0x00060000UL

#define MEMMAP_SLOT_B_START   0x08080000UL   /* sektor 8-10 : 384 KB */
#define MEMMAP_SLOT_B_SIZE    0x00060000UL

#define MEMMAP_DATA_START     0x080E0000UL   /* sektor 11   : 128 KB */
#define MEMMAP_DATA_SIZE      0x00020000UL

/* ---- SRAM ---- */
#define MEMMAP_SRAM_START     0x20000000UL
#define MEMMAP_SRAM_END       0x20020000UL   /* 128 KB, bitis sonrasi adres */

/* ---- Imaj duzeni: her slotun basinda baslik alani, ardindan uygulama ---- */
#define MEMMAP_IMAGE_HEADER_SIZE   0x00000200UL   /* 512 byte */

#define MEMMAP_SLOT_A_APP_START    (MEMMAP_SLOT_A_START + MEMMAP_IMAGE_HEADER_SIZE)
#define MEMMAP_SLOT_A_APP_SIZE     (MEMMAP_SLOT_A_SIZE  - MEMMAP_IMAGE_HEADER_SIZE)

#define MEMMAP_SLOT_B_APP_START    (MEMMAP_SLOT_B_START + MEMMAP_IMAGE_HEADER_SIZE)
#define MEMMAP_SLOT_B_APP_SIZE     (MEMMAP_SLOT_B_SIZE  - MEMMAP_IMAGE_HEADER_SIZE)

/* Slot A'nin sektor bilgisi (sektor 5-7, hepsi 128 KB) */
#define MEMMAP_SLOT_A_FIRST_SECTOR   5U
#define MEMMAP_SLOT_A_SECTOR_COUNT   3U
#define MEMMAP_SLOT_A_SECTOR_SIZE    0x00020000UL

/*Guvenli Meta Data icin*/
#define MEMMAP_META_SECTOR   4U

/* Slot B (staging): sektor 8-10 */
#define MEMMAP_SLOT_B_FIRST_SECTOR   8U
#define MEMMAP_SLOT_B_SECTOR_COUNT   3U
#define MEMMAP_SLOT_B_SECTOR_SIZE    0x00020000UL

/* Takas icin gecici alan: sektor 11 */
#define MEMMAP_SCRATCH_SECTOR   11U
#define MEMMAP_SCRATCH_START    MEMMAP_DATA_START


#endif /* MEMMAP_H_ */
