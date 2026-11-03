#include "boot_policy.h"
#include "boot_request.h"
#include "boot_shared.h"
#include "drv_iwdg.h"
#include "metadata.h"
#include "image.h"
#include "memmap.h"

swap_status_t boot_policy_run(void)
{
    uint32_t req = boot_request_take();    /* uygulamanin biraktigi istek (varsa) */

    meta_record_t m;
    meta_load(&m);

    if (m.state != META_STATE_TESTING) {
        return SWAP_NOT_NEEDED;
    }

    if (req == BOOT_REQ_CONFIRM) {
        /* Yeni imaj calisti ve kendini onayladi: artik bundan eski surum kabul edilmez */
        const image_header_t *hdr = (const image_header_t *)MEMMAP_SLOT_A_START;
        uint32_t cur = meta_min_version(&m);

        m.min_version = (hdr->fw_version > cur) ? hdr->fw_version : cur;
        m.state       = META_STATE_CONFIRMED;
        m.trial       = 0U;
        return (meta_store(&m) == META_OK) ? SWAP_NOT_NEEDED : SWAP_ERR_FLASH;
    }

    if (m.trial == 0U) {
        /* Yeni imajin ilk acilisi: deneme hakkini kullan, normal ac */
        m.trial = 1U;
        return (meta_store(&m) == META_OK) ? SWAP_NOT_NEEDED : SWAP_ERR_FLASH;
    }

    /* Ikinci acilis ve onay yok: eski imaja don */
    if (slot_swap_request_revert()) {
        return slot_swap_run_if_pending();
    }

    /* Donulecek saglam imaj yok: mevcut imajla devam et, durumu kapat */
    m.state = META_STATE_CONFIRMED;
    return (meta_store(&m) == META_OK) ? SWAP_NOT_NEEDED : SWAP_ERR_FLASH;
}

void boot_policy_prepare_launch(void)
{
    meta_record_t m;
    meta_load(&m);

    boot_request_set_status((m.state == META_STATE_TESTING) ? BOOT_STATUS_TRIAL
                                                            : BOOT_STATUS_NORMAL);
    /* Uygulama calisirken watchdog gozlesin: donarsa reset gelir */
    drv_iwdg_start(BOOT_WDG_TIMEOUT_MS);
}
