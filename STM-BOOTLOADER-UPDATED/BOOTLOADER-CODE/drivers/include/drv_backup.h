/*
 * drv_backup.h
 *
 *  Created on: 26 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_DRV_BACKUP_H_
#define INCLUDE_DRV_BACKUP_H_

#include <stdint.h>

#define DRV_BACKUP_COUNT   20U     /* RTC yedek register sayisi */

void     drv_backup_write(uint8_t index, uint32_t value);
uint32_t drv_backup_read(uint8_t index);

#endif /* INCLUDE_DRV_BACKUP_H_ */
