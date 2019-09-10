#ifndef __HAL_PD_NVME_CQ_HPP__
#define __HAL_PD_NVME_CQ_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/plugins/cfg/nvme/nvme_cq.hpp"
#include "nvme_pd.hpp"

using sdk::lib::ht_ctxt_t;
using namespace sdk::platform::capri;

namespace hal {
namespace pd {

#define HAL_MAX_HW_NVME_CQ (64 * 1024)

typedef uint64_t    nvme_cq_hw_id_t;

// nvme_cq pd state
struct pd_nvme_cq_s {
    nvme_cq_t           *nvme_cq;              // PI

    // operational state of nvme_cq pd
    nvme_cq_hw_id_t      hw_id;               // hw id for this nvme_cq

    // meta data maintained for NVME pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a nvme_cq pd instance
static inline pd_nvme_cq_t *
nvme_cq_pd_alloc (void)
{
    pd_nvme_cq_t    *nvme_cq_pd;

    nvme_cq_pd = (pd_nvme_cq_t *)g_hal_state_pd->nvme_cq_slab()->alloc();
    if (nvme_cq_pd == NULL) {
        return NULL;
    }

    return nvme_cq_pd;
}

// initialize a nvme_cq pd instance
static inline pd_nvme_cq_t *
nvme_cq_pd_init (pd_nvme_cq_t *nvme_cq_pd)
{
    if (!nvme_cq_pd) {
        return NULL;
    }
    nvme_cq_pd->nvme_cq = NULL;

    // initialize meta information
    nvme_cq_pd->hw_ht_ctxt.reset();

    return nvme_cq_pd;
}

// allocate and initialize a nvme_cq pd instance
static inline pd_nvme_cq_t *
nvme_cq_pd_alloc_init (void)
{
    return nvme_cq_pd_init(nvme_cq_pd_alloc());
}

// free nvme_cq pd instance
static inline hal_ret_t
nvme_cq_pd_free (pd_nvme_cq_t *nvme_cq_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_NVME_CQ_PD, nvme_cq_pd);
    return HAL_RET_OK;
}

// insert nvme_cq pd state in all meta data structures
static inline hal_ret_t
add_nvme_cq_pd_to_db (pd_nvme_cq_t *nvme_cq_pd)
{
    g_hal_state_pd->nvme_cq_hwid_ht()->insert(nvme_cq_pd, &nvme_cq_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_nvme_cq_pd_from_db(pd_nvme_cq_t *nvme_cq_pd)
{
    g_hal_state_pd->nvme_cq_hwid_ht()->remove(&nvme_cq_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a nvme_cq pd instance given its hw id
static inline pd_nvme_cq_t *
find_nvme_cq_by_hwid (nvme_cq_hw_id_t hwid)
{
    return (pd_nvme_cq_t *)g_hal_state_pd->nvme_cq_hwid_ht()->lookup(&hwid);
}

extern void *nvme_cq_pd_get_hw_key_func(void *entry);
extern uint32_t nvme_cq_pd_hw_key_size(void);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_NVME_CQ_HPP__

