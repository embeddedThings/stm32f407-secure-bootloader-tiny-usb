/*
 * boot_policy.h
 *
 *  Created on: 23 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_BOOT_POLICY_H_
#define INCLUDE_BOOT_POLICY_H_

#include "slot_swap.h"

/* Acilista TESTING durumunu degerlendirir. Yeni imajin ilk acilisinda deneme hakkini
 * kaydeder; onaysiz ikinci acilista eski imaja geri doner. */
swap_status_t boot_policy_run(void);

/* Uygulamaya atlamadan hemen once cagrilir: uygulamaya "deneme mi, normal mi" bilgisini birakir */
void boot_policy_prepare_launch(void);

#endif /* INCLUDE_BOOT_POLICY_H_ */
