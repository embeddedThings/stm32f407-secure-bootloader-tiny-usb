/*
 * boot_request.h
 *
 *  Created on: 26 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_BOOT_REQUEST_H_
#define INCLUDE_BOOT_REQUEST_H_

#include <stdint.h>

/* Uygulamanin biraktigi istegi okur ve siler (BOOT_REQ_* degerlerinden biri) */
uint32_t boot_request_take(void);

/* Uygulamaya "nasil acildin" bilgisini birakir (BOOT_STATUS_* degerlerinden biri) */
void boot_request_set_status(uint32_t status);

#endif /* INCLUDE_BOOT_REQUEST_H_ */
