#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/common/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/include/l2segment_api.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/hal/pd/iris/if_pd.hpp"
#include "nic/hal/pd/iris/lif_pd.hpp"
#include "nic/hal/pd/iris/enicif_pd.hpp"
#include "l2seg_pd.hpp"
#include "if_pd_utils.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/iris/p4pd/p4pd_defaults.hpp"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Enic If Create
// ----------------------------------------------------------------------------
hal_ret_t 
pd_enicif_create(pd_if_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_enicif_t          *pd_enicif;

    HAL_TRACE_DEBUG("pd-enicif:{}: creating pd state for enicif: {}", 
                    __FUNCTION__, if_get_if_id(args->intf));

    // Create Enic If PD
    pd_enicif = pd_enicif_alloc_init();
    if (pd_enicif == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    pd_enicif_link_pi_pd(pd_enicif, args->intf);

    // Create L2segs PD entries for classic
    ret = pd_enicif_alloc_l2seg_entries(pd_enicif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif::{}: unable to alloc. pd l2seg entries: ret:{}",
                      __FUNCTION__, ret);
        goto end;
    }

    // Allocate Resources
    ret = pd_enicif_alloc_res(pd_enicif);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("pd-enicif:{}: unable to alloc. resources for enicif: {}",
                      __FUNCTION__, if_get_if_id(args->intf));
        goto end;
    }

    // Program HW
    ret = pd_enicif_program_hw(pd_enicif);

end:
    if (ret != HAL_RET_OK) {
        pd_enicif_cleanup(pd_enicif);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD EnicIf Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_enicif_update (pd_if_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;

    if (args->l2seg_clsc_change) {
        ret = pd_enicif_upd_l2seg_clsc_change(args);
    } else {
        if (args->native_l2seg_clsc_change) {
            ret = pd_enicif_upd_native_l2seg_clsc_change(args);
        }
        if (args->pinned_uplink_change) {
        }
    }


    return ret;
}

//-----------------------------------------------------------------------------
// PD EnicIf Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_enicif_delete (pd_if_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_enicif_t    *enicif_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->intf != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->intf->pd_if != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-enicif:{}:deleting pd state for enicif: {}",
                    __FUNCTION__, args->intf->if_id);
    enicif_pd = (pd_enicif_t *)args->intf->pd_if;

    // deprogram HW
    ret = pd_enicif_deprogram_hw(enicif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}:unable to deprogram hw", __FUNCTION__);
    }

    ret = pd_enicif_cleanup(enicif_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}:failed pd enicif delete",
                      __FUNCTION__);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Enicif Update: Handling pinned uplink change
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_upd_pinned_uplink_change(pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_enicif_t     *pd_enicif = (pd_enicif_t *)args->intf->pd_if;
    if_t            *hal_if = (if_t *)pd_enicif->pi_if;

    HAL_TRACE_DEBUG("pd-enicif:{} pinned uplink change: ", __FUNCTION__);

    // Program input prop. entries for new l2segs
    ret = pd_enicif_pd_pgm_inp_prop(pd_enicif, &hal_if->l2seg_list_clsc_head,
                                    args, TABLE_OPER_UPDATE);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif: failed to repgm input prop."
                "for l2segs. ret:{}", ret);
        goto end;
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Enicif Update: Handling native l2seg change
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_upd_native_l2seg_clsc_change(pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_enicif_t     *pd_enicif = (pd_enicif_t *)args->intf->pd_if;
    l2seg_t         *native_l2seg = NULL;

    HAL_TRACE_DEBUG("pd-enicif:{} native l2seg change: ", __FUNCTION__);

    // Remove old native l2seg input prop entry
    ret = pd_enicif_pd_depgm_inp_prop_l2seg(pd_enicif->
                                            inp_prop_native_l2seg_clsc);

    if (args->new_native_l2seg_clsc != HAL_HANDLE_INVALID) {
        // Install new native l2seg input prop entry
        native_l2seg = find_l2seg_by_handle(args->new_native_l2seg_clsc);
        ret = pd_enicif_pd_pgm_inp_prop_l2seg(pd_enicif, 
                                              native_l2seg,
                                              NULL, args,
                                              TABLE_OPER_INSERT);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Enicif Update: Handling l2seg list change
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_upd_l2seg_clsc_change(pd_if_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_enicif_t     *pd_enicif = (pd_enicif_t *)args->intf->pd_if;

    HAL_TRACE_DEBUG("pd-enicif:{} l2seg-list change: ", __FUNCTION__);

    // Allocated PD State for new IP entries
    ret = pd_enicif_alloc_pd_l2seg_entries(args->add_l2seg_clsclist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif: failed to alloced pd l2seg entries "
                "for new l2segs. ret:{}", ret);
        goto end;
    }

    // Program input prop. entries for new l2segs
    ret = pd_enicif_pd_pgm_inp_prop(pd_enicif, args->add_l2seg_clsclist,
                                    args, TABLE_OPER_UPDATE);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif: failed to pgm input prop."
                "for new l2segs. ret:{}", ret);
        goto end;
    }

    // Deprogram input prop. entries for deleted l2segs
    ret = pd_enicif_pd_depgm_inp_prop(pd_enicif,
                                      args->del_l2seg_clsclist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif: failed to depgm input prop."
                "for deleted l2segs ret:{}", ret);
        goto end;
    }

    // free up delete IPs PD state
    ret = pd_enicif_dealloc_pd_l2seg_entries(args->del_l2seg_clsclist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif: failed to free pd l2seg entries. ret:{}", ret);
        goto end;
    }


