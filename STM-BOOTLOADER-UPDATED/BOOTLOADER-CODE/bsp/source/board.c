/*
 * board.c
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */

//this sheet was designed according to STM32F407vgt discovery board

#include "board.h"
#include "drv_gpio.h"

typedef struct {
    GPIO_TypeDef *port;
    uint8_t       pin;
} board_pin_t;

static const board_pin_t s_leds[BOARD_LED_COUNT] = {
    [BOARD_LED_GREEN]  = { GPIOD, 12U },
    [BOARD_LED_ORANGE] = { GPIOD, 13U },
    [BOARD_LED_RED]    = { GPIOD, 14U },
    [BOARD_LED_BLUE]   = { GPIOD, 15U },
};

static const board_pin_t s_button = { GPIOA, 0U };

void board_init(void)
{
    for (uint32_t i = 0; i < BOARD_LED_COUNT; i++) {
        drv_gpio_config(s_leds[i].port, s_leds[i].pin,
                        DRV_GPIO_MODE_OUTPUT, DRV_GPIO_PULL_NONE);
        drv_gpio_write(s_leds[i].port, s_leds[i].pin, 0U);
    }
    drv_gpio_config(s_button.port, s_button.pin,
                    DRV_GPIO_MODE_INPUT, DRV_GPIO_PULL_NONE);
}

void board_led_set(board_led_t led, uint8_t on)
{
    if (led >= BOARD_LED_COUNT) { return; }
    drv_gpio_write(s_leds[led].port, s_leds[led].pin, on);
}

void board_led_toggle(board_led_t led)
{
    if (led >= BOARD_LED_COUNT) { return; }
    drv_gpio_toggle(s_leds[led].port, s_leds[led].pin);
}

uint8_t board_button_is_pressed(void)
{
    return drv_gpio_read(s_button.port, s_button.pin);
}
