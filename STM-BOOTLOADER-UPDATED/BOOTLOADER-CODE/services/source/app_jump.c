/*
 * app_jump.c
 *
 *  Created on: 16 Eki 2026
 *      Author: ozan
 */


#include "app_jump.h"
#include "memmap.h"
#include "drv_clock.h"
#include "stm32f4xx.h"

int app_jump_is_valid(uint32_t slot_addr, uint32_t slot_size)
{
    uint32_t msp   = *(volatile uint32_t *)slot_addr;
    uint32_t entry = *(volatile uint32_t *)(slot_addr + 4U);

    /* Stack SRAM icinde mi? (bos flash 0xFFFFFFFF burada elenir) */
    if ((msp <= MEMMAP_SRAM_START) || (msp > MEMMAP_SRAM_END)) {
        return 0;
    }
    /* Thumb biti 1 mi? */
    if ((entry & 1U) == 0U) {
        return 0;
    }
    /* Reset_Handler slotun icinde mi? */
    if ((entry < slot_addr) || (entry >= (slot_addr + slot_size))) {
        return 0;
    }
    return 1;
}

void app_jump_start(uint32_t slot_addr)
{
    uint32_t msp   = *(volatile uint32_t *)slot_addr;
    uint32_t entry = *(volatile uint32_t *)(slot_addr + 4U);

    __disable_irq();

    drv_clock_deinit();

    /* SysTick ve tum kesmeleri temizle */
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL  = 0U;
    for (uint32_t i = 0U; i < 8U; i++) {
        NVIC->ICER[i] = 0xFFFFFFFFU;
        NVIC->ICPR[i] = 0xFFFFFFFFU;
    }

    SCB->VTOR = slot_addr;
    __DSB();
    __ISB();

    /* MSP'yi yukle ve atla: ikisi tek asm blogunda */
    __asm volatile (
        "msr msp, %0 \n"
        "bx  %1      \n"
        : : "r" (msp), "r" (entry) :
    );

    for (;;) { }	//death code
}
