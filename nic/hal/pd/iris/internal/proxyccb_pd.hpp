#ifndef __HAL_PD_PROXYCCB_HPP__
#define __HAL_PD_PROXYCCB_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/app_redir_shared.h"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_PROXYCCB_HT_SIZE               1024
#define P4PD_HBM_PROXYCCB_ENTRY_SIZE              PROXYCCB_TABLE_ENTRY_SIZE

typedef uint32_t    proxyccb_hw_id_t;       // same as proxyccb_id_t
typedef uint64_t    proxyccb_hw_addr_t;

// proxyccb pd state
struct pd_proxyccb_s {
    proxyccb_t           *proxyccb;         // PI PROXYC CB

    // operational state of proxyccb pd
    proxyccb_hw_addr_t   hw_addr;           // hw address for this proxyccb
    proxyccb_hw_id_t     hw_id;

    // meta data maintained for PROXYC CB pd
    ht_ctxt_t          hw_ht_ctxt;          // h/w id based hash table ctxt
} __PACK__;

// allocate a proxyccb pd instance
static inline pd_proxyccb_t *
proxyccb_pd_alloc (void)
{
    pd_proxyccb_t    *proxyccb_pd;

    proxyccb_pd = (pd_proxyccb_t *)g_hal_state_pd->proxyccb_slab()->alloc();
    if (proxyccb_pd == NULL) {
        return NULL;
    }

    return proxyccb_pd;
}

// initialize a proxyccb pd instance
static inline pd_proxyccb_t *
proxyccb_pd_init (pd_proxyccb_t *proxyccb_pd,
                  proxyccb_hw_id_t hw_id)
{
    if (!proxyccb_pd) {
        return NULL;
    }
    proxyccb_pd->hw_id = hw_id;
    proxyccb_pd->proxyccb = NULL;

    // initialize meta information
    proxyccb_pd->hw_ht_ctxt.reset();

    return proxyccb_pd;
}

// allocate and initialize a proxyccb pd instance
static inline pd_proxyccb_t *
proxyccb_pd_alloc_init (proxyccb_hw_id_t hw_id)
{
    return proxyccb_pd_init(proxyccb_pd_alloc(), hw_id);
}

// free proxyccb pd instance
static inline hal_ret_t
proxyccb_pd_free (pd_proxyccb_t *proxyccb_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_PROXYCCB_PD, proxyccb_pd);
    return HAL_RET_OK;
}

// insert proxyccb pd state in all meta data structures
static inline hal_ret_t
add_proxyccb_pd_to_db (pd_proxyccb_t *proxyccb_pd)
{
    g_hal_state_pd->proxyccb_hwid_ht()->insert(proxyccb_pd,
                                               &proxyccb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_proxyccb_pd_from_db(pd_proxyccb_t *proxyccb_pd)
{
    g_hal_state_pd->proxyccb_hwid_ht()->remove_entry(proxyccb_pd,
                                                     &proxyccb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a proxyccb pd instance given its hw id
static inline pd_proxyccb_t *
find_proxyccb_by_hwid (proxyccb_hw_id_t hwid)
{
    return (pd_proxyccb_t *)g_hal_state_pd->proxyccb_hwid_ht()->lookup(&hwid);
}

extern void *proxyccb_pd_get_hw_key_func(void *entry);
extern uint32_t proxyccb_pd_hw_key_size(void);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_PROXYCCB_HPP__