end:
    return ret;
}

// ----------------------------------------------------------------------------
// Allocate PD state for l2segs for classic enic
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_alloc_l2seg_entries (pd_enicif_t *pd_enicif)
{
    hal_ret_t            ret = HAL_RET_OK;
    if_t                 *hal_if = (if_t *)pd_enicif->pi_if;

    ret = pd_enicif_alloc_pd_l2seg_entries(&(hal_if->l2seg_list_clsc_head));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}: failed to allocated pd l2seg entries.",
                __FUNCTION__);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Allocate PD l2seg entries of a list of PI entries
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_alloc_pd_l2seg_entries(dllist_ctxt_t *pi_l2seg_list)
{
    hal_ret_t               ret = HAL_RET_OK;
    dllist_ctxt_t           *lnode = NULL;
    if_l2seg_entry_t         *pi_l2seg_entry = NULL;

    // Walk through l2seg entries
    dllist_for_each(lnode, pi_l2seg_list) {
        pi_l2seg_entry = dllist_entry(lnode, if_l2seg_entry_t, lentry);

        pi_l2seg_entry->pd = (pd_if_l2seg_entry_t *)g_hal_state_pd->
                             if_l2seg_entry_slab()->alloc();
        if (!pi_l2seg_entry->pd) {
            ret = HAL_RET_OOM;
            goto end;
        }

        // Link PI to PD
        ((pd_if_l2seg_entry_t *)(pi_l2seg_entry->pd))->pi_if_l2seg_entry = pi_l2seg_entry;

        HAL_TRACE_DEBUG("pd-enicif:{}: Allocating pd l2seg entry for l2seg_hdl:{}",
                __FUNCTION__, pi_l2seg_entry->l2seg_handle);
    }

end:

    if (ret != HAL_RET_OK) {
        // Walk through l2seg entries
        dllist_for_each(lnode, pi_l2seg_list) {
            pi_l2seg_entry = dllist_entry(lnode, if_l2seg_entry_t, lentry);
            if (pi_l2seg_entry->pd) {
                // Free PD l2seg entry
                g_hal_state_pd->if_l2seg_entry_slab()->free(pi_l2seg_entry->pd);
                // Unlink PD from PI
                pi_l2seg_entry->pd = NULL;
            }
        }
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Deleted PD IP entries
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_dealloc_pd_l2seg_entries(dllist_ctxt_t *pi_l2seg_list)
{
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *lnode = NULL;
    if_l2seg_entry_t         *pi_l2seg_entry = NULL;

    // Walk through ip entries
    dllist_for_each(lnode, pi_l2seg_list) {
        pi_l2seg_entry = dllist_entry(lnode, if_l2seg_entry_t, lentry);
        if (pi_l2seg_entry->pd) {
            // Free PD IP entry
            g_hal_state_pd->if_l2seg_entry_slab()->free(pi_l2seg_entry->pd);
            // Unlink PD from PI
            pi_l2seg_entry->pd = NULL;
            HAL_TRACE_DEBUG("pd-enicif:{}: freeing pd enicif l2seg entry for l2seg_hdl: {}",
                    __FUNCTION__, pi_l2seg_entry->l2seg_handle);
        } else {
            HAL_ASSERT(0);
        }
    }

    return ret;
}




// ----------------------------------------------------------------------------
// Allocate resources for PD EnicIf
// ----------------------------------------------------------------------------
hal_ret_t 
pd_enicif_alloc_res(pd_enicif_t *pd_enicif)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    // Allocate lport
    rs = g_hal_state_pd->lport_idxr()->alloc((uint32_t *)&pd_enicif->
            enic_lport_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("pd-enicif:{}:failed to alloc lport_id err: {}", 
                      __FUNCTION__, rs);
        pd_enicif->enic_lport_id = INVALID_INDEXER_INDEX;
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("pd-enicif:{}: if_id:{} allocated lport_id:{}", 
                    __FUNCTION__, 
                    if_get_if_id((if_t *)pd_enicif->pi_if),
                    pd_enicif->enic_lport_id);

    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources. 
//-----------------------------------------------------------------------------
hal_ret_t
pd_enicif_dealloc_res(pd_enicif_t *pd_enicif)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    if (pd_enicif->enic_lport_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->lport_idxr()->free(pd_enicif->enic_lport_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("pd-enicif:{}:failed to free lport_id err: {}", 
                          __FUNCTION__, pd_enicif->enic_lport_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("pd-enicif:{}:freed lport_id: {}", 
                        __FUNCTION__, pd_enicif->enic_lport_id);
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD EnicIf Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD If 
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
pd_enicif_cleanup(pd_enicif_t *pd_enicif)
{
    hal_ret_t       ret = HAL_RET_OK;
    if_t            *hal_if = (if_t *)pd_enicif->pi_if;

    if (!pd_enicif) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = pd_enicif_dealloc_res(pd_enicif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}: unable to dealloc res for enicif: {}", 
                      __FUNCTION__, 
                      ((if_t *)(pd_enicif->pi_if))->if_id);
        goto end;
    }

    // Free up l2segs PD state
    ret = pd_enicif_pd_depgm_inp_prop(pd_enicif, 
                                      &hal_if->l2seg_list_clsc_head);

    // Delinking PI<->PD
    pd_enicif_delink_pi_pd(pd_enicif, (if_t *)pd_enicif->pi_if);

    // Freeing PD
    pd_enicif_free(pd_enicif);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_deprogram_hw (pd_enicif_t *pd_enicif)
{
    hal_ret_t            ret = HAL_RET_OK;

    // De program TM register
    ret = pd_enicif_depgm_inp_prop_mac_vlan_tbl(pd_enicif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}:unable to deprogram hw", __FUNCTION__);
        goto end;
    }

    // De-Program Output Mapping Table
    ret = pd_enicif_pd_depgm_output_mapping_tbl(pd_enicif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}:unable to deprogram hw", __FUNCTION__);
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_depgm_inp_prop_mac_vlan_tbl(pd_enicif_t *pd_enicif)
{
    hal_ret_t           ret = HAL_RET_OK;
    Tcam                *inp_prop_mac_vlan_tbl = NULL;

    inp_prop_mac_vlan_tbl = g_hal_state_pd->tcam_table(
                            P4TBL_ID_INPUT_PROPERTIES_MAC_VLAN);
    HAL_ASSERT_RETURN((inp_prop_mac_vlan_tbl != NULL), HAL_RET_ERR);

    ret = inp_prop_mac_vlan_tbl->remove(pd_enicif->inp_prop_mac_vlan_idx_host);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}:unable to deprogram inp mac vlan table "
                      "for host traffic",
                      __FUNCTION__, pd_enicif->inp_prop_mac_vlan_idx_host);
        goto end;
    } else {
        HAL_TRACE_ERR("pd-enicif:{}:deprogrammed inp mac vlan table for "
                      "host traffic",
                      __FUNCTION__, pd_enicif->inp_prop_mac_vlan_idx_host);
    }

    ret = inp_prop_mac_vlan_tbl->remove(pd_enicif->inp_prop_mac_vlan_idx_upl);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}:unable to deprogram inp mac vlan table "
                      "for uplink traffic",
                      __FUNCTION__, pd_enicif->inp_prop_mac_vlan_idx_upl);
        goto end;
    } else {
        HAL_TRACE_ERR("pd-enicif:{}:deprogrammed inp mac vlan table for "
                      "uplink traffic",
                      __FUNCTION__, pd_enicif->inp_prop_mac_vlan_idx_upl);
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_pd_depgm_output_mapping_tbl (pd_enicif_t *pd_enicif)
{
    hal_ret_t           ret = HAL_RET_OK;
    DirectMap           *dm_omap = NULL;

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    HAL_ASSERT_RETURN((dm_omap != NULL), HAL_RET_ERR);
    
    ret = dm_omap->remove(pd_enicif->enic_lport_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}:unable to deprogram omapping table",
                __FUNCTION__, pd_enicif->enic_lport_id);
    } else {
        HAL_TRACE_ERR("pd-enicif:{}:deprogrammed omapping table",
                __FUNCTION__, pd_enicif->enic_lport_id);
    }


    return ret;
}
// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_program_hw(pd_enicif_t *pd_enicif)
{
    hal_ret_t            ret;
    nwsec_profile_t      *pi_nwsec = NULL;
    if_t                 *hal_if = (if_t *)pd_enicif->pi_if;
    l2seg_t              *native_l2seg_clsc = NULL;

    pi_nwsec = (nwsec_profile_t *)if_enicif_get_pi_nwsec((if_t *)pd_enicif->pi_if);
    if (pi_nwsec == NULL) {
        HAL_TRACE_DEBUG("pd-enicif:{}: No nwsec. Programming default", __FUNCTION__);
    }

    // Check if classic
    if (hal_if->enic_type != intf::IF_ENIC_TYPE_CLASSIC) {
        // Program Input Properties Mac Vlan
        ret = pd_enicif_pgm_inp_prop_mac_vlan_tbl(pd_enicif, pi_nwsec);
    }

    // Program Output Mapping 
    ret = pd_enicif_pd_pgm_output_mapping_tbl(pd_enicif, NULL,
                                              TABLE_OPER_INSERT);

    // Check if classic
    if (hal_if->enic_type == intf::IF_ENIC_TYPE_CLASSIC) {
        ret = pd_enicif_pd_pgm_inp_prop(pd_enicif, 
                                        &hal_if->l2seg_list_clsc_head,
                                        NULL, TABLE_OPER_INSERT);

        // Program native l2seg
        if (hal_if->native_l2seg_clsc != HAL_HANDLE_INVALID) {
            native_l2seg_clsc = 
                find_l2seg_by_handle(hal_if->native_l2seg_clsc);
            ret = pd_enicif_pd_pgm_inp_prop_l2seg(pd_enicif, 
                                                  native_l2seg_clsc,
                                                  NULL, NULL,
                                                  TABLE_OPER_INSERT);
        }
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Programming input properties table for classic nic
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_pd_pgm_inp_prop(pd_enicif_t *pd_enicif, 
                          dllist_ctxt_t *l2sege_list,
                          pd_if_args_t *args,
                          table_oper_t oper)
{
    hal_ret_t               ret = HAL_RET_OK;
    dllist_ctxt_t           *lnode = NULL;
    if_l2seg_entry_t        *pi_l2seg_entry = NULL;
    l2seg_t                 *l2seg = NULL;

    // Walk through l2seg entries
    dllist_for_each(lnode, l2sege_list) {
        pi_l2seg_entry = dllist_entry(lnode, if_l2seg_entry_t, lentry);
        l2seg = find_l2seg_by_handle(pi_l2seg_entry->l2seg_handle);
        if (l2seg == NULL) {
            HAL_TRACE_ERR("pd-enicif:{}:unable to find l2seg for handle:{}",
                          __FUNCTION__, pi_l2seg_entry->l2seg_handle);
            goto end;
        }

        ret = pd_enicif_pd_pgm_inp_prop_l2seg(pd_enicif, l2seg, 
                                              (pd_if_l2seg_entry_t *)
                                              pi_l2seg_entry->pd,
                                              args, oper);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-enicif:{}:unable to pgm for l2seg:{}, if{}",
                           l2seg->seg_id, 
                           if_get_if_id((if_t *)pd_enicif->pi_if));
        }
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgramming input properties table for classic nic
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_pd_depgm_inp_prop(pd_enicif_t *pd_enicif, dllist_ctxt_t *l2sege_list)
{
    hal_ret_t               ret = HAL_RET_OK;
    dllist_ctxt_t           *lnode = NULL;
    if_l2seg_entry_t        *pi_l2seg_entry = NULL;
    l2seg_t                 *l2seg = NULL;
    pd_if_l2seg_entry_t     *pd_l2seg_entry = NULL;

    // Walk through l2seg entries
    dllist_for_each(lnode, l2sege_list) {
        pi_l2seg_entry = dllist_entry(lnode, if_l2seg_entry_t, lentry);
        pd_l2seg_entry = (pd_if_l2seg_entry_t *)pi_l2seg_entry->pd;

        ret = pd_enicif_pd_depgm_inp_prop_l2seg(pd_l2seg_entry->inp_prop_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-enicif:{}:unable to depgm input properties for "
                          "l2seg:{}, if{}",
                          l2seg->seg_id, 
                          if_get_if_id((if_t *)pd_enicif->pi_if));
        }
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Check if l2seg is native on enicif in classic mode
// ----------------------------------------------------------------------------
bool
is_l2seg_native_on_enicif_classic(if_t *hal_if, l2seg_t *l2seg)
{
    bool            is_native = false;
    l2seg_t         *if_native_l2seg = NULL;

    if_native_l2seg = find_l2seg_by_handle(hal_if->native_l2seg_clsc);
    if (if_native_l2seg) {
        if (if_native_l2seg->seg_id == l2seg->seg_id) {
            is_native = true;
        }
    }

    return is_native;
}

// ----------------------------------------------------------------------------
// Programming input properties table for classic nic
// ----------------------------------------------------------------------------
#define inp_prop data.input_properties_action_u.input_properties_input_properties
hal_ret_t
pd_enicif_pd_pgm_inp_prop_l2seg(pd_enicif_t *pd_enicif, l2seg_t *l2seg,
                                pd_if_l2seg_entry_t *if_l2seg,
                                pd_if_args_t *args,
                                table_oper_t oper)
{
    hal_ret_t                   ret = HAL_RET_OK;
    input_properties_swkey_t    key;
    input_properties_actiondata data;
    if_t                        *hal_if = (if_t *)pd_enicif->pi_if;
    if_t                        *uplink = NULL;
    pd_l2seg_t                  *l2seg_pd;
    Hash                        *inp_prop_tbl = NULL;
    uint32_t                    hash_idx = INVALID_INDEXER_INDEX;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((inp_prop_tbl != NULL), HAL_RET_ERR);

    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(l2seg);
    if (args && args->pinned_uplink_change) {
        uplink = find_if_by_handle(args->new_pinned_uplink);
    } else {
        uplink = find_if_by_handle(hal_if->pinned_uplink);
    }


    // Key
    key.capri_intrinsic_lif = if_get_hw_lif_id(hal_if);
    if (!is_l2seg_native_on_enicif_classic(hal_if, l2seg)) {
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = l2seg_get_fab_encap_val(l2seg);
    }

    // Data
    inp_prop.vrf = l2seg_pd->l2seg_ten_hw_id;
    inp_prop.dir = FLOW_DIR_FROM_ENIC;
    inp_prop.l4_profile_idx = L4_PROF_DEFAULT_ENTRY;
    inp_prop.ipsg_enable = 0;
    inp_prop.src_lport = pd_enicif->enic_lport_id;
    inp_prop.dst_lport = if_get_lport_id(uplink);
    inp_prop.flow_miss_action = l2seg_get_bcast_fwd_policy(l2seg);
    inp_prop.flow_miss_idx = l2seg_get_bcast_oif_list(l2seg);
    
    if (oper == TABLE_OPER_INSERT) {
        // Insert
        ret = inp_prop_tbl->insert(&key, &data, &hash_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-enicif:{}:classic: unable to program for "
                          "(l2seg, upif): ({}, {})",
                          __FUNCTION__, 
                          hal::l2seg_get_l2seg_id(l2seg), 
                          if_get_if_id(hal_if));
            goto end;
        } else {
            HAL_TRACE_DEBUG("pd-enicif:{}:classic: Programmed "
                            "table:input_properties index:{} ", __FUNCTION__,
                            hash_idx);
        }

        if (if_l2seg) {
            // Non-Native l2seg
            if_l2seg->inp_prop_idx = hash_idx;
        } else {
            // Native l2seg
            pd_enicif->inp_prop_native_l2seg_clsc = hash_idx;
        }
    } else {
        // hash_idx
        if (if_l2seg) {
            hash_idx = if_l2seg->inp_prop_idx;
        } else {
            // No if_l2seg for native case
            hash_idx = pd_enicif->inp_prop_native_l2seg_clsc;
        }
        // Update
        ret = inp_prop_tbl->update(hash_idx, &data);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-enicif:{}:classic: unable to reprogram for "
                          "(l2seg, upif): ({}, {})",
                          __FUNCTION__, 
                          hal::l2seg_get_l2seg_id(l2seg), 
                          if_get_if_id(hal_if));
            goto end;
        } else {
            HAL_TRACE_DEBUG("pd-enicif:{}:classic: reprogrammed "
                            "table:input_properties index:{} ", __FUNCTION__,
                            hash_idx);
        }
    }

end:
    return ret;
}


