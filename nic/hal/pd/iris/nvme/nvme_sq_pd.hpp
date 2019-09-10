#ifndef __HAL_PD_NVME_SQ_HPP__
#define __HAL_PD_NVME_SQ_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/plugins/cfg/nvme/nvme_sq.hpp"
#include "nvme_pd.hpp"

using sdk::lib::ht_ctxt_t;
using namespace sdk::platform::capri;

namespace hal {
namespace pd {

#define HAL_MAX_HW_NVME_SQ (64 * 1024)

typedef uint64_t    nvme_sq_hw_id_t;

// nvme_sq pd state
struct pd_nvme_sq_s {
    nvme_sq_t           *nvme_sq;              // PI

    // operational state of nvme_sq pd
    nvme_sq_hw_id_t      hw_id;               // hw id for this nvme_sq

    // meta data maintained for NVME pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a nvme_sq pd instance
static inline pd_nvme_sq_t *
nvme_sq_pd_alloc (void)
{
    pd_nvme_sq_t    *nvme_sq_pd;

    nvme_sq_pd = (pd_nvme_sq_t *)g_hal_state_pd->nvme_sq_slab()->alloc();
    if (nvme_sq_pd == NULL) {
        return NULL;
    }

    return nvme_sq_pd;
}

// initialize a nvme_sq pd instance
static inline pd_nvme_sq_t *
nvme_sq_pd_init (pd_nvme_sq_t *nvme_sq_pd)
{
    if (!nvme_sq_pd) {
        return NULL;
    }
    nvme_sq_pd->nvme_sq = NULL;

    // initialize meta information
    nvme_sq_pd->hw_ht_ctxt.reset();

    return nvme_sq_pd;
}

// allocate and initialize a nvme_sq pd instance
static inline pd_nvme_sq_t *
nvme_sq_pd_alloc_init (void)
{
    return nvme_sq_pd_init(nvme_sq_pd_alloc());
}

// free nvme_sq pd instance
static inline hal_ret_t
nvme_sq_pd_free (pd_nvme_sq_t *nvme_sq_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_NVME_SQ_PD, nvme_sq_pd);
    return HAL_RET_OK;
}

// insert nvme_sq pd state in all meta data structures
static inline hal_ret_t
add_nvme_sq_pd_to_db (pd_nvme_sq_t *nvme_sq_pd)
{
    g_hal_state_pd->nvme_sq_hwid_ht()->insert(nvme_sq_pd, &nvme_sq_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_nvme_sq_pd_from_db(pd_nvme_sq_t *nvme_sq_pd)
{
    g_hal_state_pd->nvme_sq_hwid_ht()->remove(&nvme_sq_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a nvme_sq pd instance given its hw id
static inline pd_nvme_sq_t *
find_nvme_sq_by_hwid (nvme_sq_hw_id_t hwid)
{
    return (pd_nvme_sq_t *)g_hal_state_pd->nvme_sq_hwid_ht()->lookup(&hwid);
}

extern void *nvme_sq_pd_get_hw_key_func(void *entry);
extern uint32_t nvme_sq_pd_hw_key_size(void);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_NVME_SQ_HPP__

