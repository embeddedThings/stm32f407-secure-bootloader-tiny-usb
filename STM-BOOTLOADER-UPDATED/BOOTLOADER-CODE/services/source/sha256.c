/*
 * sha256.c
 *
 *  Created on: 31 Eki 2026
 *      Author: ozan
 */

#include "sha256.h"

static const uint32_t K[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
    0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
    0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL, 0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
    0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
    0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
    0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
    0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL, 0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

#define ROTR(x, n)  (((x) >> (n)) | ((x) << (32U - (n))))

static void compress(sha256_ctx_t *ctx, const uint8_t *p)
{
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t i;

    for (i = 0U; i < 16U; i++) {
        w[i] = ((uint32_t)p[4U * i] << 24) | ((uint32_t)p[4U * i + 1U] << 16) |
               ((uint32_t)p[4U * i + 2U] << 8) | (uint32_t)p[4U * i + 3U];
    }
    for (i = 16U; i < 64U; i++) {
        uint32_t s0 = ROTR(w[i - 15U], 7) ^ ROTR(w[i - 15U], 18) ^ (w[i - 15U] >> 3);
        uint32_t s1 = ROTR(w[i - 2U], 17) ^ ROTR(w[i - 2U], 19) ^ (w[i - 2U] >> 10);
        w[i] = w[i - 16U] + s0 + w[i - 7U] + s1;
    }

    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    for (i = 0U; i < 64U; i++) {
        uint32_t S1  = ROTR(e, 6) ^ ROTR(e, 11) ^ ROTR(e, 25);
        uint32_t ch  = (e & f) ^ ((~e) & g);
        uint32_t t1  = h + S1 + ch + K[i] + w[i];
        uint32_t S0  = ROTR(a, 2) ^ ROTR(a, 13) ^ ROTR(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2  = S0 + maj;

        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void sha256_init(sha256_ctx_t *ctx)
{
    ctx->state[0] = 0x6a09e667UL; ctx->state[1] = 0xbb67ae85UL;
    ctx->state[2] = 0x3c6ef372UL; ctx->state[3] = 0xa54ff53aUL;
    ctx->state[4] = 0x510e527fUL; ctx->state[5] = 0x9b05688cUL;
    ctx->state[6] = 0x1f83d9abUL; ctx->state[7] = 0x5be0cd19UL;
    ctx->bit_count = 0U;
    ctx->block_len = 0U;
}

void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len)
{
    size_t i;

    for (i = 0U; i < len; i++) {
        ctx->block[ctx->block_len++] = data[i];
        if (ctx->block_len == 64U) {
            compress(ctx, ctx->block);
            ctx->block_len = 0U;
        }
    }
    ctx->bit_count += (uint64_t)len * 8U;
}

void sha256_final(sha256_ctx_t *ctx, uint8_t out[SHA256_DIGEST_SIZE])
{
    uint64_t bits = ctx->bit_count;
    uint32_t i;

    ctx->block[ctx->block_len++] = 0x80U;
    if (ctx->block_len > 56U) {
        while (ctx->block_len < 64U) { ctx->block[ctx->block_len++] = 0U; }
        compress(ctx, ctx->block);
        ctx->block_len = 0U;
    }
    while (ctx->block_len < 56U) { ctx->block[ctx->block_len++] = 0U; }

    for (i = 0U; i < 8U; i++) {
        ctx->block[56U + i] = (uint8_t)(bits >> (56U - 8U * i));
    }
    compress(ctx, ctx->block);

    for (i = 0U; i < 8U; i++) {
        out[4U * i]      = (uint8_t)(ctx->state[i] >> 24);
        out[4U * i + 1U] = (uint8_t)(ctx->state[i] >> 16);
        out[4U * i + 2U] = (uint8_t)(ctx->state[i] >> 8);
        out[4U * i + 3U] = (uint8_t)(ctx->state[i]);
    }
}