#if 0
hal_ret_t
pd_enicif_pd_repgm_inp_prop_l2seg(pd_if_args_t *args, 
                                  l2seg_t *l2seg,
                                  pd_if_l2seg_entry_t *if_l2seg)
{
    hal_ret_t                   ret = HAL_RET_OK;
    input_properties_swkey_t    key;
    input_properties_actiondata data;
    if_t                        *hal_if = args->intf;
    if_t                        *uplink = NULL;
    pd_enicif_t                 *pd_enicif = (pd_enicif_t *)hal_if->pd_if;
    pd_l2seg_t                  *l2seg_pd;
    Hash                        *inp_prop_tbl = NULL;
    uint32_t                    hash_idx = INVALID_INDEXER_INDEX;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(l2seg);
    if (args->pinned_uplink_clsc_change) {
        uplink = find_if_by_handle(args->new_pinned_uplink_clsc);
    } else {
        uplink = find_if_by_handle(hal_if->pinned_up_clsc);
    }

    // Key
    if (if_l2seg) {
        hash_idx = if_l2seg->inp_prop_idx;
    } else {
        hash_idx = pd_enicif->inp_prop_native_l2seg_clsc;
    }

    // Data
    inp_prop.vrf = l2seg_pd->l2seg_ten_hw_id;
    inp_prop.dir = FLOW_DIR_FROM_ENIC;
    inp_prop.l4_profile_idx = L4_PROF_DEFAULT_ENTRY;
    inp_prop.ipsg_enable = 0;
    inp_prop.src_lport = pd_enicif->enic_lport_id;
    inp_prop.dst_lport = if_get_lport_id(uplink);
    inp_prop.flow_miss_action = l2seg_get_bcast_fwd_policy(l2seg);
    inp_prop.flow_miss_idx = l2seg_get_bcast_oif_list(l2seg);
    
    // Update
    ret = inp_prop_tbl->update(hash_idx, &data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}:classic: unable to reprogram for "
                      "(l2seg, upif): ({}, {})",
                      __FUNCTION__, 
                      hal::l2seg_get_l2seg_id(l2seg), 
                      if_get_if_id(hal_if));
        goto end;
    } else {
        HAL_TRACE_DEBUG("pd-enicif:{}:classic: reprogrammed "
                        "table:input_properties index:{} ", __FUNCTION__,
                        hash_idx);
    }

