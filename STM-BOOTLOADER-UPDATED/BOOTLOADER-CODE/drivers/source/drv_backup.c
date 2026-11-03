/*
 * drv_backup.c
 *
 *  Created on: 26 Eki 2026
 *      Author: ozan
 */

#include "drv_backup.h"
#include "stm32f4xx.h"

/* Yedek register'lara erisim icin guc arayuzu saati ve DBP biti gerekir */
static void enable_access(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    (void)RCC->APB1ENR;
    PWR->CR |= PWR_CR_DBP;
}

void drv_backup_write(uint8_t index, uint32_t value)
{
    if (index >= DRV_BACKUP_COUNT) { return; }

    enable_access();
    (&RTC->BKP0R)[index] = value;
}

uint32_t drv_backup_read(uint8_t index)
{
    if (index >= DRV_BACKUP_COUNT) { return 0U; }

    enable_access();
    return (&RTC->BKP0R)[index];
}
