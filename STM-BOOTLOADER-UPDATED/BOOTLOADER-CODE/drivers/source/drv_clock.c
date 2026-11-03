/*
 * drv_clock.c
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */

#include "drv_clock.h"
#include "stm32f4xx.h"

#define CLOCK_WAIT_LIMIT   100000U

#define PLL_M   8U      /* 8 MHz / 8   = 1 MHz   */
#define PLL_N   336U    /* 1 MHz * 336 = 336 MHz */
#define PLL_Q   7U      /* 336 / 7     = 48 MHz (USB) */
                        /* PLL_P = /2  -> 168 MHz (SYSCLK) */

#define SYSCLK_HZ  168000000UL


#define HSI_HZ     16000000UL

/* CMSIS'in bekledigi global: TinyUSB gibi kutuphaneler HCLK'yi buradan okur */
uint32_t SystemCoreClock = HSI_HZ;   /* reset sonrasi HSI */


/* Maskeden herhangi bir bit 1 olana kadar bekler; zaman asiminda 0 doner */
static int wait_for_set(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t n = CLOCK_WAIT_LIMIT;
    while (((*reg & mask) == 0U) && (n > 0U)) {
        n--;
    }
    return ((*reg & mask) != 0U);
}

drv_clock_status_t drv_clock_init(void)
{
    /* 1. HSE */
    RCC->CR |= RCC_CR_HSEON;
    if (!wait_for_set(&RCC->CR, RCC_CR_HSERDY)) {
        return DRV_CLOCK_ERR_HSE;
    }

    /* 2. Guc arayuzu + flash bekleme suresi (hizlanmadan once) */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR      |= PWR_CR_VOS;
    FLASH->ACR    = FLASH_ACR_LATENCY_5WS | FLASH_ACR_PRFTEN |
                    FLASH_ACR_ICEN | FLASH_ACR_DCEN;

    /* 3. Bus bolucileri: AHB /1, APB1 /4, APB2 /2 */
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2))
              | RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2;

    /* 4. PLL ayari (kaynak HSE, P = /2 -> alan degeri 0) */
    RCC->PLLCFGR = (PLL_M << RCC_PLLCFGR_PLLM_Pos)
                 | (PLL_N << RCC_PLLCFGR_PLLN_Pos)
                 | (0U    << RCC_PLLCFGR_PLLP_Pos)
                 | RCC_PLLCFGR_PLLSRC_HSE
                 | (PLL_Q << RCC_PLLCFGR_PLLQ_Pos);

    /* 5. PLL'i ac */
    RCC->CR |= RCC_CR_PLLON;
    if (!wait_for_set(&RCC->CR, RCC_CR_PLLRDY)) {
        return DRV_CLOCK_ERR_PLL;
    }

    /* 6. Sistem saatini PLL'e gecir ve dogrula */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;

    uint32_t n = CLOCK_WAIT_LIMIT;
    while (((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) && (n > 0U)) {
        n--;
    }
    if ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
        return DRV_CLOCK_ERR_SWITCH;
    }

    SystemCoreClock = SYSCLK_HZ;

    return DRV_CLOCK_OK;
}

uint32_t drv_clock_get_sysclk_hz(void)
{
	return SystemCoreClock;
}

void drv_clock_deinit(void)
{
    /* HSI'ya don ve gectigini dogrula */
    RCC->CR |= RCC_CR_HSION;
    (void)wait_for_set(&RCC->CR, RCC_CR_HSIRDY);

    RCC->CFGR &= ~RCC_CFGR_SW;
    uint32_t n = CLOCK_WAIT_LIMIT;
    while (((RCC->CFGR & RCC_CFGR_SWS) != 0U) && (n > 0U)) {
        n--;
    }

    /* Her seyi reset degerlerine dondur */
    RCC->CFGR = 0U;
    RCC->CR  &= ~(RCC_CR_PLLON | RCC_CR_HSEON);
    RCC->PLLCFGR = 0x24003010U;   /* PLLCFGR reset degeri */
    FLASH->ACR = 0U;

    SystemCoreClock = HSI_HZ;
}

