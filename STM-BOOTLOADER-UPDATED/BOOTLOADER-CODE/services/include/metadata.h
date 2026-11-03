/*
 * metadata.h
 *
 *  Created on: 22 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_METADATA_H_
#define INCLUDE_METADATA_H_

#include <stdint.h>

typedef enum {
    META_STATE_NONE = 0,        /* kayit yok / varsayilan */
    META_STATE_CONFIRMED = 1,   /* calisan imaj onayli */
    META_STATE_PENDING = 2,     /* B'de yeni imaj var, kurulacak (Adim 9) */
    META_STATE_TESTING = 3,      /* yeni imaj kuruldu, onay bekliyor (Adim 9) */
	META_STATE_SWAPPING = 4,     /* takas yapiliyor (swap_step ilerlemeyi tutar) */
	META_STATE_REVERTING = 5    /* geri donus takasi yapiliyor */
} meta_state_t;

typedef enum {
    META_OK = 0,
    META_ERR_FLASH
} meta_status_t;

typedef struct {
    uint32_t magic;
    uint32_t seq;
    uint32_t state;
    uint32_t boot_count;
    uint32_t swap_step;
    uint32_t trial;        /* TESTING durumunda yeni imajla yapilan acilis sayisi */
    uint32_t min_version;  /* sürüm düşürme koruması: bundan eski fw_version kabul edilmez */
    uint32_t crc32;
} meta_record_t;                /* 32 byte */

/* En son gecerli kaydi okur; kayit yoksa sifirlanmis varsayilan doner */
void          meta_load(meta_record_t *out);

/* Verilen alanlarla yeni bir kayit ekler (magic, seq, crc'yi modul doldurur) */
meta_status_t meta_store(const meta_record_t *fields);

/* Geçerli en düşük sürüm (silinmemiş/boş alan 0 sayılır) */
uint32_t meta_min_version(const meta_record_t *m);

/* 1: bu sürüm kabul edilebilir (min_version'dan küçük değil), 0: reddedilmeli */
uint8_t meta_version_allowed(uint32_t fw_version);

#endif /* INCLUDE_METADATA_H_ */
