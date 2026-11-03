/*
 * drv_usb.c
 *
 *  Created on: 21 Eki 2026
 *      Author: ozan
 */

#include "drv_usb.h"
#include "drv_gpio.h"
#include "stm32f4xx.h"

#define USB_AF   10U     /* PA11/PA12 icin AF10 = OTG_FS */

void drv_usb_init(void)
{
    /* PA11 = DM, PA12 = DP */
    drv_gpio_config(GPIOA, 11U, DRV_GPIO_MODE_AF, DRV_GPIO_PULL_NONE);
    drv_gpio_set_speed(GPIOA, 11U, DRV_GPIO_SPEED_VERY_HIGH);
    drv_gpio_set_af(GPIOA, 11U, USB_AF);

    drv_gpio_config(GPIOA, 12U, DRV_GPIO_MODE_AF, DRV_GPIO_PULL_NONE);
    drv_gpio_set_speed(GPIOA, 12U, DRV_GPIO_SPEED_VERY_HIGH);
    drv_gpio_set_af(GPIOA, 12U, USB_AF);

    /* OTG FS cevre birimi saati */
    RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
    (void)RCC->AHB2ENR;

    /* VBUS algilama kapali: bus gucu her zaman gecerli sayilir (ilk bring-up icin) */
    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
    USB_OTG_FS->GCCFG &= ~(USB_OTG_GCCFG_VBUSASEN | USB_OTG_GCCFG_VBUSBSEN);
}
