// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/nw/vrf_pd.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"

namespace hal {
namespace pd {

#define inp_prop data.action_u.input_properties_input_properties

// prototypes
hal_ret_t vrf_pd_alloc_res(pd_vrf_t *pd_ten);
hal_ret_t vrf_pd_dealloc_res(pd_vrf_t *vrf_pd);
hal_ret_t vrf_pd_cleanup(pd_vrf_t *vrf_pd);
void link_pi_pd(pd_vrf_t *pd_ten, vrf_t *pi_ten);
void delink_pi_pd(pd_vrf_t *pd_ten, vrf_t *pi_ten);
hal_ret_t vrf_pd_program_hw(pd_vrf_t *vrf_pd,
                            bool is_upgrade = false);
hal_ret_t vrf_pd_pgm_inp_prop_tbl(pd_vrf_t *vrf_pd,
                                  bool is_upgrade);
hal_ret_t vrf_pd_deprogram_hw(pd_vrf_t *vrf_pd);
hal_ret_t vrf_pd_depgm_inp_prop_tbl(pd_vrf_t *vrf_pd);
hal_ret_t vrf_pd_program_gipo_prefix(pd_vrf_t *vrf_pd,
                                     ip_prefix_t *gipo_prefix,
                                     bool is_upgrade = false);
hal_ret_t vrf_pd_deprogram_gipo_prefix(pd_vrf_t *vrf_pd);
hal_ret_t vrf_pd_alloc_cpuid(pd_vrf_t *pd_vrf);
hal_ret_t vrf_pd_dealloc_cpuid(pd_vrf_t *vrf_pd);
hal_ret_t vrf_pd_add_to_db(pd_vrf_t *pd_vrf, hal_handle_t handle);
hal_ret_t vrf_pd_del_from_db(pd_vrf_t *pd_vrf);

//-----------------------------------------------------------------------------
// PD vrf Create
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_vrf_create_args_t *args = pd_func_args->pd_vrf_create;
    pd_vrf_t                *vrf_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);

