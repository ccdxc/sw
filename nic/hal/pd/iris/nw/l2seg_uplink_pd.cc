#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment_api.hpp"
#include "nic/hal/pd/iris/nw/if_pd.hpp"
#include "nic/hal/pd/iris/nw/enicif_pd.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"
#include "nic/hal/pd/iris/nw/uplinkif_pd.hpp"
#include "nic/hal/pd/iris/nw/uplinkpc_pd.hpp"
#include "nic/hal/pd/iris/nw/l2seg_uplink_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/pd/iris/p4pd_defaults.hpp"

namespace hal {
namespace pd {


// ----------------------------------------------------------------------------
// Adding L2segment on Uplink
// ----------------------------------------------------------------------------
hal_ret_t
pd_add_l2seg_uplink(pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_add_l2seg_uplink_args_t *args = pd_func_args->pd_add_l2seg_uplink;

    HAL_TRACE_DEBUG("pd-l2seg<->uplink:pd call");

    // Program HW
    ret = l2seg_uplink_program_hw(args);

    return ret;
}

// ----------------------------------------------------------------------------
// Deleting L2segment on Uplink
// ----------------------------------------------------------------------------
hal_ret_t
pd_del_l2seg_uplink(pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_del_l2seg_uplink_args_t *args = pd_func_args->pd_del_l2seg_uplink;

    HAL_TRACE_DEBUG("pd-l2seg<->uplink:pd call");

    // Program HW
    ret = l2seg_uplink_deprogram_hw(args);

    return ret;
}

// ----------------------------------------------------------------------------
// Programming HW
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_program_hw(pd_add_l2seg_uplink_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;

    // Program Input Properties table
    ret = l2seg_uplink_pgm_input_properties_tbl(args);

    return ret;
}

// ----------------------------------------------------------------------------
// De-Programming HW
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_deprogram_hw(pd_del_l2seg_uplink_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;

    // Program Input Properties table
    ret = l2seg_uplink_depgm_input_properties_tbl(args);

    return ret;
}

// ----------------------------------------------------------------------------
// Program input properties table
// ----------------------------------------------------------------------------
#define inp_prop data.action_u.input_properties_input_properties
hal_ret_t
l2seg_uplink_pgm_input_properties_tbl(pd_add_l2seg_uplink_args_t *args)
{
    uint32_t                                uplink_ifpc_id = 0;
    bool                                    is_native = FALSE;
    types::encapType                        enc_type;
    pd_l2seg_t                              *l2seg_pd;
    sdk_hash                                *inp_prop_tbl = NULL;
    hal_ret_t                               ret = HAL_RET_OK;
    sdk_ret_t                               sdk_ret;
    uint32_t                                hash_idx = 0;
    l2seg_t                                 *infra_pi_l2seg = NULL;
    input_properties_swkey_t                key;
    input_properties_otcam_swkey_mask_t     *key_mask = NULL;
    input_properties_actiondata_t           data;
    bool                                    direct_to_otcam = false;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(args->l2seg);
    is_native = is_l2seg_native(args->l2seg, args->intf);
    uplink_ifpc_id = if_get_uplink_ifpc_id(args->intf);

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    key.capri_intrinsic_lif = if_get_hw_lif_id(args->intf);
    key.entry_inactive_input_properties = 0;

    // form data
    l2seg_uplink_inp_prop_form_data(args->l2seg, args->intf, 0, NULL, 0, NULL, data);

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
            HAL_TRACE_DEBUG("pd-add-l2seg-upif/pc::encap_type vxlan tunnel_vni: {}",
                            key.tunnel_metadata_tunnel_vni);
        }

        HAL_TRACE_DEBUG("input properties (lif, vlan_v, vlan) : "
                        "({}, {}, {}) => ",
                        key.capri_intrinsic_lif,
                        key.vlan_tag_valid, key.vlan_tag_vid);

        // Insert
        sdk_ret = inp_prop_tbl->insert(&key, &data, &hash_idx, key_mask,
                                       direct_to_otcam);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to program for "
                          "(l2seg, upif): ({}, {})",
                          hal::l2seg_get_l2seg_id(args->l2seg),
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("Programmed "
                            "table:input_properties index:{} ", hash_idx);
        }

        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = hash_idx;

    } else {
        // Install two entries. 1. Vlan_v:1 & Vlan:0 , 2. Vlan_v:0 & Vlan:0
        // Entry 1:
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = 0;

        HAL_TRACE_DEBUG("input properties (lif, vlan_v, vlan) lkup_id:{}: "
                        "({}, {}, {}) => ",
                        l2seg_pd->l2seg_fl_lkup_id,
			key.capri_intrinsic_lif,
                        key.vlan_tag_valid, key.vlan_tag_vid);
        // Insert
        sdk_ret = inp_prop_tbl->insert(&key, &data, &hash_idx,
                                       key_mask, direct_to_otcam);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program "
                          "prio. entry for (l2seg, upif): ({}, {})",
                          hal::l2seg_get_l2seg_id(args->l2seg),
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("Programmed prio. entry "
                            "table:input_properties index:{} ",
                            hash_idx);
        }

        // Add to priority array
        l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id] = hash_idx;

        // Entry 2:
        key.vlan_tag_valid = 0;
        key.vlan_tag_vid = 0;

        HAL_TRACE_DEBUG("input properties (lif, vlan_v, vlan) : "
                        "({}, {}, {}) => ",
                        key.capri_intrinsic_lif,
                        key.vlan_tag_valid, key.vlan_tag_vid);
        // Insert
        sdk_ret = inp_prop_tbl->insert(&key, &data, &hash_idx,
                                       key_mask, direct_to_otcam);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program "
                          "untag entry for (l2seg, upif): ({}, {})",
                          hal::l2seg_get_l2seg_id(args->l2seg),
                          if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("programmed untagged entry "
                            "table:input_properties index:{} ",
                            hash_idx);
        }

        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = hash_idx;
    }

    if (key_mask) {
        HAL_FREE(HAL_MEM_ALLOC_INP_PROP_KEY_MASK, key_mask);
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram input propterties table
// ----------------------------------------------------------------------------
hal_ret_t
l2seg_uplink_depgm_input_properties_tbl (pd_del_l2seg_uplink_args_t *args)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    sdk_hash                    *inp_prop_tbl = NULL;
    uint32_t                    uplink_ifpc_id = 0;
    pd_l2seg_t                  *l2seg_pd =  NULL;

    uplink_ifpc_id = if_get_uplink_ifpc_id(args->intf);
    l2seg_pd = (pd_l2seg_t *)hal::l2seg_get_pd(args->l2seg);

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    if (l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] != INVALID_INDEXER_INDEX) {
        sdk_ret = inp_prop_tbl->remove(l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to deprogram "
                          "table:input_properties index:{}",
                          l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        } else {
            HAL_TRACE_DEBUG("deprogrammed "
                          "table:input_properties index:{}",
                          l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id]);
        }
        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = INVALID_INDEXER_INDEX;
    }

    if (l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id] !=
            INVALID_INDEXER_INDEX) {
        sdk_ret = inp_prop_tbl->
            remove(l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id]);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to deprogram pri entry "
                          "table:input_properties index:{}",
                          l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id]);
        } else {
            HAL_TRACE_ERR("deprogrammed pri entry "
                          "table:input_properties index:{}",
                          l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id]);
        }
        l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id] = INVALID_INDEXER_INDEX;
    }

    return ret;
}

}    // namespace pd
}    // namespace hal
