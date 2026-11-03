/*
 * drv_systick.c
 *
 *  Created on: 21 Eki 2026
 *      Author: ozan
 */

#include "drv_systick.h"
#include "stm32f4xx.h"

static volatile uint32_t s_millis;

void drv_systick_init(uint32_t core_hz)
{
    s_millis = 0U;
    (void)SysTick_Config(core_hz / 1000U);
}

uint32_t drv_systick_millis(void)
{
    return s_millis;
}

void SysTick_Handler(void)
{
    s_millis++;
}
