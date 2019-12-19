//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_PD_L2SEG_HPP__
#define __HAL_PD_L2SEG_HPP__

#include "lib/ht/ht.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define L2SEG_UPLINK_UPD_FLAGS_NWSEC_PROF           0x1
#define HAL_MAX_HW_L2SEGMENTS                       2048

// Mgmt L2seg:
// (A) - l2seg_fl_lkup_id
// Customer L2seg:
// (B) - l2seg_fl_lkup_id
// (C) - l2seg_fl_lkup_id_upl
//
// Non-Shared Mgmt 
// - Uplink: InProp: vrf: (A), mcast_vrf: (A) 
// - Lif: InProp: vrf: (A), mcast_vrf: (A) 
// - Reg_mac entries
//   - (A)
// Non-Shared Customer L2seg
// - Uplink: InProp: vrf: (B), mcast_vrf: (B) 
// - Lif: InPropMacVlan: vrf: (B), mcast_vrf: (B) 
// - Reg_mac entries
//   - Unicast
//     - (B)
//   - Multicast
//     - N/A
// - Flow entries
//   - (B)
// Shared Mgmt & Customer l2seg
// - Uplink: InProp: vrf: (B), mcast_vrf: (A), if_lbl_check: drop
// - Lif: InPropMacVlan: vrf: (B), mcast_vrf: (B) Mcast is never a hit
//                       if_label_check_en:1, if_label_check_fail_drop = 0
// - Lif: InProp: vrf: (B), mcast_vrf: (A), 
//                if_lbl_check: make reg_mac miss and go out.
// - Reg_mac
//   - Unicast
//     - (B)
//   - Multicast
//     - (A)
// - Flow
//   - (B)
//

// l2seg pd state
typedef struct pd_l2seg_s {
    l2seg_hw_id_t   l2seg_hw_id;         // hw id for this segment
    l2seg_hw_id_t   l2seg_hw_id_upl[HAL_MAX_UPLINK_IF_PCS];
    uint32_t        l2seg_fl_lkup_id;    
    uint32_t        l2seg_fl_lkup_id_upl[HAL_MAX_UPLINK_IF_PCS];// May be not needed
    uint32_t        cpu_l2seg_id;        // traffic from CPU
    // [Uplink ifpc_id] -> Input Properties(Hash Index).
    // If L2Seg is native on an uplink, it will have two entries.
    // (Vlan_v: 1, Vlan: 0; Vlan_v: 0, Vlan: 0);
    uint32_t        inp_prop_tbl_idx[HAL_MAX_UPLINK_IF_PCS];
    uint32_t        inp_prop_tbl_idx_pri[HAL_MAX_UPLINK_IF_PCS];
    uint32_t        inp_prop_tbl_cpu_idx;   // traffic from CPU

    // Valid only for classic mode
    uint32_t        num_prom_lifs;       // Prom lifs in l2seg.
    hal_handle_t    prom_if_handle;      // Enic if handle for prom_lif.
                                         // Valid only if num_prom_lifs is 1.
    uint32_t        prom_if_dest_lport;  // Prom IF's dlport

    void            *l2seg;              // PI L2 segment
} __PACK__ pd_l2seg_t;

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
    l2seg_pd->l2seg                = NULL;
    l2seg_pd->l2seg_hw_id          = INVALID_INDEXER_INDEX;
    l2seg_pd->l2seg_fl_lkup_id     = INVALID_INDEXER_INDEX;
    l2seg_pd->cpu_l2seg_id         = INVALID_INDEXER_INDEX;
    l2seg_pd->inp_prop_tbl_cpu_idx = INVALID_INDEXER_INDEX;
    l2seg_pd->num_prom_lifs        = 0;
    l2seg_pd->prom_if_handle       = HAL_HANDLE_INVALID;
    l2seg_pd->prom_if_dest_lport   = INVALID_INDEXER_INDEX;

    for (int i = 0; i < HAL_MAX_UPLINK_IF_PCS; i++) {
        l2seg_pd->inp_prop_tbl_idx[i]     = INVALID_INDEXER_INDEX;
        l2seg_pd->inp_prop_tbl_idx_pri[i] = INVALID_INDEXER_INDEX;
        l2seg_pd->l2seg_hw_id_upl[i]      = INVALID_INDEXER_INDEX; 
        l2seg_pd->l2seg_fl_lkup_id_upl[i] = INVALID_INDEXER_INDEX;
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
    hal::pd::delay_delete_to_slab(HAL_SLAB_L2SEG_PD, l2seg_pd);
    return HAL_RET_OK;
}