    // allocate PD vrf state
    vrf_pd = vrf_pd_alloc_init();
    if (vrf_pd == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // link pi & pd
    link_pi_pd(vrf_pd, args->vrf);

    // allocate resources
    ret = vrf_pd_alloc_res(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to allocated resources");
        goto end;
    }

    // program hw
    ret = vrf_pd_program_hw(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program hw");
        goto end;
    }

    // add to flow lookup id ht
    ret = vrf_pd_add_to_db(vrf_pd, ((vrf_t *)(vrf_pd->vrf))->hal_handle);
    if (ret != HAL_RET_OK) {
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to create vrf PD. ret: {}", ret);
        // vrf_pd_cleanup(vrf_pd);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD vrf Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret     = HAL_RET_OK;
    pd_vrf_update_args_t *args = pd_func_args->pd_vrf_update;
    pd_vrf_t    *vrf_pd = (pd_vrf_t *)args->vrf->pd;

    if (args->gipo_prefix_change) {
        // de-program input mapping native & tunnel
        ret = vrf_pd_deprogram_gipo_prefix(vrf_pd);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to deprogram gipo pfx term. ret:{}", ret);
            goto end;
        }

        // program input mapping native & tunnel with new gipo prefix
        ret = vrf_pd_program_gipo_prefix(vrf_pd, args->new_gipo_prefix);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to program gipo pfx term. ret:{}", ret);
            goto end;
        }
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD vrf Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_vrf_delete_args_t *args = pd_func_args->pd_vrf_delete;
    pd_vrf_t    *vrf_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->vrf != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->vrf->pd != NULL), HAL_RET_INVALID_ARG);

    vrf_pd = (pd_vrf_t *)args->vrf->pd;

    // deprogram HW
    ret = vrf_pd_deprogram_hw(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to deprogram hw");
    }

    // remove from db
    ret = vrf_pd_del_from_db(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to delete from db");
        goto end;
    }

    // dealloc resources and free
    ret = vrf_pd_cleanup(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed pd vrf cleanup");
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// pd vrf get
// ----------------------------------------------------------------------------
hal_ret_t
pd_vrf_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_vrf_get_args_t *args = pd_func_args->pd_vrf_get;
    vrf_t           *vrf = args->vrf;
    pd_vrf_t        *vrf_pd = (pd_vrf_t *)vrf->pd;
    VrfGetResponse  *rsp = args->rsp;

    auto vrf_info = rsp->mutable_status()->mutable_epd_status();
    vrf_info->set_hw_vrf_id(vrf_pd->vrf_hw_id);
    vrf_info->set_vrf_lookup_id(vrf_pd->vrf_fl_lkup_id);
    vrf_info->set_vrf_vlan_id_cpu(vrf_pd->vrf_fromcpu_vlan_id);
    vrf_info->set_inp_prop_cpu_idx(vrf_pd->inp_prop_tbl_cpu_idx);
    for (int i = 0; i < HAL_PD_INP_MAP_ENTRIES; i++) {
        vrf_info->add_gipo_inp_map_nat_idx(vrf_pd->gipo_imn_idx[i]);
        vrf_info->add_gipo_inp_map_tnl_idx(vrf_pd->gipo_imt_idx[i]);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// pd vrf restore from response
// ----------------------------------------------------------------------------
hal_ret_t
pd_vrf_restore_data (pd_vrf_restore_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    vrf_t           *vrf = args->vrf;
    pd_vrf_t        *vrf_pd = (pd_vrf_t *)vrf->pd;

    auto vrf_info = args->vrf_status->epd_status();
    vrf_pd->vrf_hw_id = vrf_info.hw_vrf_id();
    vrf_pd->vrf_fl_lkup_id = vrf_info.vrf_lookup_id();
    vrf_pd->vrf_fromcpu_vlan_id = vrf_info.vrf_vlan_id_cpu();
    vrf_pd->inp_prop_tbl_cpu_idx = vrf_info.inp_prop_cpu_idx();
    for (int i = 0; i < HAL_PD_INP_MAP_ENTRIES; i++) {
        vrf_pd->gipo_imn_idx[i] = vrf_info.gipo_inp_map_tnl_idx(i);
        vrf_pd->gipo_imt_idx[i] = vrf_info.gipo_inp_map_tnl_idx(i);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// pd vrf restore
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_restore (pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret;
    pd_vrf_restore_args_t *args = pd_func_args->pd_vrf_restore;
    pd_vrf_t    *vrf_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("Restoring pd state for vrf {}", args->vrf->vrf_id);

    // allocate PD vrf state
    vrf_pd = vrf_pd_alloc_init();
    if (vrf_pd == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // link pi & pd
    link_pi_pd(vrf_pd, args->vrf);

    ret = pd_vrf_restore_data(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to restore pd data for vrf: {}, err: {}",
                      args->vrf->vrf_id, ret);
        goto end;
    }


    // This call will just populate table libs and calls to HW will be
    // a NOOP in p4pd code
    ret = vrf_pd_program_hw(vrf_pd, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to program hw");
        goto end;
    }

    // add to flow lookup id ht
    ret = vrf_pd_add_to_db(vrf_pd, ((vrf_t *)(vrf_pd->vrf))->hal_handle);
    if (ret != HAL_RET_OK) {
        goto end;
    }

end:

    if (ret != HAL_RET_OK) {
        vrf_pd_cleanup(vrf_pd);
    }

    return ret;
}

//------------------------------------------------------------------------------
// insert the vrf into flow lookup id DB
//------------------------------------------------------------------------------
hal_ret_t
vrf_pd_add_to_db (pd_vrf_t *pd_vrf, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    // ht entry alloc
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // ht insert
    entry->handle_id = handle;
    sdk_ret = g_hal_state_pd->flow_lkupid_ht()->insert_with_key(&pd_vrf->
                                                                vrf_fl_lkup_id,
                                                                entry,
                                                                &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add hw id to handle mapping, err : {}",
                      sdk_ret);
        hal::pd::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    HAL_TRACE_DEBUG("Insert flow_lkupid_ht fl_lkup_id:{}",
                    pd_vrf->vrf_fl_lkup_id);

    return ret;
}

//------------------------------------------------------------------------------
// delete a vrf from hwid database
//------------------------------------------------------------------------------
hal_ret_t
vrf_pd_del_from_db (pd_vrf_t *pd_vrf)
{
    hal_handle_id_ht_entry_t    *entry = NULL;

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state_pd->flow_lkupid_ht()->
        remove(&pd_vrf->vrf_fl_lkup_id);

    if (entry) {
        // free up
        hal::pd::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    HAL_TRACE_DEBUG("Remove flow_lkupid_ht fl_lkup_id:{}",
                    pd_vrf->vrf_fl_lkup_id);

    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// DeProgram HW
//----------------------------------------------------------------------------
hal_ret_t
vrf_pd_deprogram_hw (pd_vrf_t *vrf_pd)
{
    hal_ret_t            ret = HAL_RET_OK;

    // De-Program Input properties Table
    ret = vrf_pd_depgm_inp_prop_tbl(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to deprogram hw");
    }

    // De-Program Input Mapping Native & Tunnel
    if (((vrf_t *)(vrf_pd->vrf))->vrf_type == types::VRF_TYPE_INFRA) {
        ret = vrf_pd_deprogram_gipo_prefix(vrf_pd);
    }

    return ret;
}

//----------------------------------------------------------------------------
// DeProgram input propterties table for cpu tx traffic
//----------------------------------------------------------------------------
hal_ret_t
vrf_pd_depgm_inp_prop_tbl (pd_vrf_t *vrf_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    sdk_hash                    *inp_prop_tbl = NULL;


    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    if (vrf_pd->inp_prop_tbl_cpu_idx != INVALID_INDEXER_INDEX) {
        sdk_ret = inp_prop_tbl->remove(vrf_pd->inp_prop_tbl_cpu_idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to deprogram from_cpu_entry input properties"
                          "i at:{}", vrf_pd->inp_prop_tbl_cpu_idx);
        } else {
            HAL_TRACE_DEBUG("Deprogrammed from_cpu_entry input properties "
                            "at:{}", vrf_pd->inp_prop_tbl_cpu_idx);
        }
        vrf_pd->inp_prop_tbl_cpu_idx = INVALID_INDEXER_INDEX;
    }

    return ret;
}


//----------------------------------------------------------------------------
// program HW
//----------------------------------------------------------------------------
hal_ret_t
vrf_pd_program_hw (pd_vrf_t *vrf_pd, bool is_upgrade)
{
    hal_ret_t            ret;

    // program input properties table, for cpu traffic
    ret = vrf_pd_pgm_inp_prop_tbl(vrf_pd, is_upgrade);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Unable to program inp. prop table for"
                        "cpu tx traffic. ret: {}", ret);
        goto end;
    }

    // program input mapping native & tunnel tables: For MyTep Termination
    if (((vrf_t *)(vrf_pd->vrf))->vrf_type == types::VRF_TYPE_INFRA) {
        ret = vrf_pd_program_gipo_prefix(vrf_pd,
                                         &((vrf_t*)(vrf_pd->vrf))->
                                         gipo_prefix, is_upgrade);
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// program input propterties table for cpu tx traffic
//----------------------------------------------------------------------------
hal_ret_t
vrf_pd_pgm_inp_prop_tbl (pd_vrf_t *vrf_pd, bool is_upgrade)
{
    hal_ret_t                   ret           = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    sdk_hash                    *inp_prop_tbl = NULL;
    input_properties_swkey_t    key           = { 0 };
    input_properties_actiondata_t data          = { 0 };
    nwsec_profile_t             *nwsec_prof = NULL;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    nwsec_prof = find_nwsec_profile_by_id(L4_PROFILE_HOST_DEFAULT);

    key.capri_intrinsic_lif   = HAL_LIF_CPU;
    key.vlan_tag_valid        = 1;
    key.vlan_tag_vid          = vrf_pd->vrf_fromcpu_vlan_id;
    inp_prop.dir              = FLOW_DIR_FROM_UPLINK;
    inp_prop.vrf              = vrf_pd->vrf_fl_lkup_id;
    inp_prop.l4_profile_idx   = nwsec_get_nwsec_prof_hw_id(nwsec_prof);
    inp_prop.ipsg_enable      = 0;
    inp_prop.src_lport        = 0;
    inp_prop.mdest_flow_miss_action = 0;
    inp_prop.flow_miss_idx    = 0;

    if (is_upgrade) {
        // insert
        sdk_ret = inp_prop_tbl->insert_withid(&key, &data,
                                              vrf_pd->inp_prop_tbl_cpu_idx);
    } else {
        // insert
        sdk_ret = inp_prop_tbl->insert(&key, &data, &vrf_pd->inp_prop_tbl_cpu_idx);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to program from_cpu_entry input properties. ret: {}", ret);
        if (ret == HAL_RET_ENTRY_EXISTS) {
            // If hash lib returns entry exists, return hw prog error. Otherwise
            // entry exists means that vrf was already created.
            ret = HAL_RET_HW_PROG_ERR;
        }
        goto end;
    } else {
        HAL_TRACE_DEBUG("Programmed from_cpu_entry input properties at:{}",
                        vrf_pd->inp_prop_tbl_cpu_idx);
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// program input mapping
//----------------------------------------------------------------------------
hal_ret_t
pd_vrf_program_input_mapping_table(ip_prefix_t *ip_prefix,
                                   uint8_t tunnel_type,
                                   bool inner_v4_vld,
                                   bool inner_v6_vld,
                                   uint8_t actionid,
                                   p4pd_table_id tbl_id, uint32_t *idx,
                                   bool is_upgrade)
{
    hal_ret_t                           ret      = HAL_RET_OK;
    sdk_ret_t                           sdk_ret;
    input_mapping_native_swkey_t        key      = {0};
    input_mapping_native_swkey_mask_t   mask     = {0};
    input_mapping_native_actiondata_t     data     = {0};
    tcam                                *tcam;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    SDK_ASSERT(tcam != NULL);
    /* Input mapping native and tunneled tables have the same key, mask and data
     * So, we can populate the structs and typecast accordingly */
    memset(&key, 0, sizeof(input_mapping_native_swkey_t));
    memset(&mask, 0, sizeof(input_mapping_native_swkey_mask_t));
    key.inner_ipv4_valid                  = (uint8_t)inner_v4_vld;
    key.inner_ipv6_valid                  = (uint8_t)inner_v6_vld;
    key.tunnel_metadata_tunnel_type       = tunnel_type;
    mask.inner_ipv4_valid_mask            = 0xFF;
    mask.inner_ipv6_valid_mask            = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    if (ip_prefix->addr.af == IP_AF_IPV4) {
        key.ipv4_valid = 1;
        key.ipv4_dstAddr = ip_prefix->addr.addr.v4_addr;
        mask.ipv4_valid_mask = 0xFF;
        mask.ipv4_dstAddr_mask = ipv4_prefix_len_to_mask(ip_prefix->len);
    } else {
        HAL_TRACE_ERR("IPv6 TEP is not supprted");
        return HAL_RET_NOT_SUPPORTED;
    }
    data.action_id = actionid;
    if (is_upgrade) {
        sdk_ret       = tcam->insert_withid(&key, &mask, &data, *idx);
    } else {
        sdk_ret       = tcam->insert(&key, &mask, &data, idx);
    }
    ret           = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_ENTRY_EXISTS) {
        /* Entry already exists. Can be skipped */
        *idx = INVALID_INDEXER_INDEX;
    } else {
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Input mapping table failure, idx : {}, err : {}",
                          *idx, ret);
            return ret;
        }
    }
    HAL_TRACE_DEBUG("Input mapping table tcam for prefix:{}, idx :{}, ret:{}",
                    ippfx2str(ip_prefix), *idx, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Program input mapping table to terminate GIPo tunnels
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_del_gipo_termination_prefix(pd_vrf_t *vrf_pd,
                                   p4pd_table_id tbl_id)
{
    tcam         *tcam   = NULL;
    uint32_t     *arr    = NULL;
    hal_ret_t    ret     = HAL_RET_OK;
    sdk_ret_t    sdk_ret = sdk::SDK_RET_OK;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    if (tcam == NULL) {
        HAL_TRACE_ERR("Unable to find tcam for table id: {}", tbl_id);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if (tbl_id == P4TBL_ID_INPUT_MAPPING_NATIVE) {
        arr = vrf_pd->gipo_imn_idx;
    } else {
        arr = vrf_pd->gipo_imt_idx;
    }

    for (int i = 0; i < 3; i++) {
        if (arr[i] != INVALID_INDEXER_INDEX) {
            sdk_ret = tcam->remove((uint32_t)arr[i]);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Input mapping native remove failure, "
                              "idx : {}, err : {}", arr[i], ret);
            }
            HAL_TRACE_DEBUG("Removing from tbld_id:{} at {}",
                            tbl_id, arr[i]);
            arr[i] = INVALID_INDEXER_INDEX;
        }
    }
end:
    return ret;
}

// ----------------------------------------------------------------------------
// deprogram gipo prefix
// ----------------------------------------------------------------------------
hal_ret_t
vrf_pd_deprogram_gipo_prefix(pd_vrf_t *vrf_pd)
{
    hal_ret_t ret;

    // deprogram input mapping native
    ret = vrf_pd_del_gipo_termination_prefix(vrf_pd,
                                             P4TBL_ID_INPUT_MAPPING_NATIVE);
    // deprogram input mapping tunneled
    ret = vrf_pd_del_gipo_termination_prefix(vrf_pd,
                                             P4TBL_ID_INPUT_MAPPING_TUNNELED);
    return ret;
}

//-----------------------------------------------------------------------------
// Program input mapping table to terminate GIPo tunnels
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_program_gipo_prefix(pd_vrf_t *vrf_pd,
                           ip_prefix_t *gipo_prefix,
                           bool is_upgrade)
{
    hal_ret_t    ret = HAL_RET_OK;

    if (gipo_prefix->len == 0) {
        return ret; // GIPo not specified. Nothing to program.
    }

    /* We program 3 entries in the INPUT_MAPPING_NATIVE Table for the GIPo Entry */
    /* Entry 1 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                             INGRESS_TUNNEL_TYPE_VXLAN,
                                             true, false,
                                             INPUT_MAPPING_NATIVE_NOP_ID,
                                             P4TBL_ID_INPUT_MAPPING_NATIVE,
                                             &vrf_pd->gipo_imn_idx[0],
                                             is_upgrade);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS))
        goto fail_flag;
    /* Entry 2 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                             INGRESS_TUNNEL_TYPE_VXLAN,
                                             false, true,
                                             INPUT_MAPPING_NATIVE_NOP_ID,
                                             P4TBL_ID_INPUT_MAPPING_NATIVE,
                                             &vrf_pd->gipo_imn_idx[1],
                                             is_upgrade);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS))
        goto fail_flag;
    /* Entry 3 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                             INGRESS_TUNNEL_TYPE_VXLAN,
                                             false, false,
                                             INPUT_MAPPING_NATIVE_NOP_ID,
                                             P4TBL_ID_INPUT_MAPPING_NATIVE,
                                             &vrf_pd->gipo_imn_idx[2],
                                             is_upgrade);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS))
        goto fail_flag;

    /* We program 3 entries in the INPUT_MAPPING_TUNNELED Table for the GIPo Entry */
    /* Entry 1 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                             INGRESS_TUNNEL_TYPE_VXLAN,
                                             true, false,
                                             INPUT_MAPPING_TUNNELED_TUNNELED_IPV4_PACKET_ID,
                                             P4TBL_ID_INPUT_MAPPING_TUNNELED,
                                             &vrf_pd->gipo_imt_idx[0],
                                             is_upgrade);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS))
        goto fail_flag;
    /* Entry 2 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                    INGRESS_TUNNEL_TYPE_VXLAN,
                                    false, true,
                                    INPUT_MAPPING_TUNNELED_TUNNELED_IPV6_PACKET_ID,
                                    P4TBL_ID_INPUT_MAPPING_TUNNELED,
                                    &vrf_pd->gipo_imt_idx[1],
                                    is_upgrade);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS))
        goto fail_flag;
    /* Entry 3 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                    INGRESS_TUNNEL_TYPE_VXLAN,
                                    false, false,
                                    INPUT_MAPPING_TUNNELED_TUNNELED_NON_IP_PACKET_ID,
                                    P4TBL_ID_INPUT_MAPPING_TUNNELED,
                                    &vrf_pd->gipo_imt_idx[2],
                                    is_upgrade);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS))
        goto fail_flag;

    return HAL_RET_OK;

fail_flag:
    vrf_pd_deprogram_gipo_prefix(vrf_pd);
    return ret;
}

// allocate resources.
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_alloc_res(pd_vrf_t *vrf_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    // allocate h/w id for this vrf
    rs = g_hal_state_pd->vrf_hwid_idxr()->
                         alloc((uint32_t *)&vrf_pd->vrf_hw_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("Failed to alloc vrf_hw_id err: {}", rs);
        vrf_pd->vrf_hw_id = INVALID_INDEXER_INDEX;
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }

    vrf_pd->vrf_fl_lkup_id = vrf_pd->vrf_hw_id << HAL_PD_VRF_SHIFT;

    HAL_TRACE_DEBUG("Allocated vrf_hw_id:{}, vrf_fl_lkup_id:{}",
                    vrf_pd->vrf_hw_id, vrf_pd->vrf_fl_lkup_id);

    ret = vrf_pd_alloc_cpuid(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to alloc cpuid. ret:{}", ret);
        goto end;
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// de-allocate resources.
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_dealloc_res(pd_vrf_t *vrf_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    if (vrf_pd->vrf_hw_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->vrf_hwid_idxr()->free(vrf_pd->vrf_hw_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to free vrf_hw_id err: {}",
                          vrf_pd->vrf_hw_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("Freed vrf_hw_id: {}", vrf_pd->vrf_hw_id);
    }

    ret = vrf_pd_dealloc_cpuid(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to dealloc cpuid");
        goto end;
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// allocating cpuid used as vlan id for traffic coming from cpu
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_alloc_cpuid(pd_vrf_t *pd_vrf)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    // Allocate from cpu id
    rs = g_hal_state_pd->l2seg_cpu_idxr()->
        alloc((uint32_t *)&pd_vrf->vrf_fromcpu_vlan_id);
    if (rs != indexer::SUCCESS) {
        pd_vrf->vrf_fromcpu_vlan_id = INVALID_INDEXER_INDEX;
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("Allocated from_cpu_vlan_id: {}",
                    pd_vrf->vrf_fromcpu_vlan_id);

    return ret;
}

//-----------------------------------------------------------------------------
// de-allocate cpuid
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_dealloc_cpuid(pd_vrf_t *vrf_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    if (vrf_pd->vrf_fromcpu_vlan_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->l2seg_cpu_idxr()->free(vrf_pd->
                                                    vrf_fromcpu_vlan_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to free cpuid {}, rs: {}",
                          vrf_pd->vrf_fromcpu_vlan_id, rs);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("Freed from_cpu_vlan_id: {}",
                        vrf_pd->vrf_fromcpu_vlan_id);
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD vrf Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD vrf
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_cleanup(pd_vrf_t *vrf_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!vrf_pd) {
        goto end;
    }

    // check if l2segs have been removed before vrf cleanup
    // index 0 is reserved.
    if (vrf_pd->l2seg_hw_id_idxr_->num_indices_allocated() > 1) {
        HAL_TRACE_ERR("l2seg idxr still in use. usage:{}",
                      vrf_pd->l2seg_hw_id_idxr_->
                      num_indices_allocated());
        ret = HAL_RET_INVALID_OP;
        goto end;
    }

    // releasing resources
    ret = vrf_pd_dealloc_res(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to dealloc res for vrf: {}",
                      ((vrf_t *)(vrf_pd->vrf))->vrf_id);
        goto end;
    }

    // delinking PI<->PD
    delink_pi_pd(vrf_pd, (vrf_t *)vrf_pd->vrf);

    // freeing PD
    vrf_pd_free(vrf_pd);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// allocate l2seg hwid per vrf
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_alloc_l2seg_hw_id(pd_vrf_t *vrf_pd, uint32_t *l2seg_hw_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs = indexer::SUCCESS;

    if (!vrf_pd || !l2seg_hw_id) {
        ret =  HAL_RET_INVALID_ARG;
        goto end;
    }

    rs = vrf_pd->l2seg_hw_id_idxr_->alloc(l2seg_hw_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("Failed to alloc l2seg_hw_id err: {}", rs);
        *l2seg_hw_id = INVALID_INDEXER_INDEX;
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }

    HAL_TRACE_DEBUG("Allocated l2seg_hw_id: {} for vrf: {}",
                    *l2seg_hw_id,
                    ((vrf_t *)(vrf_pd->vrf))->vrf_id);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// free l2seg hwid per vrf
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_free_l2seg_hw_id(pd_vrf_t *vrf_pd, uint32_t l2seg_hw_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs = indexer::SUCCESS;

    if (!vrf_pd) {
        ret =  HAL_RET_INVALID_ARG;
        goto end;
    }

    if (l2seg_hw_id != INVALID_INDEXER_INDEX) {
        rs = vrf_pd->l2seg_hw_id_idxr_->free(l2seg_hw_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to free l2seg_hw_id:{} "
                    "err: {}", l2seg_hw_id, rs);
            ret = HAL_RET_NO_RESOURCE;
            goto end;
        }
        HAL_TRACE_DEBUG("Freed l2seg_hw_id: {} for vrf: {}",
                        l2seg_hw_id,
                        ((vrf_t *)(vrf_pd->vrf))->vrf_id);
    }

end:
    return ret;
}

//----------------------------------------------------------------------------
// linking PI <-> PD
//----------------------------------------------------------------------------
void
link_pi_pd(pd_vrf_t *pd_vrf, vrf_t *pi_vrf)
{
    pd_vrf->vrf = pi_vrf;
    pi_vrf->pd = pd_vrf;
}

// ----------------------------------------------------------------------------
// un-linking PI <-> PD
// ----------------------------------------------------------------------------
void
delink_pi_pd(pd_vrf_t *pd_vrf, vrf_t *pi_vrf)
{
    if (pd_vrf) {
        pd_vrf->vrf = NULL;
    }
    if (pi_vrf) {
        pi_vrf->pd = NULL;
    }
}

// ----------------------------------------------------------------------------
// makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_vrf_make_clone(pd_func_args_t *pd_func_args)
{
    hal_ret_t     ret = HAL_RET_OK;
    pd_vrf_make_clone_args_t *args = pd_func_args->pd_vrf_make_clone;
    pd_vrf_t      *pd_vrf_clone = NULL;
    vrf_t         *vrf, *clone;

    vrf = args->vrf;
    clone = args->clone;

    pd_vrf_clone = vrf_pd_alloc_init();
    if (pd_vrf_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_vrf_clone, vrf->pd, sizeof(pd_vrf_t));

    link_pi_pd(pd_vrf_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_vrf_mem_free(pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_vrf_mem_free_args_t *args = pd_func_args->pd_vrf_mem_free;
    pd_vrf_t    *vrf_pd;

    vrf_pd = (pd_vrf_t *)args->vrf->pd;
    vrf_pd_mem_free(vrf_pd);

    return ret;
}

// ----------------------------------------------------------------------------
// Returns the vlan id for packets from CPU
// Note: Currently being used only for IPSec packets.
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_get_fromcpu_vlanid(pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_vrf_get_fromcpu_vlanid_args_t *args = pd_func_args->pd_vrf_get_fromcpu_vlanid;
    vrf_t *vrf    = args->vrf;
    uint16_t *vid = args->vid;

    if (vrf == NULL || vid == NULL) {
        HAL_TRACE_ERR("invalid args");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    *vid = ((pd_vrf_t *)vrf->pd)->vrf_fromcpu_vlan_id;

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Returns the vrf lookupid of the vrf (used as lkp_vrf in flow key)
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_get_lookup_id(pd_func_args_t *pd_func_args)
{
    pd_vrf_get_lookup_id_args_t *args = pd_func_args->pd_vrf_get_lookup_id;
    vrf_t *vrf = args->vrf;
    args->lkup_id = ((pd_vrf_t *)vrf->pd)->vrf_fl_lkup_id;
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