end:
    return ret;
}
#endif




// ----------------------------------------------------------------------------
// Deprogram input properties for classic
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_pd_depgm_inp_prop_l2seg(uint32_t inp_prop_idx)
{
    hal_ret_t                   ret = HAL_RET_OK;
    input_properties_swkey_t    key;
    input_properties_actiondata data;
    Hash                        *inp_prop_tbl = NULL;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);
    
    ret = inp_prop_tbl->remove(inp_prop_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}:classic: unable to deprogram at:{} ",
                      __FUNCTION__, inp_prop_idx);
        goto end;
    } else {
        HAL_TRACE_ERR("pd-enicif:{}:classic: deprogrammed at:{} ",
                      __FUNCTION__, inp_prop_idx);
    }
end:
    return ret;
}



// ----------------------------------------------------------------------------
// Enic reprogramming because of lif params change
// ----------------------------------------------------------------------------
hal_ret_t 
pd_enicif_lif_update(pd_if_lif_upd_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_enicif_t          *pd_enicif;

    HAL_TRACE_DEBUG("pd-enicif:{}: updating lif params for enicif: {}", 
                    __FUNCTION__, if_get_if_id(args->intf));

    pd_enicif = (pd_enicif_t *)args->intf->pd_if;

    // Program Output Mapping 
    ret = pd_enicif_pd_pgm_output_mapping_tbl(pd_enicif, args, 
                                              TABLE_OPER_UPDATE);

    return ret;
}

