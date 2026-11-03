/*
 * drv_iwdg.c
 *
 *  Created on: 26 Eki 2026
 *      Author: ozan
 */

#include "drv_iwdg.h"
#include "stm32f4xx.h"

#define IWDG_KEY_RELOAD    0xAAAAUL
#define IWDG_KEY_UNLOCK    0x5555UL
#define IWDG_KEY_START     0xCCCCUL

#define IWDG_PRESCALER_64  4UL       /* LSI ~32 kHz / 64 = ~500 Hz -> 1 sayim = ~2 ms */
#define IWDG_MS_PER_TICK   2UL
#define IWDG_RELOAD_MAX    4095UL

#define IWDG_SYNC_LOOPS    100000UL  /* register senkronu icin ust sinir */

void drv_iwdg_start(uint32_t timeout_ms)
{
    uint32_t reload = timeout_ms / IWDG_MS_PER_TICK;

    if (reload < 1UL)               { reload = 1UL; }
    if (reload > IWDG_RELOAD_MAX)   { reload = IWDG_RELOAD_MAX; }

    IWDG->KR = IWDG_KEY_START;       /* baslat (LSI otomatik acilir) */
    IWDG->KR = IWDG_KEY_UNLOCK;      /* PR ve RLR yazilabilsin */
    IWDG->PR = IWDG_PRESCALER_64;
    IWDG->RLR = reload - 1UL;

    /* PR/RLR LSI alanina senkronlanana kadar bekle (sinirli: takilirsa watchdog zaten resetler) */
    uint32_t n = IWDG_SYNC_LOOPS;
    while ((IWDG->SR != 0UL) && (n-- > 0UL)) { }

    IWDG->KR = IWDG_KEY_RELOAD;
}

void drv_iwdg_feed(void)
{
    IWDG->KR = IWDG_KEY_RELOAD;
}
