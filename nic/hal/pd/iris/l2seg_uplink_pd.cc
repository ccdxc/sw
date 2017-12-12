#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/include/l2segment_api.hpp"
#include "nic/hal/pd/iris/if_pd.hpp"
#include "nic/hal/pd/iris/l2seg_pd.hpp"
#include "nic/hal/pd/iris/uplinkif_pd.hpp"
#include "nic/hal/pd/iris/uplinkpc_pd.hpp"
#include "nic/hal/pd/iris/l2seg_uplink_pd.hpp"
#include "if_pd_utils.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/iris/p4pd/p4pd_defaults.hpp"

namespace hal {
namespace pd {


// ----------------------------------------------------------------------------
// Adding L2segment on Uplink
// ----------------------------------------------------------------------------
hal_ret_t
pd_add_l2seg_uplink(pd_l2seg_uplink_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("pd-l2seg<->uplink:{}:pd call", __FUNCTION__); 

    // Program HW
    ret = l2seg_uplink_program_hw(args);

    return ret;
}

// ----------------------------------------------------------------------------
// Deleting L2segment on Uplink
// ----------------------------------------------------------------------------
hal_ret_t
pd_del_l2seg_uplink(pd_l2seg_uplink_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("pd-l2seg<->uplink:{}:pd call", __FUNCTION__); 

    // Program HW
    ret = l2seg_uplink_deprogram_hw(args);

    return ret;
}

// ----------------------------------------------------------------------------
// Programming HW 
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_program_hw(pd_l2seg_uplink_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    nwsec_profile_t     *pi_nwsec = NULL;

    pi_nwsec = (nwsec_profile_t *)l2seg_get_pi_nwsec((l2seg_t *)args->l2seg);

    // Program Input Properties table
    ret = l2seg_uplink_pgm_input_properties_tbl(args, pi_nwsec);

    return ret;
}

// ----------------------------------------------------------------------------
// De-Programming HW 
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_deprogram_hw(pd_l2seg_uplink_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;

    // Program Input Properties table
    ret = l2seg_uplink_depgm_input_properties_tbl(args);

    return ret;
}

// ----------------------------------------------------------------------------
// Program input properties table
// ----------------------------------------------------------------------------
#define inp_prop data.input_properties_action_u.input_properties_input_properties
hal_ret_t
l2seg_uplink_pgm_input_properties_tbl(pd_l2seg_uplink_args_t *args, nwsec_profile_t *nwsec_prof)
{
    uint32_t                    uplink_ifpc_id = 0;
    bool                        is_native = FALSE;
    types::encapType            enc_type;
    pd_l2seg_t                  *l2seg_pd;
    Hash                        *inp_prop_tbl = NULL;
    hal_ret_t                   ret = HAL_RET_OK;
    uint32_t                    hash_idx = 0;
    l2seg_t                     *infra_pi_l2seg = NULL;
    input_properties_swkey_t    key;
    input_properties_actiondata data;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(args->l2seg);
    is_native = is_l2seg_native(args->l2seg, args->intf);
    uplink_ifpc_id = if_get_uplink_ifpc_id(args->intf);

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    key.capri_intrinsic_lif = if_get_hw_lif_id(args->intf);

    // Data
    inp_prop.vrf = l2seg_pd->l2seg_fl_lkup_id;
    inp_prop.dir = FLOW_DIR_FROM_UPLINK;
    inp_prop.l4_profile_idx = nwsec_prof ? nwsec_get_nwsec_prof_hw_id(nwsec_prof) : L4_PROF_DEFAULT_ENTRY;
    inp_prop.ipsg_enable = nwsec_prof ? nwsec_prof->ipsg_en : 0;
    // Always disable ipsg on uplinks
    // inp_prop.ipsg_enable = 0;
    inp_prop.src_lport = if_get_uplink_lport_id(args->intf);
    inp_prop.flow_miss_action = l2seg_get_bcast_fwd_policy(args->l2seg);
    inp_prop.flow_miss_idx = l2seg_get_bcast_oif_list(args->l2seg);

    if ((is_forwarding_mode_host_pinned()) &&
        (hal::l2seg_get_pinned_uplink(args->l2seg) != hal::if_get_hal_handle(args->intf))) {
        inp_prop.allow_flood = 0;
    } else{
        inp_prop.allow_flood = 1;
    }

    key.entry_inactive_input_properties = 0;
    if (!is_native) {
        // Install one entry
        enc_type = l2seg_get_wire_encap_type(args->l2seg);
        if (enc_type == types::ENCAP_TYPE_DOT1Q) {
            key.vlan_tag_valid = 1;
            key.vlan_tag_vid = l2seg_get_wire_encap_val(args->l2seg);
        } else if (enc_type == types::ENCAP_TYPE_VXLAN) {
            infra_pi_l2seg = hal::l2seg_get_infra_l2seg();
            if_l2seg_get_encap(args->intf, infra_pi_l2seg, &key.vlan_tag_valid, &key.vlan_tag_vid);
            // TODO: If infra is native on this uplink, do we have to install two entries ? 
            key.tunnel_metadata_tunnel_type = INGRESS_TUNNEL_TYPE_VXLAN;
            key.tunnel_metadata_tunnel_vni = l2seg_get_wire_encap_val(args->l2seg);
            HAL_TRACE_DEBUG("pd-add-l2seg-upif/pc::{}: encap_type vxlan tunnel_vni: {}",
                            __FUNCTION__, key.tunnel_metadata_tunnel_vni);
            /* No IPSG check for packets coming in on tunnel interfaces */
            inp_prop.ipsg_enable = FALSE;
        }

        HAL_TRACE_DEBUG("{}: input properties (lif, vlan_v, vlan) : "
                        "({}, {}, {}) => ",
                        __FUNCTION__, key.capri_intrinsic_lif, 
                        key.vlan_tag_valid, key.vlan_tag_vid);

        // Insert
        ret = inp_prop_tbl->insert(&key, &data, &hash_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-l2seg<->uplink:{}: Unable to program for "
                          "(l2seg, upif): ({}, {})",
                          __FUNCTION__, 
                          hal::l2seg_get_l2seg_id(args->l2seg), 
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("pd-l2seg<->uplink:{}: Programmed "
                            "table:input_properties index:{} ", __FUNCTION__,
                            hash_idx);
        }

        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = hash_idx;

    } else {
        // Install two entries. 1. Vlan_v:1 & Vlan:0 , 2. Vlan_v:0 & Vlan:0
        // Entry 1:
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = 0;

        HAL_TRACE_DEBUG("{}: input properties (lif, vlan_v, vlan) : "
                        "({}, {}, {}) => ",
                        __FUNCTION__, key.capri_intrinsic_lif, 
                        key.vlan_tag_valid, key.vlan_tag_vid);
        // Insert
        ret = inp_prop_tbl->insert(&key, &data, &hash_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-l2seg<->uplink:{}:unable to program "
                          "prio. entry for (l2seg, upif): ({}, {})",
                          __FUNCTION__, 
                          hal::l2seg_get_l2seg_id(args->l2seg), 
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("pd-l2seg<->uplink:{}:programmed prio. entry "
                            "table:input_properties index:{} ", __FUNCTION__,
                            hash_idx);
        }

        // Add to priority array
        l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id] = hash_idx;

        // Entry 2:
        key.vlan_tag_valid = 0;
        key.vlan_tag_vid = 0;

        HAL_TRACE_DEBUG("{}: input properties (lif, vlan_v, vlan) : "
                        "({}, {}, {}) => ",
                        __FUNCTION__, key.capri_intrinsic_lif, 
                        key.vlan_tag_valid, key.vlan_tag_vid);
        // Insert
        ret = inp_prop_tbl->insert(&key, &data, &hash_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-l2seg<->uplink:{}:unable to program "
                          "untag entry for (l2seg, upif): ({}, {})",
                          __FUNCTION__, 
                          hal::l2seg_get_l2seg_id(args->l2seg), 
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("pd-l2seg<->uplink:{}:programmed untagged entry "
                            "table:input_properties index:{} ", __FUNCTION__,
                            hash_idx);
        }

        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = hash_idx;
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram input propterties table
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_depgm_input_properties_tbl (pd_l2seg_uplink_args_t *args)
{
    hal_ret_t                   ret = HAL_RET_OK;
    Hash                        *inp_prop_tbl = NULL;
    uint32_t                    uplink_ifpc_id = 0;
    pd_l2seg_t                  *l2seg_pd =  NULL;

    uplink_ifpc_id = if_get_uplink_ifpc_id(args->intf);
    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(args->l2seg);

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);
    
    if (l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] != INVALID_INDEXER_INDEX) {
        ret = inp_prop_tbl->remove(l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-l2seg<->uplink:{}:unable to deprogram "
                          "table:input_properties index:{}",
                          __FUNCTION__,
                          l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        } else {
            HAL_TRACE_ERR("pd-l2seg<->uplink:{}:deprogrammed "
                          "table:input_properties index:{}", 
                          __FUNCTION__, 
                          l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        }
        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = INVALID_INDEXER_INDEX;
    }

    if (l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id] != 
            INVALID_INDEXER_INDEX) {
        ret = inp_prop_tbl->
            remove(l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id]);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-l2seg<->uplink:{}:unable to deprogram pri entry "
                          "table:input_properties index:{}",
                          __FUNCTION__,
                          l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id]);
        } else {
            HAL_TRACE_ERR("pd-l2seg<->uplink:{}:deprogrammed pri entry "
                          "table:input_properties index:{}", 
                          __FUNCTION__, 
                          l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id]);
        }
        l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id] = INVALID_INDEXER_INDEX;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Form data for input properties table
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_inp_prop_form_data (pd_l2seg_uplink_args_t *args,
                                 nwsec_profile_t *nwsec_prof,
                                 input_properties_actiondata &data)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_l2seg_t          *l2seg_pd;
    bool                is_native = FALSE;
    types::encapType    enc_type;

    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(args->l2seg);
    is_native = is_l2seg_native(args->l2seg, args->intf);

    memset(&data, 0, sizeof(data));

    // Data
    inp_prop.vrf = l2seg_pd->l2seg_fl_lkup_id;
    inp_prop.dir = FLOW_DIR_FROM_UPLINK;
    inp_prop.l4_profile_idx = nwsec_prof ? nwsec_get_nwsec_prof_hw_id(nwsec_prof) : L4_PROF_DEFAULT_ENTRY;
    inp_prop.ipsg_enable = nwsec_prof ? nwsec_prof->ipsg_en : 0;
    inp_prop.src_lport = if_get_uplink_lport_id(args->intf);
    inp_prop.flow_miss_action = l2seg_get_bcast_fwd_policy(args->l2seg);
    inp_prop.flow_miss_idx = l2seg_get_bcast_oif_list(args->l2seg);

    if ((is_forwarding_mode_host_pinned()) &&
        (hal::l2seg_get_pinned_uplink(args->l2seg) != hal::if_get_hal_handle(args->intf))) {
        inp_prop.allow_flood = 0;
    } else {
        inp_prop.allow_flood = 1;
    }

    if (!is_native) {
        enc_type = l2seg_get_wire_encap_type(args->l2seg);
        if (enc_type == types::ENCAP_TYPE_VXLAN) {
            /* No IPSG check for packets coming in on tunnel interfaces */
            inp_prop.ipsg_enable = FALSE;
        }
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Update data for input properties table
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_upd_input_properties_tbl (pd_l2seg_uplink_args_t *args,
                                       nwsec_profile_t *nwsec_prof)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd_l2seg_t                  *l2seg_pd;
    bool                        is_native = FALSE;
    uint32_t                    uplink_ifpc_id = 0;
    Hash                        *inp_prop_tbl = NULL;
    input_properties_actiondata data;

    l2seg_uplink_inp_prop_form_data(args, nwsec_prof, data);

    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(args->l2seg);
    is_native = is_l2seg_native(args->l2seg, args->intf);
    uplink_ifpc_id = if_get_uplink_ifpc_id(args->intf);

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    if (!is_native) {
        // Update one entry
        ret = inp_prop_tbl->update(l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id], &data);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-l2seg<->uplink:{}: Unable to program for "
                          "(l2seg, upif): ({}, {})",
                          __FUNCTION__, 
                          hal::l2seg_get_l2seg_id(args->l2seg), 
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("pd-l2seg<->uplink:{}: Programmed "
                            "table:input_properties index:{} ", __FUNCTION__,
                            l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        }
    } else {
        // Update Priority tagged entry
        ret = inp_prop_tbl->update(
                l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id], &data);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-l2seg<->uplink:{}:unable to program "
                          "prio. entry for (l2seg, upif): ({}, {})",
                          __FUNCTION__, 
                          hal::l2seg_get_l2seg_id(args->l2seg), 
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("pd-l2seg<->uplink:{}:programmed prio. entry "
                            "table:input_properties index:{} ", __FUNCTION__,
                            l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id]);
        }

        // Update Untagged entry
        ret = inp_prop_tbl->update(l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id], &data);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-l2seg<->uplink:{}:unable to program "
                          "untag entry for (l2seg, upif): ({}, {})",
                          __FUNCTION__, 
                          hal::l2seg_get_l2seg_id(args->l2seg), 
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("pd-l2seg<->uplink:{}:programmed untagged entry "
                            "table:input_properties index:{} ", __FUNCTION__,
                            l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        }
    }

end:
        return ret;
}

}    // namespace pd
}    // namespace hal
