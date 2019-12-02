#ifndef __HAL_PD_PROXYRCB_HPP__
#define __HAL_PD_PROXYRCB_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/app_redir_shared.h"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_PROXYRCB_HT_SIZE     1024
#define P4PD_HBM_PROXYRCB_ENTRY_SIZE    PROXYRCB_TABLE_ENTRY_SIZE

typedef uint32_t    proxyrcb_hw_id_t;       // same as proxyrcb_id_t
typedef uint64_t    proxyrcb_hw_addr_t;

// proxyrcb pd state
struct pd_proxyrcb_s {
    proxyrcb_t           *proxyrcb;         // PI PROXYR CB

    // operational state of proxyrcb pd
    proxyrcb_hw_addr_t   hw_addr;           // hw address for this proxyrcb
    proxyrcb_hw_id_t     hw_id;

    // meta data maintained for PROXYR CB pd
    ht_ctxt_t          hw_ht_ctxt;          // h/w id based hash table ctxt
} __PACK__;

// allocate a proxyrcb pd instance
static inline pd_proxyrcb_t *
proxyrcb_pd_alloc (void)
{
    pd_proxyrcb_t    *proxyrcb_pd;

    proxyrcb_pd = (pd_proxyrcb_t *)g_hal_state_pd->proxyrcb_slab()->alloc();
    if (proxyrcb_pd == NULL) {
        return NULL;
    }

    return proxyrcb_pd;
}

// initialize a proxyrcb pd instance
static inline pd_proxyrcb_t *
proxyrcb_pd_init (pd_proxyrcb_t *proxyrcb_pd,
                  proxyrcb_hw_id_t hw_id)
{
    if (!proxyrcb_pd) {
        return NULL;
    }
    proxyrcb_pd->hw_id = hw_id;
    proxyrcb_pd->proxyrcb = NULL;

    // initialize meta information
    proxyrcb_pd->hw_ht_ctxt.reset();

    return proxyrcb_pd;
}

// allocate and initialize a proxyrcb pd instance
static inline pd_proxyrcb_t *
proxyrcb_pd_alloc_init (proxyrcb_hw_id_t hw_id)
{
    return proxyrcb_pd_init(proxyrcb_pd_alloc(), hw_id);
}

// free proxyrcb pd instance
static inline hal_ret_t
proxyrcb_pd_free (pd_proxyrcb_t *proxyrcb_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_PROXYRCB_PD, proxyrcb_pd);
    return HAL_RET_OK;
}

// insert proxyrcb pd state in all meta data structures
static inline hal_ret_t
add_proxyrcb_pd_to_db (pd_proxyrcb_t *proxyrcb_pd)
{
    g_hal_state_pd->proxyrcb_hwid_ht()->insert(proxyrcb_pd,
                                               &proxyrcb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_proxyrcb_pd_from_db(pd_proxyrcb_t *proxyrcb_pd)
{
    g_hal_state_pd->proxyrcb_hwid_ht()->remove_entry(proxyrcb_pd,
                                                     &proxyrcb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a proxyrcb pd instance given its hw id
static inline pd_proxyrcb_t *
find_proxyrcb_by_hwid (proxyrcb_hw_id_t hwid)
{
    return (pd_proxyrcb_t *)g_hal_state_pd->proxyrcb_hwid_ht()->lookup(&hwid);
}

extern void *proxyrcb_pd_get_hw_key_func(void *entry);
extern uint32_t proxyrcb_pd_hw_key_size(void);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_PROXYRCB_HPP__

