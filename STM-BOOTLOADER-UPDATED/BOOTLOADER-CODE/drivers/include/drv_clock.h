/*
 * drv_clock.h
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */

#ifndef DRV_CLOCK_H_
#define DRV_CLOCK_H_

#include <stdint.h>

typedef enum {
    DRV_CLOCK_OK = 0,
    DRV_CLOCK_ERR_HSE,      /* kristal hazir olmadi */
    DRV_CLOCK_ERR_PLL,      /* PLL kilitlenmedi */
    DRV_CLOCK_ERR_SWITCH    /* PLL'e gecis yapilamadi */
} drv_clock_status_t;

drv_clock_status_t drv_clock_init(void);
uint32_t           drv_clock_get_sysclk_hz(void);

//reset sonrası standart duruma return icin.
void drv_clock_deinit(void);

#endif /* DRV_CLOCK_H_ */
