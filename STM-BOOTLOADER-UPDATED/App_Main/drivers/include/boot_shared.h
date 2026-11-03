/*
 * boot_shared.h
 *
 *  Created on: 26 Eki 2026
 *      Author: ozan
 */

#ifndef INCLUDE_BOOT_SHARED_H_
#define INCLUDE_BOOT_SHARED_H_


/* Uygulama ile bootloader arasindaki ortak sozlesme.
 * Her iki proje de bu dosyayi kullanir; degisirse ikisi birden guncellenmeli.
 * Iletisim RTC yedek register'lari uzerinden yapilir (sistem reset'inde silinmezler). */

/* Uygulama -> bootloader: istek */
#define BOOT_REQ_BKP_INDEX      0U
#define BOOT_REQ_NONE           0x00000000UL
#define BOOT_REQ_CONFIRM        0xC0F1A5EDUL   /* "imaj saglikli, onayliyorum" */

/* Bootloader -> uygulama: nasil acildin */
#define BOOT_STATUS_BKP_INDEX   1U
#define BOOT_STATUS_NORMAL      0x00000000UL
#define BOOT_STATUS_TRIAL       0x7121A100UL   /* yeni imaj deneniyor, onay bekleniyor */

#endif /* INCLUDE_BOOT_SHARED_H_ */
