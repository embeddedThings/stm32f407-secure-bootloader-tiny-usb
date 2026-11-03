/*
 * drv_gpio.c
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */


#include "drv_gpio.h"

/* Portlar 0x400 aralikla dizili: GPIOA=0, GPIOB=1, ... */
static void enable_port_clock(GPIO_TypeDef *port)
{
    uint32_t idx = ((uint32_t)port - (uint32_t)GPIOA) / 0x400U;
    RCC->AHB1ENR |= (1U << idx);
    (void)RCC->AHB1ENR;   /* saatin oturmasi icin geri oku */
}

void drv_gpio_config(GPIO_TypeDef *port, uint8_t pin,
                     drv_gpio_mode_t mode, drv_gpio_pull_t pull)
{
    if (pin > 15U) { return; }

    enable_port_clock(port);

    uint32_t shift = (uint32_t)pin * 2U;
    port->MODER = (port->MODER & ~(3U << shift)) | ((uint32_t)mode << shift);
    port->PUPDR = (port->PUPDR & ~(3U << shift)) | ((uint32_t)pull << shift);
}

void drv_gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t level)
{
    if (pin > 15U) { return; }

    if (level) {
        port->BSRR = (1U << pin);           /* alt 16 bit: set */
    } else {
        port->BSRR = (1U << (pin + 16U));   /* ust 16 bit: reset */
    }
}

void drv_gpio_toggle(GPIO_TypeDef *port, uint8_t pin)
{
    if (pin > 15U) { return; }
    drv_gpio_write(port, pin, (port->ODR & (1U << pin)) ? 0U : 1U);
}

uint8_t drv_gpio_read(GPIO_TypeDef *port, uint8_t pin)
{
    if (pin > 15U) { return 0U; }
    return (uint8_t)((port->IDR >> pin) & 1U);
}