// ----------------------------------------------------------------------------
// Program Output Mapping Table
// ----------------------------------------------------------------------------
#define om_tmoport data.output_mapping_action_u.output_mapping_set_tm_oport
hal_ret_t
pd_enicif_pd_pgm_output_mapping_tbl(pd_enicif_t *pd_enicif, 
                                    pd_if_lif_upd_args_t *lif_upd,
                                    table_oper_t oper)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint8_t                     tm_oport = 0;
    uint8_t                     p4plus_app_id = 0;
    output_mapping_actiondata   data;
    DirectMap                   *dm_omap = NULL;
    pd_lif_t                    *pd_lif = NULL;

    memset(&data, 0, sizeof(data));

    pd_lif = pd_enicif_get_pd_lif(pd_enicif);

    tm_oport = TM_PORT_DMA;
    p4plus_app_id = P4PLUS_APPTYPE_CLASSIC_NIC;

    data.actionid = OUTPUT_MAPPING_SET_TM_OPORT_ID;
    om_tmoport.nports = 1;
    om_tmoport.egress_mirror_en = 1;
    om_tmoport.egress_port1 = tm_oport;
    om_tmoport.p4plus_app_id = p4plus_app_id;
    om_tmoport.dst_lif = pd_lif->hw_lif_id;
    om_tmoport.rdma_enabled = lif_get_enable_rdma((lif_t *)pd_lif->pi_lif);
    om_tmoport.encap_vlan_id_valid = 1;
    om_tmoport.encap_vlan_id = if_get_encap_vlan((if_t *)pd_enicif->pi_if);
    om_tmoport.vlan_strip = pd_enicif_get_vlan_strip((lif_t *)pd_lif->pi_lif,
                                                     lif_upd);

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    if (oper == TABLE_OPER_INSERT) {
        ret = dm_omap->insert_withid(&data, pd_enicif->enic_lport_id);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-enicif::{}:if_id:{} {} unable to program",
                          __FUNCTION__, 
                          ((if_t*)pd_enicif->pi_if)->if_id,
                          oper);
        } else {
            HAL_TRACE_DEBUG("pd-enicif:{}: lif_id:{} {} programmed output "
                            "mapping at:{}",
                            __FUNCTION__, 
                            ((if_t*)pd_enicif->pi_if)->if_id,
                            oper, pd_enicif->enic_lport_id);
        }
    } else {
        ret = dm_omap->update(pd_enicif->enic_lport_id, &data);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-enicif::{}:if_id:{} {} unable to program",
                          __FUNCTION__, 
                          ((if_t*)pd_enicif->pi_if)->if_id,
                          oper);
        } else {
            HAL_TRACE_DEBUG("pd-enicif:{}: lif_id:{} {} programmed output "
                            "mapping at:{}",
                            __FUNCTION__, 
                            ((if_t*)pd_enicif->pi_if)->if_id,
                            oper, pd_enicif->enic_lport_id);
        }
    }
    return ret;
}

