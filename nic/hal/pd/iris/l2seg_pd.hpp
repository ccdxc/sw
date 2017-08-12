#ifndef __HAL_PD_L2SEG_HPP__
#define __HAL_PD_L2SEG_HPP__

#include <base.h>
#include <ht.hpp>
#include <pd.hpp>
#include <hal_state_pd.hpp>

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_L2SEGMENTS                        2048

// l2seg pd state
struct pd_l2seg_s {
    void               *l2seg;              // PI L2 segment

    // operational state of l2seg pd
    l2seg_hw_id_t      hw_id;               // hw id for this segment

    // meta data maintained for l2seg pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt

    // [Uplink ifpc_id] -> Input Properties(Hash Index). 
    // If L2Seg is native on an uplink, it will have two entries. 
    // (Vlan_v: 1, Vlan: 0; Vlan_v: 0, Vlan: 0);
    uint32_t           inp_prop_tbl_idx[HAL_MAX_UPLINK_IF_PCS];
    uint32_t           inp_prop_tbl_idx_pri[HAL_MAX_UPLINK_IF_PCS];
} __PACK__;

// allocate a l2seg pd instance
static inline pd_l2seg_t *
l2seg_pd_alloc (void)
{
    pd_l2seg_t    *l2seg_pd;

    l2seg_pd = (pd_l2seg_t *)g_hal_state_pd->l2seg_slab()->alloc();
    if (l2seg_pd == NULL) {
        return NULL;
    }

    return l2seg_pd;
}

// initialize a l2seg pd instance
static inline pd_l2seg_t *
l2seg_pd_init (pd_l2seg_t *l2seg_pd)
{
    if (!l2seg_pd) {
        return NULL;
    }
    l2seg_pd->l2seg = NULL;

    // initialize meta information
    l2seg_pd->hw_ht_ctxt.reset();

    return l2seg_pd;
}

// allocate and initialize a l2seg pd instance
static inline pd_l2seg_t *
l2seg_pd_alloc_init (void)
{
    return l2seg_pd_init(l2seg_pd_alloc());
}

// free l2seg pd instance
static inline hal_ret_t
l2seg_pd_free (pd_l2seg_t *l2seg_pd)
{
    g_hal_state_pd->l2seg_slab()->free(l2seg_pd);
    return HAL_RET_OK;
}

// insert l2seg pd state in all meta data structures
static inline hal_ret_t
add_l2seg_pd_to_db (pd_l2seg_t *l2seg_pd)
{
    g_hal_state_pd->l2seg_hwid_ht()->insert(l2seg_pd, &l2seg_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a l2seg pd instance given its hw id
static inline pd_l2seg_t *
find_l2seg_pd_by_hwid (l2seg_hw_id_t hwid)
{
    return (pd_l2seg_t *)g_hal_state_pd->l2seg_hwid_ht()->lookup(&hwid);
}

extern void *l2seg_pd_get_hw_key_func(void *entry);
extern uint32_t l2seg_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool l2seg_pd_compare_hw_key_func(void *key1, void *key2);

uint32_t pd_l2seg_get_l4_prof_idx(pd_l2seg_t *pd_l2seg);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_L2SEG_HPP__

