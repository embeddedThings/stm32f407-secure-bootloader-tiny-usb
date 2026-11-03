/*
 * sha256.h
 *
 *  Created on: 31 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_SHA256_H_
#define INCLUDE_SHA256_H_

#include <stdint.h>
#include <stddef.h>

#define SHA256_DIGEST_SIZE   32U

typedef struct {
    uint32_t state[8];
    uint64_t bit_count;
    uint8_t  block[64];
    uint32_t block_len;
} sha256_ctx_t;

void sha256_init(sha256_ctx_t *ctx);
void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len);
void sha256_final(sha256_ctx_t *ctx, uint8_t out[SHA256_DIGEST_SIZE]);

#endif /* INCLUDE_SHA256_H_ */
