/*
 * image_sig.h
 *
 *  Created on: 31 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_IMAGE_SIG_H_
#define INCLUDE_IMAGE_SIG_H_

#include <stdint.h>

typedef enum {
    IMAGE_SIG_OK = 0,
    IMAGE_SIG_ERR_FORMAT,     /* boyut alani mantiksiz */
    IMAGE_SIG_ERR_MISSING,    /* imza alani bos (0xFF): imzasiz imaj */
    IMAGE_SIG_ERR_INVALID     /* imza var ama bu anahtarla dogrulanmadi */
} image_sig_status_t;

/* slot_addr: slotun basi (baslik burada), slot_size: slotun toplam boyutu.
 * Imza, SHA-256( baslik[0..31] + govde ) uzerinden ECDSA P-256 ile dogrulanir. */
image_sig_status_t image_sig_verify(uint32_t slot_addr, uint32_t slot_size);

/* CRC ve imza birlikte gecerliyse 1, degilse 0 */
uint8_t image_is_trusted(uint32_t slot_addr, uint32_t slot_size);

#endif /* INCLUDE_IMAGE_SIG_H_ */
