/*
 * tusb_config.h
 *
 *  Created on: 20 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_TUSB_CONFIG_H_
#define INCLUDE_TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Port: OTG FS (rhport 0), F407'de sadece Full Speed var ---- */
#define BOARD_TUD_RHPORT      0
#define BOARD_TUD_MAX_SPEED   OPT_MODE_DEFAULT_SPEED

/* ---- Genel ---- */
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#define CFG_TUSB_OS           OPT_OS_NONE
#define CFG_TUSB_DEBUG        0

#define CFG_TUD_ENABLED       1
#define CFG_TUD_MAX_SPEED     BOARD_TUD_MAX_SPEED

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN    __attribute__ ((aligned(4)))

/* ---- Cihaz ---- */
#define CFG_TUD_ENDPOINT0_SIZE    64

/* ---- Sinif: sadece DFU ---- */
#define CFG_TUD_DFU               1
/* DIKKAT: EP0 buffer boyutundan (64) BUYUK olmali. Esit ya da kucukse TinyUSB
 * DFU verisi icin paylasilan EP0 buffer'ini kullanir ve gelen blogun ilk
 * 6 byte'i GETSTATUS cevabiyla ezilir. */
#define CFG_TUD_DFU_XFER_BUFSIZE  1024

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_TUSB_CONFIG_H_ */
