#include <hal_lock.hpp>
#include <hal_state_pd.hpp>
#include <pd_api.hpp>
#include <interface_api.hpp>
#include <l2segment_api.hpp>
#include <if_pd.hpp>
#include <l2seg_pd.hpp>
#include <uplinkif_pd.hpp>
#include <uplinkpc_pd.hpp>
#include <l2seg_uplink_pd.hpp>
#include "if_pd_utils.hpp"
#include <p4pd.h>
#include "p4pd_api.hpp"
#include "defines.h"

namespace hal {
namespace pd {


// ----------------------------------------------------------------------------
// Adding L2segment to Uplink
// ----------------------------------------------------------------------------
hal_ret_t
pd_add_l2seg_uplink(pd_l2seg_uplink_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("Handling add l2seg on uplink in PD");

    // Program HW
    ret = l2seg_uplink_program_hw(args);

    return ret;
}

hal_ret_t
l2seg_uplink_program_hw(pd_l2seg_uplink_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;

    // Program Input Properties table
    ret = l2set_uplink_pgm_input_properties_tbl(args);

    return ret;
}

#define inp_prop data.input_properties_action_u.input_properties_input_properties
hal_ret_t
l2set_uplink_pgm_input_properties_tbl(pd_l2seg_uplink_args_t *args)
{
    uint32_t                    uplink_ifpc_id = 0;
    bool                        is_native = FALSE;
    types::encapType            enc_type;
    pd_l2seg_t                  *l2seg_pd;
    Hash                        *inp_prop_tbl = NULL;
    hal_ret_t                   ret = HAL_RET_OK;
    uint32_t                    hash_idx = 0;
    char                        buff[4096] = {0};
    p4pd_error_t                p4_err;
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
    inp_prop.vrf = l2seg_pd->hw_id;
    inp_prop.dir = FLOW_DIR_FROM_UPLINK;
    inp_prop.l4_profile_idx = pd_l2seg_get_l4_prof_idx(l2seg_pd);
    inp_prop.ipsg_enable = l2seg_get_ipsg_en(args->l2seg);

    if (!is_native) {
        // Install one entry
        enc_type = l2seg_get_fab_encap_type(args->l2seg);
        if (enc_type == types::ENCAP_TYPE_DOT1Q) {
            key.vlan_tag_valid = 1;
            key.vlan_tag_vid = l2seg_get_fab_encap_val(args->l2seg);
        } else {
            // TODO: Handle Tunnels
            HAL_TRACE_ERR("PD-ADD-L2SEG-UPIF/PC::{}: Tunnel Case - NOT IMPLEMENTED.",
                          __FUNCTION__);
            return HAL_RET_OK;
        }

        // Insert
        ret = inp_prop_tbl->insert(&key, &data, &hash_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("PD-ADD-L2SEG-UPIF/PC::{}: Unable to program for (l2seg, upif): ({}, {})",
                    __FUNCTION__, hal::l2seg_get_l2seg_id(args->l2seg), if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_DEBUG("PD-ADD-L2SEG-UPIF/PC::{}: Programmed for (l2seg, upif): ({}, {}), "
                            "Vlan: {} hw_lifid: {} hw_l2segid: {}",
                            __FUNCTION__, hal::l2seg_get_l2seg_id(args->l2seg), 
                            if_get_if_id(args->intf), key.vlan_tag_vid, 
                            key.capri_intrinsic_lif, inp_prop.vrf);
            p4_err = p4pd_table_ds_decoded_string_get(P4TBL_ID_INPUT_PROPERTIES, 
                                                      &key, NULL, &data, buff, 
                                                      sizeof(buff));
            HAL_ASSERT(p4_err == P4PD_SUCCESS);
            HAL_TRACE_DEBUG("Index: {} \n {}", hash_idx, buff);
        }

        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = hash_idx;

    } else {
        // Install two entries. 1. Vlan_v:1 & Vlan:0 , 2. Vlan_v:0 & Vlan:0
        // Entry 1:
        key.vlan_tag_valid = 1;
        key.vlan_tag_vid = 0;

        // Insert
        ret = inp_prop_tbl->insert(&key, &data, &hash_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("PD-ADD-L2SEG-UPIF/PC::{}: Unable to program Prio. for (l2seg, upif): ({}, {})",
                    __FUNCTION__, hal::l2seg_get_l2seg_id(args->l2seg), if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_ERR("PD-ADD-L2SEG-UPIF/PC::{}: Programmed Prio. for (l2seg, upif): ({}, {})"
                            "Vlan: {} hw_lifid: {} hw_l2segid: {}",
                            __FUNCTION__, hal::l2seg_get_l2seg_id(args->l2seg), 
                            if_get_if_id(args->intf), key.vlan_tag_vid, 
                            key.capri_intrinsic_lif, inp_prop.vrf);
        }

        // Add to priority array
        l2seg_pd->inp_prop_tbl_idx_pri[uplink_ifpc_id] = hash_idx;

        // Entry 2:
        key.vlan_tag_valid = 0;
        key.vlan_tag_vid = 0;

        // Insert
        ret = inp_prop_tbl->insert(&key, &data, &hash_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("PD-ADD-L2SEG-UPIF::{}: Unable to program NoVlan for (l2seg, upif): ({}, {})",
                    __FUNCTION__, hal::l2seg_get_l2seg_id(args->l2seg), if_get_if_id(args->intf));
            goto end;
        } else {
            HAL_TRACE_ERR("PD-ADD-L2SEG-UPIF::{}: Programmed NoVlan for (l2seg, upif): ({}, {})"
                            "Vlan: {} hw_lifid: {} hw_l2segid: {}",
                            __FUNCTION__, hal::l2seg_get_l2seg_id(args->l2seg), 
                            if_get_if_id(args->intf), key.vlan_tag_vid,
                            key.capri_intrinsic_lif, inp_prop.vrf);
        }

        l2seg_pd->inp_prop_tbl_idx[uplink_ifpc_id] = hash_idx;
    }

end:
    return ret;
}

}    // namespace pd
}    // namespace hal
