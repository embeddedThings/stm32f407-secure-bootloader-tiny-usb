/*
 * slot_swap.h
 *
 *  Created on: 23 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_SLOT_SWAP_H_
#define INCLUDE_SLOT_SWAP_H_

typedef enum {
    SWAP_NOT_NEEDED = 0,   /* bekleyen ya da yarim kalmis takas yok */
    SWAP_DONE,             /* takas tamamlandi */
    SWAP_ERR_IMAGE,        /* B'deki imaj dogrulanamadi, takas iptal edildi */
    SWAP_ERR_FLASH         /* flash/metadata hatasi (sonraki acilista tekrar denenir) */
} swap_status_t;

/* PENDING veya yarim kalmis SWAPPING durumunu isler. main() tarafindan
 * her acilista, uygulamaya atlamadan ONCE cagrilir. */
swap_status_t slot_swap_run_if_pending(void);

/* Onaylanmamis yeni imajdan eski imaja donus ister (B'deki eski imaj saglamsa).
 * 1: istek kaydedildi, ardindan slot_swap_run_if_pending() cagrilmali.
 * 0: donus yapilamaz. */
int slot_swap_request_revert(void);

#endif /* INCLUDE_SLOT_SWAP_H_ */
