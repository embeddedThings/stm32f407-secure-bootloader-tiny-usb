/*
 * boot_request.c
 *
 *  Created on: 26 Eki 2026
 *      Author: ozan
 */

#include "boot_request.h"
#include "boot_shared.h"
#include "drv_backup.h"

uint32_t boot_request_take(void)
{
    uint32_t req = drv_backup_read(BOOT_REQ_BKP_INDEX);

    if (req != BOOT_REQ_NONE) {
        drv_backup_write(BOOT_REQ_BKP_INDEX, BOOT_REQ_NONE);
    }
    return req;
}

void boot_request_set_status(uint32_t status)
{
    drv_backup_write(BOOT_STATUS_BKP_INDEX, status);
}
