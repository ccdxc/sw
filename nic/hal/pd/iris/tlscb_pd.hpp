#ifndef __HAL_PD_TLSCB_HPP__
#define __HAL_PD_TLSCB_HPP__

#include <base.h>
#include <ht.hpp>
#include <pd.hpp>
#include <hal_state_pd.hpp>

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_TLSCBS                        2048
#define P4PD_HBM_TCP_CB_ENTRY_SIZE               512

typedef uint32_t    tlscb_hw_id_t;

// tlscb pd state
struct pd_tlscb_s {
    tlscb_t           *tlscb;              // PI TLS CB

    // operational state of tlscb pd
    tlscb_hw_id_t      hw_id;               // hw id for this tlscb

    // meta data maintained for TLS CB pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a tlscb pd instance
static inline pd_tlscb_t *
tlscb_pd_alloc (void)
{
    pd_tlscb_t    *tlscb_pd;

    tlscb_pd = (pd_tlscb_t *)g_hal_state_pd->tlscb_slab()->alloc();
    if (tlscb_pd == NULL) {
        return NULL;
    }

    return tlscb_pd;
}

// initialize a tlscb pd instance
static inline pd_tlscb_t *
tlscb_pd_init (pd_tlscb_t *tlscb_pd)
{
    if (!tlscb_pd) {
        return NULL;
    }
    tlscb_pd->tlscb = NULL;

    // initialize meta information
    tlscb_pd->hw_ht_ctxt.reset();

    return tlscb_pd;
}

// allocate and initialize a tlscb pd instance
static inline pd_tlscb_t *
tlscb_pd_alloc_init (void)
{
    return tlscb_pd_init(tlscb_pd_alloc());
}

// free tlscb pd instance
static inline hal_ret_t
tlscb_pd_free (pd_tlscb_t *tlscb_pd)
{
    g_hal_state_pd->tlscb_slab()->free(tlscb_pd);
    return HAL_RET_OK;
}

// insert tlscb pd state in all meta data structures
static inline hal_ret_t
add_tlscb_pd_to_db (pd_tlscb_t *tlscb_pd)
{
    g_hal_state_pd->tlscb_hwid_ht()->insert(tlscb_pd, &tlscb_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a tlscb pd instance given its hw id
static inline pd_tlscb_t *
find_tlscb_by_hwid (tlscb_hw_id_t hwid)
{
    return (pd_tlscb_t *)g_hal_state_pd->tlscb_hwid_ht()->lookup(&hwid);
}

extern void *tlscb_pd_get_hw_key_func(void *entry);
extern uint32_t tlscb_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool tlscb_pd_compare_hw_key_func(void *key1, void *key2);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_TLSCB_HPP__