#define inp_prop_mac_vlan_data data.input_properties_mac_vlan_action_u.input_properties_mac_vlan_input_properties_mac_vlan
hal_ret_t
pd_enicif_pgm_inp_prop_mac_vlan_tbl(pd_enicif_t *pd_enicif, nwsec_profile_t *nwsec_prof)
{
    hal_ret_t                                   ret = HAL_RET_OK;
    input_properties_mac_vlan_swkey_t           key;
    input_properties_mac_vlan_swkey_mask_t      mask;
    input_properties_mac_vlan_actiondata        data;
    Tcam                                        *inp_prop_mac_vlan_tbl = NULL;
    mac_addr_t                                  *mac = NULL;
    // uint64_t                                    mac_int = 0;
    // intf::IfEnicType                            enicif_type = intf::IF_ENIC_TYPE_NONE;
    // pd_l2seg_t                                  *pd_l2seg = NULL;
    void                                        *pi_l2seg = NULL;
    types::encapType                            enc_type;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    inp_prop_mac_vlan_tbl = g_hal_state_pd->tcam_table(
                            P4TBL_ID_INPUT_PROPERTIES_MAC_VLAN);
    HAL_ASSERT_RETURN((inp_prop_mac_vlan_tbl != NULL), HAL_RET_ERR);

    // enicif_type = if_get_enicif_type((if_t*)pd_enicif->pi_if);

    // 2 Entries. 1. Host Side Entry 2. Uplink Entry
    // Entry 1: Host Side Entry
#if 0
    if (enicif_type == intf::IF_ENIC_TYPE_USEG) {
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = if_get_useg_vlan((if_t*)pd_enicif->pi_if);
    } else if (enicif_type == intf::IF_ENIC_TYPE_PVLAN) {
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = if_get_iso_vlan((if_t*)pd_enicif->pi_if);
        mac = if_get_mac_addr((if_t*)pd_enicif->pi_if);
        mac_int = MAC_TO_UINT64(*mac);
        memcpy(key.ethernet_srcAddr, &mac_int, 6);
    } else if (enicif_type == intf::IF_ENIC_TYPE_DIRECT){
        // SRIOV
    }
#endif
    key.entry_inactive_input_mac_vlan = 0;
    key.vlan_tag_valid = 1;
    key.vlan_tag_vid = if_get_encap_vlan((if_t*)pd_enicif->pi_if);
    mac = if_get_mac_addr((if_t*)pd_enicif->pi_if);
    memcpy(key.ethernet_srcAddr, *mac, 6);
    memrev(key.ethernet_srcAddr, 6);

    mask.entry_inactive_input_mac_vlan_mask = 0x1;
    mask.vlan_tag_valid_mask = ~(mask.vlan_tag_valid_mask & 0);
    mask.vlan_tag_vid_mask = ~(mask.vlan_tag_vid_mask & 0);
    memset(mask.ethernet_srcAddr_mask, ~0, sizeof(mask.ethernet_srcAddr_mask));

    pd_enicif_inp_prop_form_data(pd_enicif, nwsec_prof, data, true);

    ret = inp_prop_mac_vlan_tbl->insert(&key, &mask, &data, 
                                        &(pd_enicif->inp_prop_mac_vlan_idx_host));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}:unable to program for Host traffic (EnicIf): {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if));
        goto end;
    } else {
        HAL_TRACE_DEBUG("pd-enicif:{}:programmed for Host traffic (EnicIf): {} TcamIdx: {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if), 
                pd_enicif->inp_prop_mac_vlan_idx_host);
    }



    // Entry 2: Uplink Entry - Has to be installed only in EndHost Mode
    //          Used to do Deja-vu check. The src_lif will not match the 
    //          lif on uplink if.
    memset(&data, 0, sizeof(data));
    

    pi_l2seg = if_enicif_get_pi_l2seg((if_t*)pd_enicif->pi_if);
    enc_type = l2seg_get_fab_encap_type((l2seg_t*)pi_l2seg);
    // Direction bit: Handles Encap Vlan vs User Vlan conflicts
    key.control_metadata_uplink = 1;
    mask.control_metadata_uplink_mask = ~(mask.control_metadata_uplink_mask & 0);
    if (enc_type == types::ENCAP_TYPE_DOT1Q) {
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = l2seg_get_fab_encap_val((l2seg_t *)pi_l2seg);
    } else {
        // TODO: What if fab encap is Tunnel ...
        HAL_TRACE_ERR("pd-enicif:{}: FabEncap = VXLAN - NOT IMPLEMENTED",
                      __FUNCTION__);
        return HAL_RET_OK;
    }

    pd_enicif_inp_prop_form_data(pd_enicif, nwsec_prof, data, false);
