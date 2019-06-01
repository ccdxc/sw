#ifndef __HAL_PD_NVME_SESSCB_HPP__
#define __HAL_PD_NVME_SESSCB_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/plugins/cfg/nvme/nvme_sesscb.hpp"
#include "nvme_pd.hpp"

using sdk::lib::ht_ctxt_t;
using namespace sdk::platform::capri;

namespace hal {
namespace pd {

#define HAL_MAX_HW_NVME_SESSCBS 2048

typedef uint64_t    nvme_sesscb_hw_id_t;

// nvme_sesscb pd state
struct pd_nvme_sesscb_s {
    nvme_sesscb_t           *nvme_sesscb;              // PI TCP CB

    // operational state of nvme_sesscb pd
    nvme_sesscb_hw_id_t      hw_id;               // hw id for this nvme_sesscb
    nvme_sesscb_hw_id_t      hw_id_qtype1;        // hw id for qtype 1

    // meta data maintained for TCP CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a nvme_sesscb pd instance
static inline pd_nvme_sesscb_t *
nvme_sesscb_pd_alloc (void)
{
    pd_nvme_sesscb_t    *nvme_sesscb_pd;

    nvme_sesscb_pd = (pd_nvme_sesscb_t *)g_hal_state_pd->nvme_sesscb_slab()->alloc();
    if (nvme_sesscb_pd == NULL) {
        return NULL;
    }

    return nvme_sesscb_pd;
}

// initialize a nvme_sesscb pd instance
static inline pd_nvme_sesscb_t *
nvme_sesscb_pd_init (pd_nvme_sesscb_t *nvme_sesscb_pd)
{
    if (!nvme_sesscb_pd) {
        return NULL;
    }
    nvme_sesscb_pd->nvme_sesscb = NULL;

    // initialize meta information
    nvme_sesscb_pd->hw_ht_ctxt.reset();

    return nvme_sesscb_pd;
}

// allocate and initialize a nvme_sesscb pd instance
static inline pd_nvme_sesscb_t *
nvme_sesscb_pd_alloc_init (void)
{
    return nvme_sesscb_pd_init(nvme_sesscb_pd_alloc());
}

// free nvme_sesscb pd instance
static inline hal_ret_t
nvme_sesscb_pd_free (pd_nvme_sesscb_t *nvme_sesscb_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_NVME_SESSCB_PD, nvme_sesscb_pd);
    return HAL_RET_OK;
}

// insert nvme_sesscb pd state in all meta data structures
static inline hal_ret_t
add_nvme_sesscb_pd_to_db (pd_nvme_sesscb_t *nvme_sesscb_pd)
{
    g_hal_state_pd->nvme_sesscb_hwid_ht()->insert(nvme_sesscb_pd, &nvme_sesscb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_nvme_sesscb_pd_from_db(pd_nvme_sesscb_t *nvme_sesscb_pd)
{
    g_hal_state_pd->nvme_sesscb_hwid_ht()->remove(&nvme_sesscb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a nvme_sesscb pd instance given its hw id
static inline pd_nvme_sesscb_t *
find_nvme_sesscb_by_hwid (nvme_sesscb_hw_id_t hwid)
{
    return (pd_nvme_sesscb_t *)g_hal_state_pd->nvme_sesscb_hwid_ht()->lookup(&hwid);
}

extern void *nvme_sesscb_pd_get_hw_key_func(void *entry);
extern uint32_t nvme_sesscb_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool nvme_sesscb_pd_compare_hw_key_func(void *key1, void *key2);
extern uint64_t nvme_sesscb_pd_serq_prod_ci_addr_get(uint32_t g_sess_id);


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_NVME_SESSCB_HPP__

