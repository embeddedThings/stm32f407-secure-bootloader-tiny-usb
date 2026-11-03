/*
 * app_jump.h
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_APP_JUMP_H_
#define INCLUDE_APP_JUMP_H_

#include <stdint.h>

int  app_jump_is_valid(uint32_t slot_addr, uint32_t slot_size);
void app_jump_start(uint32_t slot_addr);   /* geri donmez */

#endif /* INCLUDE_APP_JUMP_H_ */
