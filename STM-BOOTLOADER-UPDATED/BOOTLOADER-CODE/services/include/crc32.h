/*
 * crc32.h
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_CRC32_H_
#define INCLUDE_CRC32_H_


#include <stdint.h>

uint32_t crc32_update(uint32_t crc, const uint8_t *data, uint32_t len);
uint32_t crc32_calc(const uint8_t *data, uint32_t len);


#endif /* INCLUDE_CRC32_H_ */