// free l2seg pd instance. Just freeing as it will be used during
// update to just memory free.
static inline hal_ret_t
l2seg_pd_mem_free (pd_l2seg_t *l2seg_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_L2SEG_PD, l2seg_pd);
    return HAL_RET_OK;
}

extern void *flow_lkupid_get_hw_key_func(void *entry);
extern uint32_t flow_lkupid_hw_key_size(void);
hal_ret_t pd_l2seg_update_prom_lifs(pd_l2seg_t *pd_l2seg,
                                    if_t *prom_enic_if,
                                    bool inc, bool skip_hw_pgm);
#if 0
hal_ret_t l2seg_pd_pgm_mbr_ifs (block_list *if_list, l2seg_t *l2seg,
                                bool is_upgrade);
#endif
hal_ret_t l2seg_pd_depgm_mbr_ifs (block_list *if_list, l2seg_t *l2seg);
hal_ret_t l2seg_uplink_depgm_input_properties_tbl (l2seg_t *l2seg,
                                                   if_t *hal_if);
hal_ret_t l2seg_uplink_pgm_input_properties_tbl(l2seg_t *l2seg,
                                                if_t *hal_if,
                                                bool is_upgrade = false);
hal_ret_t l2seg_pd_depgm_ifs_inp_prop_tbl(l2seg_t *l2seg);
hal_ret_t l2seg_pd_depgm_cpu_tx_inp_prop_tbl (pd_l2seg_t *l2seg_pd);
hal_ret_t l2seg_pd_repgm_mbr_ifs (block_list *agg_list, l2seg_t *l2seg,
                                  bool is_upgrade = false);
hal_ret_t l2seg_uplink_upd_input_properties_tbl (pd_add_l2seg_uplink_args_t *args,
                                                 uint32_t upd_flags,
                                                 nwsec_profile_t *nwsec_prof,
                                                 uint32_t num_prom_lifs,
                                                 if_t *prom_enic_if);
hal_ret_t l2seg_uplink_inp_prop_form_data (l2seg_t *l2seg, if_t *hal_if,
                                           uint32_t upd_flags,
                                           nwsec_profile_t *nwsec_prof,
                                           uint32_t num_prom_lifs,
                                           if_t *prom_enic_if,
                                           input_properties_actiondata_t &data);
hal_ret_t l2seg_pd_repgm_enics (l2seg_t *l2seg);
hal_ret_t l2seg_pd_copy_inp_prop_tbl_idx (l2seg_t *l2seg, 
                                          hal_handle_t other_l2seg_hdl,
                                          uint32_t up_ifpc_id);
l2seg_t *l2seg_pd_get_shared_mgmt_l2seg(l2seg_t *l2seg, 
                                        if_t *hal_if);
hal_ret_t l2seg_pd_inp_prop_info(l2seg_t *cl_l2seg, l2seg_t *hp_l2seg, 
                                 if_t *hal_if, uint32_t upd_flags,
                                 nwsec_profile_t *nwsec_prof,
                                 uint32_t num_prom_lifs,
                                 if_t *prom_enic_if,
                                 input_properties_actiondata_t &data);
hal_ret_t l2seg_program_eps_reg_mac(l2seg_t *l2seg, table_oper_t oper);
hal_ret_t l2seg_pd_upd_cpu_inp_prop_tbl(pd_l2seg_t *l2seg_pd);
hal_ret_t l2seg_cpu_inp_prop_form_data(pd_l2seg_t *l2seg_pd,
                                       input_properties_actiondata_t &data);
hal_ret_t l2seg_repgm_mgmt_enics_eps(l2seg_t *l2seg, l2seg_t *hp_l2seg);
}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_L2SEG_HPP__

