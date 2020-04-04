#ifndef __HAL_PD_ENICIF_HPP__
#define __HAL_PD_ENICIF_HPP__

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"

namespace hal {
namespace pd {

#define ENICIF_UPD_FLAGS_NONE           0x0
#define ENICIF_UPD_FLAGS_NWSEC_PROF     0x1
#define ENICIF_UPD_FLAGS_NUM_PROM_LIFS  0x2

struct pd_enicif_s {
    // Hw Indices
    uint32_t    inp_prop_mac_vlan_idx_host;     // Pkts from Host
    uint32_t    inp_prop_mac_vlan_idx_upl;      // Pkts from Uplink
    uint32_t    enic_lport_id;                  // lport
    uint32_t    inp_prop_native_l2seg_clsc;     // Classic mode, pkts from host
    uint32_t    inp_prop_native_l2seg_pri_clsc; // Classic mode, pkts from host with priority tag
    uint32_t    inp_prop_native_l2seg_pri_sband_clsc; // Classic mode, pkts from host with priority tag in sideband

    // pi ptr
    void        *pi_if;
} __PACK__;

struct pd_if_l2seg_entry_s {
    uint32_t    inp_prop_idx;
    uint32_t    inp_prop_idx_sband;

    // pi ptr
    void        *pi_if_l2seg_entry;
} __PACK__;

// ----------------------------------------------------------------------------
// Allocate EnicIf Instance
// ----------------------------------------------------------------------------
static inline pd_enicif_t *
pd_enicif_alloc (void)
{
    pd_enicif_t    *enicif;

    enicif = (pd_enicif_t *)g_hal_state_pd->enicif_pd_slab()->alloc();
    if (enicif == NULL) {
        return NULL;
    }
    return enicif;
}

// ----------------------------------------------------------------------------
// Initialize EnicIF PD instance
// ----------------------------------------------------------------------------
static inline pd_enicif_t *
pd_enicif_init (pd_enicif_t *enicif)
{
    // Nothing to do currently
    if (!enicif) {
        return NULL;
    }

    // Set here if you want to initialize any fields
    enicif->inp_prop_mac_vlan_idx_host           = INVALID_INDEXER_INDEX;
    enicif->inp_prop_mac_vlan_idx_upl            = INVALID_INDEXER_INDEX;
    enicif->enic_lport_id                        = INVALID_INDEXER_INDEX;
    enicif->inp_prop_native_l2seg_clsc           = INVALID_INDEXER_INDEX;
    enicif->inp_prop_native_l2seg_pri_clsc       = INVALID_INDEXER_INDEX;
    enicif->inp_prop_native_l2seg_pri_sband_clsc = INVALID_INDEXER_INDEX;

    return enicif;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize EnicIf PD Instance
// ----------------------------------------------------------------------------
static inline pd_enicif_t *
pd_enicif_alloc_init(void)
{
    return pd_enicif_init(pd_enicif_alloc());
}

// ----------------------------------------------------------------------------
// Freeing EnicIF PD
// ----------------------------------------------------------------------------
static inline hal_ret_t
pd_enicif_free (pd_enicif_t *enicif)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_ENICIF_PD, enicif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void
pd_enicif_link_pi_pd(pd_enicif_t *pd_enicif, if_t *pi_if)
{
    pd_enicif->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_enicif);
}

// ----------------------------------------------------------------------------
// De-Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void
pd_enicif_delink_pi_pd(pd_enicif_t *pd_enicif, if_t *pi_if)
{
    pd_enicif->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
}

hal_ret_t pd_enicif_alloc_res(pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_program_hw(pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_free (pd_enicif_t *enicif);
void link_pi_pd(pd_enicif_t *pd_upif, if_t *pi_if);
void unlink_pi_pd(pd_enicif_t *pd_upif, if_t *pi_if);
hal_ret_t
pd_enicif_pgm_inp_prop_mac_vlan_tbl(pd_enicif_t *pd_enicif,
                                    pd_if_update_args_t *args,
                                    pd_if_lif_update_args_t *lif_args,
                                    table_oper_t oper);
hal_ret_t
pd_enicif_pd_pgm_output_mapping_tbl(pd_enicif_t *pd_enicif,
                                    pd_if_update_args_t *args,
                                    pd_if_lif_update_args_t *lif_upd,
                                    table_oper_t oper);
hal_ret_t pd_enicif_cleanup(pd_enicif_t *pd_enicif);

uint32_t pd_enicif_get_l4_prof_idx(pd_enicif_t *pd_enicif);
pd_lif_t *pd_enicif_get_pd_lif(pd_enicif_t *pd_enicif);
hal_ret_t
pd_enicif_inp_prop_mac_vlan_form_data(pd_enicif_t *pd_enicif,
                                      lif_t *lif,
                                      uint32_t upd_flags,
                                      nwsec_profile_t *nwsec_prof,
                                      pd_if_update_args_t *args,
                                      pd_if_lif_update_args_t *lif_args,
                                      input_properties_mac_vlan_actiondata_t &data,
                                      bool host_entry);
hal_ret_t pd_enicif_lif_update(pd_if_lif_update_args_t *args);
hal_ret_t
pd_enicif_upd_inp_prop_mac_vlan_tbl (pd_enicif_t *pd_enicif,
                                     uint32_t upd_flags,
                                     nwsec_profile_t *nwsec_prof);
hal_ret_t
pd_enicif_pd_pgm_inp_prop(pd_enicif_t *pd_enicif,
                          dllist_ctxt_t *l2sege_list,
                          pd_if_update_args_t *args,
                          pd_if_lif_update_args_t *lif_args,
                          table_oper_t oper);
hal_ret_t
pd_enicif_depgm_inp_prop_mac_vlan_tbl(pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_pd_depgm_output_mapping_tbl (pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_deprogram_hw (pd_enicif_t *pd_enicif, bool del_only_inp_mac_vlan);
bool pd_enicif_get_vlan_strip (lif_t *lif, pd_if_lif_update_args_t *lif_upd);
hal_ret_t pd_enicif_alloc_pd_l2seg_entries(dllist_ctxt_t *pi_l2seg_list);
hal_ret_t pd_enicif_dealloc_pd_l2seg_entries(dllist_ctxt_t *pi_l2seg_list);
hal_ret_t pd_enicif_alloc_l2seg_entries (pd_enicif_t *pd_enicif);
hal_ret_t
pd_enicif_pd_pgm_inp_prop_l2seg(pd_enicif_t *pd_enicif,
                                uint32_t upd_flags,
                                uint32_t num_prom_lifs,
                                l2seg_t *l2seg,
                                pd_if_l2seg_entry_t *if_l2seg,
                                pd_if_update_args_t *args,
                                pd_if_lif_update_args_t *lif_args,
                                l2seg_t *attached_l2seg,
                                table_oper_t oper);
hal_ret_t
pd_enicif_upd_inp_prop_l2seg(if_t *hal_if,
                             l2seg_t *l2seg,
                             uint32_t upd_flags,
                             uint32_t num_prom_lifs);
hal_ret_t pd_enicif_upd_l2seg_clsc_change(pd_if_update_args_t *args);
hal_ret_t pd_enicif_pd_depgm_inp_prop(pd_enicif_t *pd_enicif,
                                      dllist_ctxt_t *l2sege_list);
hal_ret_t pd_enicif_pd_depgm_inp_prop_l2seg(uint32_t inp_prop_idx);
hal_ret_t pd_enicif_upd_native_l2seg_clsc_change(pd_if_update_args_t *args);
hal_ret_t pd_enicif_update_num_prom_lifs(if_t *hal_if,
                                         bool inc, bool skip_hw_pgm);
hal_ret_t pd_enicif_create(pd_if_create_args_t *args);
hal_ret_t pd_enicif_update(pd_if_update_args_t *args);
hal_ret_t pd_enicif_delete(pd_if_delete_args_t *args);
hal_ret_t pd_enicif_get (pd_if_get_args_t *args);
hal_ret_t pd_enicif_make_clone(pd_if_make_clone_args_t *args);
hal_ret_t pd_enicif_mem_free(pd_if_mem_free_args_t *args);
hal_ret_t pd_enicif_upd_egress_en_change (pd_if_update_args_t *args);
hal_ret_t pd_enicif_upd_pinned_uplink_change (pd_if_update_args_t *args);
if_t *pd_enicif_get_pinned_uplink_for_inp_props(if_t *hal_if,
                                                pd_if_update_args_t *if_args,
                                                pd_if_lif_update_args_t *lif_args);
bool pd_enicif_has_lif(if_t *hal_if, pd_if_update_args_t *if_args);
hal_ret_t pd_enicif_update_num_prom(pd_enicif_t *pd_enicif,
                                    dllist_ctxt_t *add_l2seg_list,
                                    dllist_ctxt_t *del_l2seg_list);
hal_ret_t pd_enicif_inb_pgm_inp_prop_mac_vlan_tbl(pd_enicif_t *pd_enicif,
                                                  table_oper_t oper);
}   // namespace pd
}   // namespace hal
#endif    // __HAL_PD_ENICIF_HPP__