#if 0
    // Data. Only srclif as this will make the pkt drop
    inp_prop_mac_vlan_data.src_lif_check_en = 1;
    inp_prop_mac_vlan_data.src_lif = if_get_hw_lif_id((if_t*)pd_enicif->pi_if);
#endif

    ret = inp_prop_mac_vlan_tbl->insert(&key, &mask, &data, 
                                        &(pd_enicif->inp_prop_mac_vlan_idx_upl));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}: unable to program for Uplink traffic (EnicIf): {} ret: {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if), ret);
        goto end;
    } else {
        HAL_TRACE_ERR("pd-enicif:{}: programmed for Uplink traffic (EnicIf): {} TcamIdx: {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if), 
                pd_enicif->inp_prop_mac_vlan_idx_upl);
    }

end:
    return ret;
}

hal_ret_t
pd_enicif_inp_prop_form_data (pd_enicif_t *pd_enicif,
                              nwsec_profile_t *nwsec_prof,
                              input_properties_mac_vlan_actiondata &data,
                              bool host_entry)
{
    pd_l2seg_t      *pd_l2seg = NULL;
    hal_ret_t       ret = HAL_RET_OK;

    memset(&data, 0, sizeof(data));

    if (host_entry) {
        pd_l2seg = (pd_l2seg_t *)if_enicif_get_pd_l2seg((if_t*)pd_enicif->pi_if);
        HAL_ASSERT_RETURN((pd_l2seg != NULL), HAL_RET_ERR);

        data.actionid = INPUT_PROPERTIES_MAC_VLAN_INPUT_PROPERTIES_MAC_VLAN_ID;
        inp_prop_mac_vlan_data.vrf = pd_l2seg->l2seg_ten_hw_id;
        inp_prop_mac_vlan_data.dir = FLOW_DIR_FROM_ENIC;
        // inp_prop_mac_vlan_data.ipsg_enable = if_enicif_get_ipsg_en((if_t *)pd_enicif->pi_if);
        inp_prop_mac_vlan_data.ipsg_enable = nwsec_prof ? nwsec_prof->ipsg_en : 0;
        inp_prop_mac_vlan_data.src_lif_check_en = 0; // Enabled only for Deja-vu entry
        inp_prop_mac_vlan_data.src_lif = 0;
        // inp_prop_mac_vlan_data.l4_profile_idx = pd_enicif_get_l4_prof_idx(pd_enicif);
        inp_prop_mac_vlan_data.l4_profile_idx = nwsec_prof ? nwsec_get_nwsec_prof_hw_id(nwsec_prof) : L4_PROF_DEFAULT_ENTRY;
        inp_prop_mac_vlan_data.src_lport = pd_enicif->enic_lport_id;
        inp_prop_mac_vlan_data.flow_miss_action = l2seg_get_bcast_fwd_policy((l2seg_t*)(pd_l2seg->l2seg));
        inp_prop_mac_vlan_data.flow_miss_idx = l2seg_get_bcast_oif_list((l2seg_t*)(pd_l2seg->l2seg));
    } else {
        inp_prop_mac_vlan_data.src_lif_check_en = 1;
        inp_prop_mac_vlan_data.src_lif = if_get_hw_lif_id((if_t*)pd_enicif->pi_if);
    }

    return ret;
}

