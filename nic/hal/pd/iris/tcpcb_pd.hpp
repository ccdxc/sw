#ifndef __HAL_PD_TCPCB_HPP__
#define __HAL_PD_TCPCB_HPP__

#include <base.h>
#include <ht.hpp>
#include <hal_state_pd.hpp>

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_TCPCBS                        2048

typedef uint64_t    tcpcb_hw_id_t;

// tcpcb pd state
struct pd_tcpcb_s {
    tcpcb_t           *tcpcb;              // PI TCP CB

    // operational state of tcpcb pd
    tcpcb_hw_id_t      hw_id;               // hw id for this tcpcb

    // meta data maintained for TCP CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a tcpcb pd instance
static inline pd_tcpcb_t *
tcpcb_pd_alloc (void)
{
    pd_tcpcb_t    *tcpcb_pd;

    tcpcb_pd = (pd_tcpcb_t *)g_hal_state_pd->tcpcb_slab()->alloc();
    if (tcpcb_pd == NULL) {
        return NULL;
    }

    return tcpcb_pd;
}

// initialize a tcpcb pd instance
static inline pd_tcpcb_t *
tcpcb_pd_init (pd_tcpcb_t *tcpcb_pd)
{
    if (!tcpcb_pd) {
        return NULL;
    }
    tcpcb_pd->tcpcb = NULL;

    // initialize meta information
    tcpcb_pd->hw_ht_ctxt.reset();

    return tcpcb_pd;
}

// allocate and initialize a tcpcb pd instance
static inline pd_tcpcb_t *
tcpcb_pd_alloc_init (void)
{
    return tcpcb_pd_init(tcpcb_pd_alloc());
}

// free tcpcb pd instance
static inline hal_ret_t
tcpcb_pd_free (pd_tcpcb_t *tcpcb_pd)
{
    g_hal_state_pd->tcpcb_slab()->free(tcpcb_pd);
    return HAL_RET_OK;
}

// insert tcpcb pd state in all meta data structures
static inline hal_ret_t
add_tcpcb_pd_to_db (pd_tcpcb_t *tcpcb_pd)
{
    g_hal_state_pd->tcpcb_hwid_ht()->insert(tcpcb_pd, &tcpcb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a tcpcb pd instance given its hw id
static inline pd_tcpcb_t *
find_tcpcb_by_hwid (tcpcb_hw_id_t hwid)
{
    return (pd_tcpcb_t *)g_hal_state_pd->tcpcb_hwid_ht()->lookup(&hwid);
}

extern void *tcpcb_pd_get_hw_key_func(void *entry);
extern uint32_t tcpcb_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool tcpcb_pd_compare_hw_key_func(void *key1, void *key2);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_TCPCB_HPP__

