#ifndef __HAL_PD_NVME_NS_HPP__
#define __HAL_PD_NVME_NS_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/plugins/cfg/nvme/nvme_ns.hpp"
#include "nvme_pd.hpp"

using sdk::lib::ht_ctxt_t;
using namespace sdk::platform::capri;

namespace hal {
namespace pd {

#define HAL_MAX_HW_NVME_NS 2048

typedef uint64_t    nvme_ns_hw_id_t;

// nvme_ns pd state
struct pd_nvme_ns_s {
    nvme_ns_t           *nvme_ns;              // PI

    // operational state of nvme_ns pd
    nvme_ns_hw_id_t      hw_id;               // hw id for this nvme_ns

    // meta data maintained for NVME pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a nvme_ns pd instance
static inline pd_nvme_ns_t *
nvme_ns_pd_alloc (void)
{
    pd_nvme_ns_t    *nvme_ns_pd;

    nvme_ns_pd = (pd_nvme_ns_t *)g_hal_state_pd->nvme_ns_slab()->alloc();
    if (nvme_ns_pd == NULL) {
        return NULL;
    }

    return nvme_ns_pd;
}

// initialize a nvme_ns pd instance
static inline pd_nvme_ns_t *
nvme_ns_pd_init (pd_nvme_ns_t *nvme_ns_pd)
{
    if (!nvme_ns_pd) {
        return NULL;
    }
    nvme_ns_pd->nvme_ns = NULL;

    // initialize meta information
    nvme_ns_pd->hw_ht_ctxt.reset();

    return nvme_ns_pd;
}

// allocate and initialize a nvme_ns pd instance
static inline pd_nvme_ns_t *
nvme_ns_pd_alloc_init (void)
{
    return nvme_ns_pd_init(nvme_ns_pd_alloc());
}

// free nvme_ns pd instance
static inline hal_ret_t
nvme_ns_pd_free (pd_nvme_ns_t *nvme_ns_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_NVME_NS_PD, nvme_ns_pd);
    return HAL_RET_OK;
}

// insert nvme_ns pd state in all meta data structures
static inline hal_ret_t
add_nvme_ns_pd_to_db (pd_nvme_ns_t *nvme_ns_pd)
{
    g_hal_state_pd->nvme_ns_hwid_ht()->insert(nvme_ns_pd, &nvme_ns_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_nvme_ns_pd_from_db(pd_nvme_ns_t *nvme_ns_pd)
{
    g_hal_state_pd->nvme_ns_hwid_ht()->remove(&nvme_ns_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a nvme_ns pd instance given its hw id
static inline pd_nvme_ns_t *
find_nvme_ns_by_hwid (nvme_ns_hw_id_t hwid)
{
    return (pd_nvme_ns_t *)g_hal_state_pd->nvme_ns_hwid_ht()->lookup(&hwid);
}

extern void *nvme_ns_pd_get_hw_key_func(void *entry);
extern uint32_t nvme_ns_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool nvme_ns_pd_compare_hw_key_func(void *key1, void *key2);

hal_ret_t
nvme_ns_update_session_id(uint32_t g_nsid, uint16_t sess_id);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_NVME_NS_HPP__

