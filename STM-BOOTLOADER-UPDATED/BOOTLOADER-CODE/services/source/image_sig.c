/*
 * image_sig.c
 *
 *  Created on: 31 Eki 2026
 *      Author: ozan
 */

#include "image_sig.h"
#include "image.h"
#include "sha256.h"
#include "memmap.h"
#include "uECC.h"
#include "boot_public_key.h"

#define SIGNED_FIELDS_SIZE   32U     /* baslikta imzalanan kisim: image_header_t */
#define SIG_OFFSET           0x20U   /* imza basligin hemen arkasinda */
#define SIG_SIZE             64U     /* r || s */

image_sig_status_t image_sig_verify(uint32_t slot_addr, uint32_t slot_size)
{
    const image_header_t *hdr  = (const image_header_t *)slot_addr;
    const uint8_t        *sig  = (const uint8_t *)(slot_addr + SIG_OFFSET);
    const uint8_t        *body = (const uint8_t *)(slot_addr + MEMMAP_IMAGE_HEADER_SIZE);
    uint32_t max_body = slot_size - MEMMAP_IMAGE_HEADER_SIZE;

    if ((hdr->image_size == 0U) || (hdr->image_size > max_body)) {
        return IMAGE_SIG_ERR_FORMAT;
    }

    /* Imza alani tamamen 0xFF ise imzasiz imajdir */
    uint32_t blank = 0U;
    for (uint32_t i = 0U; i < SIG_SIZE; i++) {
        if (sig[i] == 0xFFU) { blank++; }
    }
    if (blank == SIG_SIZE) {
        return IMAGE_SIG_ERR_MISSING;
    }

    /* Ozet: SHA-256( baslik[0..31] || govde ) */
    sha256_ctx_t ctx;
    uint8_t hash[SHA256_DIGEST_SIZE];

    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)hdr, SIGNED_FIELDS_SIZE);
    sha256_update(&ctx, body, hdr->image_size);
    sha256_final(&ctx, hash);

    if (uECC_verify(BOOT_PUBLIC_KEY, hash, SHA256_DIGEST_SIZE, sig, uECC_secp256r1()) != 1) {
        return IMAGE_SIG_ERR_INVALID;
    }
    return IMAGE_SIG_OK;
}

uint8_t image_is_trusted(uint32_t slot_addr, uint32_t slot_size)
{
    if (image_verify(slot_addr, slot_size) != IMAGE_OK) {
        return 0U;
    }
    return (image_sig_verify(slot_addr, slot_size) == IMAGE_SIG_OK) ? 1U : 0U;
}
