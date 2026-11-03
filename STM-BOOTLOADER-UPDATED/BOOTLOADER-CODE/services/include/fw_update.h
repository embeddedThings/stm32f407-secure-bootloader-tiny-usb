/*
 * fw_update.h
 *
 *  Created on: 21 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_FW_UPDATE_H_
#define INCLUDE_FW_UPDATE_H_

#include <stdint.h>

typedef enum {
    FW_UPDATE_OK = 0,
    FW_UPDATE_ERR_RANGE,    /* imaj slota sigmiyor */
    FW_UPDATE_ERR_ERASE,
    FW_UPDATE_ERR_WRITE,
    FW_UPDATE_ERR_VERIFY,    /* imaj dogrulamasi (CRC vb.) basarisiz */
    FW_UPDATE_ERR_META      /* metadata kaydi yazilamadi */
} fw_update_status_t;


void               fw_update_begin(void);    /* oturumu baslatir, flash kilidini acar */
fw_update_status_t fw_update_write(uint32_t offset, const uint8_t *data, uint32_t len);
fw_update_status_t fw_update_finish(void);   /* kilidi kapatir, imaji dogrular */
void               fw_update_abort(void);    /* kilidi kapatir */


#endif /* INCLUDE_FW_UPDATE_H_ */
