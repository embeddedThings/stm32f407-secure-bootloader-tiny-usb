/*
 * drv_gpio.h
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_DRV_GPIO_H_
#define INCLUDE_DRV_GPIO_H_

#include <stdint.h>
#include "stm32f4xx.h"

typedef enum {
    DRV_GPIO_MODE_INPUT  = 0,
    DRV_GPIO_MODE_OUTPUT = 1,
    DRV_GPIO_MODE_AF     = 2,
    DRV_GPIO_MODE_ANALOG = 3
} drv_gpio_mode_t;

typedef enum {
    DRV_GPIO_PULL_NONE = 0,
    DRV_GPIO_PULL_UP   = 1,
    DRV_GPIO_PULL_DOWN = 2
} drv_gpio_pull_t;

void    drv_gpio_config(GPIO_TypeDef *port, uint8_t pin,
                        drv_gpio_mode_t mode, drv_gpio_pull_t pull);
void    drv_gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t level);
void    drv_gpio_toggle(GPIO_TypeDef *port, uint8_t pin);
uint8_t drv_gpio_read(GPIO_TypeDef *port, uint8_t pin);

////////For Tiny-USB/////////
typedef enum {
    DRV_GPIO_SPEED_LOW       = 0,
    DRV_GPIO_SPEED_MEDIUM    = 1,
    DRV_GPIO_SPEED_FAST      = 2,
    DRV_GPIO_SPEED_VERY_HIGH = 3
} drv_gpio_speed_t;

void drv_gpio_set_speed(GPIO_TypeDef *port, uint8_t pin, drv_gpio_speed_t speed);
void drv_gpio_set_af(GPIO_TypeDef *port, uint8_t pin, uint8_t af);


#endif /* INCLUDE_DRV_GPIO_H_ */