hal_ret_t
pd_enicif_upd_inp_prop_mac_vlan_tbl (pd_enicif_t *pd_enicif, 
                                     nwsec_profile_t *nwsec_prof)
{
    hal_ret_t                                   ret = HAL_RET_OK;
    input_properties_mac_vlan_actiondata        data;
    Tcam                                        *inp_prop_mac_vlan_tbl = NULL;

    inp_prop_mac_vlan_tbl = g_hal_state_pd->tcam_table(
                            P4TBL_ID_INPUT_PROPERTIES_MAC_VLAN);
    HAL_ASSERT_RETURN((inp_prop_mac_vlan_tbl != NULL), HAL_RET_ERR);

    pd_enicif_inp_prop_form_data(pd_enicif, nwsec_prof, data, true);

    ret = inp_prop_mac_vlan_tbl->update(pd_enicif->inp_prop_mac_vlan_idx_host, &data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-enicif:{}: unable to program for Host traffic (EnicIf): {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if));
        goto end;
    } else {
        HAL_TRACE_DEBUG("pd-enicif:{}: programmed for Host traffic (EnicIf): {} TcamIdx: {}",
                __FUNCTION__, if_get_if_id((if_t*)pd_enicif->pi_if), 
                pd_enicif->inp_prop_mac_vlan_idx_host);
    }

    // Generally there is nothing to update for network side entries. So skipping it

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_make_clone(if_t *hal_if, if_t *clone)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_enicif_t       *pd_enicif_clone = NULL;

    pd_enicif_clone = pd_enicif_alloc_init();
    if (pd_enicif_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_enicif_clone, hal_if->pd_if, sizeof(pd_enicif_t));

    pd_enicif_link_pi_pd(pd_enicif_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_enicif_mem_free(pd_if_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_enicif_t  *upif_pd;

    upif_pd = (pd_enicif_t *)args->intf->pd_if;
    pd_enicif_free(upif_pd);

    return ret;
}

uint32_t
pd_enicif_get_l4_prof_idx(pd_enicif_t *pd_enicif)
{
    if_t        *pi_if = NULL;
    tenant_t    *pi_tenant = NULL;

    pi_if = (if_t *)pd_enicif->pi_if;
    HAL_ASSERT_RETURN(pi_if != NULL, 0);

    pi_tenant = if_get_pi_tenant(pi_if);
    HAL_ASSERT_RETURN(pi_tenant != NULL, 0);

    return ten_get_nwsec_prof_hw_id(pi_tenant);
}

pd_lif_t *
pd_enicif_get_pd_lif(pd_enicif_t *pd_enicif)
{
    if_t        *pi_if = NULL;
    pd_lif_t    *pd_lif = NULL;
    lif_t       *pi_lif = NULL;

    pi_if = (if_t *)pd_enicif->pi_if;
    HAL_ASSERT_RETURN(pi_if != NULL, 0);

    pi_lif = if_get_lif(pi_if);
    HAL_ASSERT(pi_lif != NULL);

    pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
    HAL_ASSERT(pi_lif != NULL);

    return pd_lif;
}

bool
pd_enicif_get_vlan_strip (lif_t *lif, pd_if_lif_upd_args_t *lif_upd)
{
    if (lif_upd && lif_upd->vlan_strip_en_changed) {
        return lif_upd->vlan_strip_en;
    }
    return lif->vlan_strip_en;
}

}    // namespace pd
}    // namespace hal
