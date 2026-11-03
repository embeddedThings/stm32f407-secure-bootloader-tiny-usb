/*
 * drv_flash.c
 *
 *  Created on: 20 Eki 2026
 *      Author: ozan
 */

#include "drv_flash.h"
#include "stm32f4xx.h"
#include "drv_systick.h"

#define FLASH_KEY1          0x45670123UL
#define FLASH_KEY2          0xCDEF89ABUL
#define FLASH_LAST_ADDR     0x080FFFFFUL
#define FLASH_SECTOR_COUNT  12U
#define FLASH_CR_SNB_SHIFT  3U

#define FLASH_ERASE_TIMEOUT_MS    4000U    /* 128 KB sektor silme: en fazla ~2 sn */
#define FLASH_PROGRAM_TIMEOUT_MS  50U

/* RM0090'da OPERR olan bit CMSIS'te SOP adini tasiyor */
#define FLASH_ERR_MASK  (FLASH_SR_SOP | FLASH_SR_WRPERR | FLASH_SR_PGAERR | \
                         FLASH_SR_PGPERR | FLASH_SR_PGSERR)

static drv_flash_status_t wait_ready(uint32_t timeout_ms)
{
    uint32_t start = drv_systick_millis();

    while ((FLASH->SR & FLASH_SR_BSY) != 0U) {
        if ((drv_systick_millis() - start) > timeout_ms) {
            return DRV_FLASH_ERR_TIMEOUT;
        }
    }
    if ((FLASH->SR & FLASH_ERR_MASK) != 0U) {
        FLASH->SR = FLASH_ERR_MASK;          /* 1 yazarak temizle */
        return DRV_FLASH_ERR_HW;
    }
    return DRV_FLASH_OK;
}

/* Silme/yazma sonrasi cache'te bayat veri kalmasin */
static void flush_caches(void)
{
    uint32_t acr = FLASH->ACR;

    if ((acr & FLASH_ACR_ICEN) != 0U) {
        FLASH->ACR &= ~FLASH_ACR_ICEN;
        FLASH->ACR |= FLASH_ACR_ICRST;
        FLASH->ACR &= ~FLASH_ACR_ICRST;
        FLASH->ACR |= FLASH_ACR_ICEN;
    }
    if ((acr & FLASH_ACR_DCEN) != 0U) {
        FLASH->ACR &= ~FLASH_ACR_DCEN;
        FLASH->ACR |= FLASH_ACR_DCRST;
        FLASH->ACR &= ~FLASH_ACR_DCRST;
        FLASH->ACR |= FLASH_ACR_DCEN;
    }
}

void drv_flash_unlock(void)
{
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

void drv_flash_lock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}

drv_flash_status_t drv_flash_erase_sector(uint8_t sector)
{
    if (sector >= FLASH_SECTOR_COUNT) {
        return DRV_FLASH_ERR_PARAM;
    }
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U) {
        return DRV_FLASH_ERR_LOCKED;
    }

    drv_flash_status_t st = wait_ready(FLASH_PROGRAM_TIMEOUT_MS);
    if (st != DRV_FLASH_OK) {
        return st;
    }

    FLASH->CR &= ~(FLASH_CR_PSIZE | FLASH_CR_SNB);
    FLASH->CR |= FLASH_CR_PSIZE_1 | FLASH_CR_SER |
                 ((uint32_t)sector << FLASH_CR_SNB_SHIFT);
    FLASH->CR |= FLASH_CR_STRT;

    st = wait_ready(FLASH_ERASE_TIMEOUT_MS);
    FLASH->CR &= ~(FLASH_CR_SER | FLASH_CR_SNB);

    flush_caches();

    return st;
}

drv_flash_status_t drv_flash_write(uint32_t addr, const uint8_t *data, uint32_t len)
{
    if ((data == 0) || ((addr & 3U) != 0U) || ((len & 3U) != 0U) ||
        (addr < FLASH_BASE) || (addr > FLASH_LAST_ADDR) ||
        (len > (FLASH_LAST_ADDR + 1UL - addr))) {
        return DRV_FLASH_ERR_PARAM;
    }
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U) {
        return DRV_FLASH_ERR_LOCKED;
    }

    drv_flash_status_t st = wait_ready(FLASH_PROGRAM_TIMEOUT_MS);
    if (st != DRV_FLASH_OK) {
        return st;
    }

    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_CR_PSIZE_1 | FLASH_CR_PG;

    for (uint32_t i = 0U; i < len; i += 4U) {
        uint32_t w = (uint32_t)data[i]
                   | ((uint32_t)data[i + 1U] << 8)
                   | ((uint32_t)data[i + 2U] << 16)
                   | ((uint32_t)data[i + 3U] << 24);
        *(volatile uint32_t *)(addr + i) = w;

        st = wait_ready(FLASH_PROGRAM_TIMEOUT_MS);
        if (st != DRV_FLASH_OK) {
            break;
        }
    }

    FLASH->CR &= ~FLASH_CR_PG;

    flush_caches();

    return st;
}
