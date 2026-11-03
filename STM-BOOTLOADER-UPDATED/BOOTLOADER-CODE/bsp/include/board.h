/*
 * board.h
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_BOARD_H_
#define INCLUDE_BOARD_H_

#include <stdint.h>

typedef enum {
    BOARD_LED_GREEN = 0,
    BOARD_LED_ORANGE,
    BOARD_LED_RED,
    BOARD_LED_BLUE,
    BOARD_LED_COUNT
} board_led_t;

void    board_init(void);
void    board_led_set(board_led_t led, uint8_t on);
void    board_led_toggle(board_led_t led);
uint8_t board_button_is_pressed(void);

#endif /* INCLUDE_BOARD_H_ */
