/*
 * drv_systick.h
 *
 *  Created on: 21 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_DRV_SYSTICK_H_
#define INCLUDE_DRV_SYSTICK_H_

#include <stdint.h>

void     drv_systick_init(uint32_t core_hz);   /* 1 ms'lik tick baslatir */
uint32_t drv_systick_millis(void);

#endif /* INCLUDE_DRV_SYSTICK_H_ */
