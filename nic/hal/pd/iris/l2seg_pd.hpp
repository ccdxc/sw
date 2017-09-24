#ifndef __HAL_PD_L2SEG_HPP__
#define __HAL_PD_L2SEG_HPP__

#include <base.h>
#include <ht.hpp>
#include <pd.hpp>
#include <pd_api.hpp>
#include <hal_state_pd.hpp>

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_L2SEGMENTS                        2048

// l2seg pd state
struct pd_l2seg_s {
    void                *l2seg;              // PI L2 segment

    // operational state of l2seg pd
    l2seg_hw_id_t       l2seg_hw_id;         // hw id for this segment

    uint16_t            l2seg_ten_hw_id;     // used in data plane as vrf   
    uint16_t            l2seg_fromcpu_id;    // traffic from CPU

    // meta data maintained for l2seg pd
    // ht_ctxt_t           hw_ht_ctxt;          // h/w id based hash table ctxt

    // [Uplink ifpc_id] -> Input Properties(Hash Index). 
    // If L2Seg is native on an uplink, it will have two entries. 
    // (Vlan_v: 1, Vlan: 0; Vlan_v: 0, Vlan: 0);
    uint32_t            inp_prop_tbl_idx[HAL_MAX_UPLINK_IF_PCS];
    uint32_t            inp_prop_tbl_idx_pri[HAL_MAX_UPLINK_IF_PCS];

    // Entry used by traffic From CPU traffic
    uint32_t            inp_prop_tbl_cpu_idx;
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
    // l2seg_pd->hw_ht_ctxt.reset();
    for (int i = 0; i < HAL_MAX_UPLINK_IF_PCS; i++) {
        l2seg_pd->inp_prop_tbl_idx[i] = INVALID_INDEXER_INDEX;
        l2seg_pd->inp_prop_tbl_idx_pri[i] = INVALID_INDEXER_INDEX;
    }

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

// free l2seg pd instance. Just freeing as it will be used during
// update to just memory free.
static inline hal_ret_t
l2seg_pd_mem_free (pd_l2seg_t *l2seg_pd)
{
    g_hal_state_pd->l2seg_slab()->free(l2seg_pd);
    return HAL_RET_OK;
}

#if 0
// insert l2seg pd state in all meta data structures
static inline hal_ret_t
add_l2seg_pd_to_db (pd_l2seg_t *l2seg_pd)
{
    g_hal_state_pd->l2seg_hwid_ht()->insert(l2seg_pd, &l2seg_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}
#endif

// find a l2seg pd instance given its hw id
static inline pd_l2seg_t *
find_l2seg_pd_by_hwid (l2seg_hw_id_t hwid)
{
    hal_handle_id_ht_entry_t    *entry;
    l2seg_t                     *l2seg;
    pd_l2seg_t                  *l2seg_pd = NULL;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state_pd->l2seg_hwid_ht()->lookup(&hwid);
    if (entry) {
        l2seg = (l2seg_t *)hal_handle_get_obj(entry->handle_id);
        l2seg_pd = (pd_l2seg_t *)l2seg->pd;
        return l2seg_pd;;
    }
    return NULL;
}

extern void *l2seg_pd_hwid_get_hw_key_func(void *entry);
extern uint32_t l2seg_pd_hwid_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool l2seg_pd_hwid_compare_hw_key_func(void *key1, void *key2);

hal_ret_t l2seg_pd_alloc_res(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_alloc_cpuid(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_alloc_hwid(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_dealloc_res(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_dealloc_cpuid(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_dealloc_hwid(pd_l2seg_t *pd_l2seg);
void link_pi_pd(pd_l2seg_t *pd_l2seg, l2seg_t *pi_l2seg);
void delink_pi_pd(pd_l2seg_t *pd_l2seg, l2seg_t *pi_l2seg);

hal_ret_t l2seg_pd_cleanup(pd_l2seg_t *l2seg_pd);
uint32_t l2seg_pd_l2seguplink_count(pd_l2seg_t *l2seg_pd);
uint32_t pd_l2seg_get_l4_prof_idx(pd_l2seg_t *pd_l2seg);
pd_tenant_t *pd_l2seg_get_pd_tenant(pd_l2seg_t *pd_l2seg);
hal_ret_t l2seg_pd_program_hw(pd_l2seg_t *l2seg_pd);
hal_ret_t l2seg_pd_pgm_inp_prop_tbl (pd_l2seg_t *l2seg_pd);
hal_ret_t l2seg_pd_deprogram_hw (pd_l2seg_t *l2seg_pd);
hal_ret_t l2seg_pd_depgm_inp_prop_tbl (pd_l2seg_t *l2seg_pd);


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_L2SEG_HPP__

