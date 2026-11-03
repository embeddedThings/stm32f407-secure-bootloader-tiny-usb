/*
 * drv_iwdg.h
 *
 *  Created on: 26 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_DRV_IWDG_H_
#define INCLUDE_DRV_IWDG_H_

#include <stdint.h>

/* Bagimsiz watchdog'u baslatir. Bir kez baslayinca durdurulamaz.
 * timeout_ms yaklasiktir (LSI osilatoru %50 civari sapabilir), en fazla ~8000 ms. */
void drv_iwdg_start(uint32_t timeout_ms);

/* Sayaci yeniden yukler: "yasiyorum" */
void drv_iwdg_feed(void);

#endif /* INCLUDE_DRV_IWDG_H_ */
