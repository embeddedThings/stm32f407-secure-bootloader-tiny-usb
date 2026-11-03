/*
 * drv_flash.h
 *
 *  Created on: 20 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_DRV_FLASH_H_
#define INCLUDE_DRV_FLASH_H_

#include <stdint.h>

typedef enum {
    DRV_FLASH_OK = 0,
    DRV_FLASH_ERR_PARAM,     /* gecersiz adres/boyut/sektor */
    DRV_FLASH_ERR_LOCKED,    /* flash kilitli */
    DRV_FLASH_ERR_TIMEOUT,   /* BSY temizlenmedi */
    DRV_FLASH_ERR_HW         /* flash hata bayragi set */
} drv_flash_status_t;

void               drv_flash_unlock(void);
void               drv_flash_lock(void);
drv_flash_status_t drv_flash_erase_sector(uint8_t sector);   /* 0..11 */
drv_flash_status_t drv_flash_write(uint32_t addr, const uint8_t *data, uint32_t len);

#endif /* INCLUDE_DRV_FLASH_H_ */
