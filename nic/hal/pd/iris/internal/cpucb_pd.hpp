#ifndef __HAL_PD_CPUCB_HPP__
#define __HAL_PD_CPUCB_HPP__

#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_CPUCBS                        4

#define P4PD_CPUCB_STAGE_ENTRY_OFFSET            64
#define P4PD_HBM_CPU_CB_ENTRY_SIZE               128

typedef enum cpucb_hwid_order_ {
    P4PD_HWID_CPU_TX_STAGE0 = 0,
    P4PD_HWID_CPU_RX_STAGE0,
} cpucb_hwid_order_t;

typedef uint64_t    cpucb_hw_id_t;

// cpucb pd state
struct pd_cpucb_s {
    cpucb_t           *cpucb;              // PI CPU CB

    // operational state of cpucb pd
    cpucb_hw_id_t      hw_id;               // hw id for this cpucb

    // meta data maintained for CPU CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a cpucb pd instance
static inline pd_cpucb_t *
cpucb_pd_alloc (void)
{
    pd_cpucb_t    *cpucb_pd;

    cpucb_pd = (pd_cpucb_t *)g_hal_state_pd->cpucb_slab()->alloc();
    if (cpucb_pd == NULL) {
        return NULL;
    }

    return cpucb_pd;
}

// initialize a cpucb pd instance
static inline pd_cpucb_t *
cpucb_pd_init (pd_cpucb_t *cpucb_pd)
{
    if (!cpucb_pd) {
        return NULL;
    }
    cpucb_pd->cpucb = NULL;

    // initialize meta information
    cpucb_pd->hw_ht_ctxt.reset();

    return cpucb_pd;
}

// allocate and initialize a cpucb pd instance
static inline pd_cpucb_t *
cpucb_pd_alloc_init (void)
{
    return cpucb_pd_init(cpucb_pd_alloc());
}

// free cpucb pd instance
static inline hal_ret_t
cpucb_pd_free (pd_cpucb_t *cpucb_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_CPUCB_PD, cpucb_pd);
    return HAL_RET_OK;
}

// insert cpucb pd state in all meta data structures
static inline hal_ret_t
add_cpucb_pd_to_db (pd_cpucb_t *cpucb_pd)
{
    g_hal_state_pd->cpucb_hwid_ht()->insert(cpucb_pd, &cpucb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
del_cpucb_pd_from_db(pd_cpucb_t *cpucb_pd)
{
    g_hal_state_pd->cpucb_hwid_ht()->remove(&cpucb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a cpucb pd instance given its hw id
static inline pd_cpucb_t *
find_cpucb_by_hwid (cpucb_hw_id_t hwid)
{
    return (pd_cpucb_t *)g_hal_state_pd->cpucb_hwid_ht()->lookup(&hwid);
}

extern void *cpucb_pd_get_hw_key_func(void *entry);
extern uint32_t cpucb_pd_hw_key_size(void);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_CPUCB_HPP__

