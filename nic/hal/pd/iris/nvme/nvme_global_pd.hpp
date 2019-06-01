#ifndef __HAL_PD_NVME_GLOBAL_HPP__
#define __HAL_PD_NVME_GLOBAL_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/plugins/cfg/nvme/nvme_global.hpp"
#include "nvme_pd.hpp"

using sdk::lib::ht_ctxt_t;
using namespace sdk::platform::capri;

namespace hal {
namespace pd {


//nvme global PD state
struct pd_nvme_global_s {
    nvme_global_t           *nvme_global;              // PI TCP CB

    uint64_t nscb_base_addr;
    uint32_t cur_sess;
    uint64_t txsessprodcb_base;
    uint64_t rxsessprodcb_base;
    uint64_t tx_sess_xtsq_base;
    uint64_t tx_sess_dgstq_base;
    uint64_t rx_sess_xtsq_base;
    uint64_t rx_sess_dgstq_base;
    uint64_t sess_bitmap_addr;
    uint64_t cmd_context_page_base;
    uint64_t cmd_context_ring_base;
    uint64_t tx_pdu_context_page_base;
    uint64_t tx_pdu_context_ring_base;
    uint64_t tx_nmdpr_ring_base;
    uint64_t tx_nmdpr_ring_size;
    uint64_t rx_pdu_context_page_base;
    uint64_t rx_pdu_context_ring_base;
    uint64_t rx_nmdpr_ring_base;
    uint64_t rx_nmdpr_ring_size;
    uint64_t resourcecb_addr;
    uint64_t tx_hwxtscb_addr;
    uint64_t rx_hwxtscb_addr;
    uint64_t tx_hwdgstcb_addr;
    uint64_t rx_hwdgstcb_addr;
    uint64_t tx_xts_aol_array_addr;
    uint64_t tx_xts_iv_array_addr;
    uint64_t rx_xts_aol_array_addr;
    uint64_t rx_xts_iv_array_addr;
} __PACK__ ;

// allocate a nvme_global pd instance
static inline pd_nvme_global_t *
nvme_global_pd_alloc (void)
{
    pd_nvme_global_t    *nvme_global_pd;

    nvme_global_pd = (pd_nvme_global_t *)g_hal_state_pd->nvme_global_slab()->alloc();
    if (nvme_global_pd == NULL) {
        return NULL;
    }

    return nvme_global_pd;
}

// initialize a nvme_global pd instance
static inline pd_nvme_global_t *
nvme_global_pd_init (pd_nvme_global_t *nvme_global_pd)
{
    if (!nvme_global_pd) {
        return NULL;
    }
    nvme_global_pd->nvme_global = NULL;

    return nvme_global_pd;
}

// allocate and initialize a nvme_global pd instance
static inline pd_nvme_global_t *
nvme_global_pd_alloc_init (void)
{
    return nvme_global_pd_init(nvme_global_pd_alloc());
}

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_NVME_GLOBAL_HPP__

