// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/vrf_pd.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/src/proxy.hpp"
#include "if_pd_utils.hpp"

namespace hal {
namespace pd {

#define inp_prop data.input_properties_action_u.input_properties_input_properties

//-----------------------------------------------------------------------------
// PD vrf Create
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_create (pd_vrf_create_args_t *args)
{
    hal_ret_t               ret;
    pd_vrf_t                *vrf_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-vrf:{}:creating pd state for vrf {}",
                    __FUNCTION__, args->vrf->vrf_id);

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
        HAL_TRACE_ERR("pd-vrf:{}:failed to allocated resources", 
                      __FUNCTION__);
        goto end;
    }

    // program hw
    ret = vrf_pd_program_hw(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-vrf:{}:failed to program hw", __FUNCTION__);
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

//-----------------------------------------------------------------------------
// PD vrf Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_update (pd_vrf_update_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_vrf_t    *vrf_pd = (pd_vrf_t *)args->vrf->pd;

    if (args->gipo_prefix_change) {

        // De-program input mapping native & tunnel
        ret = vrf_pd_deprogram_gipo_prefix(vrf_pd);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}:unable to deprogram gipo pfx term. ret:{}",
                          __FUNCTION__, ret);
            goto end;
        }

        // Program input mapping native & tunnel with new gipo prefix
        ret = vrf_pd_program_gipo_prefix(vrf_pd, 
                                         args->new_gipo_prefix);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}:unable to program gipo pfx term. ret:{}",
                          __FUNCTION__, ret);
            goto end;
        }
    }

end:
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD vrf Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_vrf_delete (pd_vrf_delete_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_vrf_t    *vrf_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->vrf != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->vrf->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-vrf:{}:Deleting pd state for vrf {}",
                    __FUNCTION__, args->vrf->vrf_id);
    vrf_pd = (pd_vrf_t *)args->vrf->pd;

    // deprogram HW
    ret = vrf_pd_deprogram_hw(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-vrf:{}:unable to deprogram hw", __FUNCTION__);
    }

    // remove from db
    ret = vrf_pd_del_from_db(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-vrf:{}:unable to delete from db", __FUNCTION__);
        goto end;
    }

    // dealloc resources and free
    ret = vrf_pd_cleanup(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-vrf:{}:failed pd vrf delete",
                      __FUNCTION__);
    }

end:
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

    HAL_TRACE_DEBUG("pd-vrf:{}:adding to flow lkup id hash table. fl_lkup_id:{} => ",
                    __FUNCTION__, pd_vrf->vrf_fl_lkup_id);

    // allocate an entry to establish mapping from vrf hwid to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from vrf id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state_pd->flow_lkupid_ht()->
        insert_with_key(&pd_vrf->vrf_fl_lkup_id,
                        entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("pd-vrf:{}:failed to add hw id to handle mapping, "
                      "err : {}", __FUNCTION__, sdk_ret);
        hal::pd::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    return ret;
}

//------------------------------------------------------------------------------
// delete a vrf from hwid database
//------------------------------------------------------------------------------
hal_ret_t
vrf_pd_del_from_db (pd_vrf_t *pd_vrf)
{
    hal_handle_id_ht_entry_t    *entry = NULL;

    HAL_TRACE_DEBUG("pd-vrf:{}:removing from flow lkup id hash table", __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state_pd->flow_lkupid_ht()->
        remove(&pd_vrf->vrf_fl_lkup_id);

    if (entry) {
        // free up
        hal::pd::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
hal_ret_t
vrf_pd_deprogram_hw (pd_vrf_t *vrf_pd)
{
    hal_ret_t            ret = HAL_RET_OK;

    // De-Program Input properties Table
    ret = vrf_pd_depgm_inp_prop_tbl(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-vrf:{}:unable to deprogram hw", __FUNCTION__);
    }

    // De-Program Input Mapping Native & Tunnel
    if (((vrf_t *)(vrf_pd->vrf))->vrf_type == types::VRF_TYPE_INFRA) {
        ret = vrf_pd_deprogram_gipo_prefix(vrf_pd);
    }


    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram input propterties table for cpu tx traffic
// ----------------------------------------------------------------------------
hal_ret_t
vrf_pd_depgm_inp_prop_tbl (pd_vrf_t *vrf_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    sdk_hash                    *inp_prop_tbl = NULL;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);
    
    sdk_ret = inp_prop_tbl->remove(vrf_pd->inp_prop_tbl_cpu_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-vrf::{}:unable to deprogram from cpu entry "
                "input properties for seg_id:{}",
                __FUNCTION__, ((vrf_t*)(vrf_pd->vrf))->vrf_id);
    } else {
        HAL_TRACE_DEBUG("pd-vrf::{}:deprogrammed from_cpu_entry "
                "input properties for seg_id:{}",
                __FUNCTION__, ((vrf_t*)(vrf_pd->vrf))->vrf_id);
    }

    return ret;
}


// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
vrf_pd_program_hw (pd_vrf_t *vrf_pd)
{
    hal_ret_t            ret;

    // Program Input properties Table
    ret = vrf_pd_pgm_inp_prop_tbl(vrf_pd);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    // Program Input Mapping Native & Tunnel Tables: For MyTep Termination
    if (((vrf_t *)(vrf_pd->vrf))->vrf_type == types::VRF_TYPE_INFRA) {
        ret = vrf_pd_program_gipo_prefix(vrf_pd,
                                         &((vrf_t*)(vrf_pd->vrf))->gipo_prefix);
    }


    return ret;
}

// ----------------------------------------------------------------------------
// Program input propterties table for cpu tx traffic
// ----------------------------------------------------------------------------
hal_ret_t
vrf_pd_pgm_inp_prop_tbl (pd_vrf_t *vrf_pd)
{
    hal_ret_t                   ret           = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    sdk_hash                    *inp_prop_tbl = NULL;
    input_properties_swkey_t    key           = { 0 };
    input_properties_actiondata data          = { 0 };

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    key.capri_intrinsic_lif = SERVICE_LIF_CPU;
    key.vlan_tag_valid      = 1;
    key.vlan_tag_vid        = vrf_pd->vrf_fromcpu_vlan_id;
    inp_prop.dir            = FLOW_DIR_FROM_UPLINK;

    inp_prop.vrf              = vrf_pd->vrf_fl_lkup_id;
    inp_prop.l4_profile_idx   = 0;
    inp_prop.ipsg_enable      = 0;
    inp_prop.src_lport        = 0;
    inp_prop.flow_miss_action = 0;
    inp_prop.flow_miss_idx    = 0;
    inp_prop.allow_flood      = 0;

    // Insert
    sdk_ret = inp_prop_tbl->insert(&key, &data, &vrf_pd->inp_prop_tbl_cpu_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-vrf::{}: Unable to program from cpu entry "
                      "input properties for vrf_id:{}",
                      __FUNCTION__, ((vrf_t*)(vrf_pd->vrf))->vrf_id);
        goto end;
    } else {
        HAL_TRACE_DEBUG("pd-vrf::{}: Programmed from_cpu_entry "
                        "input properties for vrf_id:{}",
                        __FUNCTION__, ((vrf_t*)(vrf_pd->vrf))->vrf_id);
    }

end:
    return ret;
}


hal_ret_t
pd_vrf_program_input_mapping_table(ip_prefix_t *ip_prefix,
                                      uint8_t tunnel_type,
                                      bool inner_v4_vld,
                                      bool inner_v6_vld,
                                      uint8_t actionid,
                                      p4pd_table_id tbl_id, uint32_t *idx)
{
    hal_ret_t                           ret = HAL_RET_OK;
    sdk_ret_t                           sdk_ret;
    input_mapping_native_swkey_t        key = {0};
    input_mapping_native_swkey_mask_t   mask = {0};
    input_mapping_native_actiondata     data = {0};
    tcam                                *tcam;
    uint32_t                            ret_idx;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);
    /* Input mapping native and tunneled tables have the same key, mask and data
     * So, we can populate the structs and typecast accordingly */
    memset(&key, 0, sizeof(input_mapping_native_swkey_t));
    memset(&mask, 0, sizeof(input_mapping_native_swkey_mask_t));
    key.inner_ipv4_valid = (uint8_t)inner_v4_vld;
    key.inner_ipv6_valid = (uint8_t)inner_v6_vld;
    key.tunnel_metadata_tunnel_type = tunnel_type;
    mask.inner_ipv4_valid_mask = 0xFF;
    mask.inner_ipv6_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    if (ip_prefix->addr.af == IP_AF_IPV4) {
        HAL_TRACE_DEBUG("Programming Addr: {}", ippfx2str(ip_prefix));
        key.ipv4_valid = 1;
        key.input_mapping_native_u1.ipv4_dstAddr = ip_prefix->addr.addr.v4_addr;
        mask.ipv4_valid_mask = 0xFF;
        mask.input_mapping_native_mask_u1.ipv4_dstAddr_mask =
                ipv4_prefix_len_to_mask(ip_prefix->len);
    } else {
        key.ipv6_valid = 1;
        memcpy(key.input_mapping_native_u1.ipv6_dstAddr,
               ip_prefix->addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
        memrev(key.input_mapping_native_u1.ipv6_dstAddr, IP6_ADDR8_LEN);
        mask.ipv6_valid_mask = 0xFF;
        ipv6_prefix_len_to_mask(
                (ipv6_addr_t*)(mask.input_mapping_native_mask_u1.ipv6_dstAddr_mask),
                ip_prefix->len);
        memrev(mask.input_mapping_native_mask_u1.ipv6_dstAddr_mask, IP6_ADDR8_LEN);
    }
    data.actionid = actionid;
    sdk_ret = tcam->insert(&key, &mask, &data, &ret_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_DUP_INS_FAIL) {
        /* Entry already exists. Can be skipped */
        *idx = INVALID_INDEXER_INDEX;
    } else {
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Input mapping table tcam write failure, idx : {}, err : {}",
                          ret_idx, ret);
            return ret;
        }
    }
    HAL_TRACE_DEBUG("Input mapping table tcam write, idx : {}, ret: {}",
                    ret_idx, ret);
    *idx = (int) ret_idx;
    return ret;
}

// ----------------------------------------------------------------------------
// Program input mapping table to terminate GIPo tunnels
// ----------------------------------------------------------------------------
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
        HAL_TRACE_ERR("pd-vrf:{}:unable to find tcam for table id: {}",
                      __FUNCTION__, tbl_id);
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
                HAL_TRACE_ERR("Input mapping native tcam remove failure, "
                              "idx : {}, err : {}", arr[i], ret);
            }
            HAL_TRACE_DEBUG("pd-vrf:{}:Removing from tbld_id:{} at {}",
                            __FUNCTION__, tbl_id, arr[i]);
            arr[i] = INVALID_INDEXER_INDEX;
        }
    }
end:
    return ret;
}

// ----------------------------------------------------------------------------
// De-Program HW
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

// ----------------------------------------------------------------------------
// Program input mapping table to terminate GIPo tunnels
// ----------------------------------------------------------------------------
hal_ret_t
vrf_pd_program_gipo_prefix(pd_vrf_t *vrf_pd, 
                           ip_prefix_t *gipo_prefix)
{
    uint32_t     idx;
    hal_ret_t    ret = HAL_RET_OK;
    // ip_prefix_t  *gipo_prefix = &((vrf_t*)(vrf_pd->vrf))->gipo_prefix;

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
                                                &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    vrf_pd->gipo_imn_idx[0] = idx;
    /* Entry 2 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                                INGRESS_TUNNEL_TYPE_VXLAN,
                                                false, true,
                                                INPUT_MAPPING_NATIVE_NOP_ID,
                                                P4TBL_ID_INPUT_MAPPING_NATIVE,
                                                &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    vrf_pd->gipo_imn_idx[1] = idx;
    /* Entry 3 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                                INGRESS_TUNNEL_TYPE_VXLAN,
                                                false, false,
                                                INPUT_MAPPING_NATIVE_NOP_ID,
                                                P4TBL_ID_INPUT_MAPPING_NATIVE,
                                                &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    vrf_pd->gipo_imn_idx[2] = idx;

    /* We program 3 entries in the INPUT_MAPPING_TUNNELED Table for the GIPo Entry */
    /* Entry 1 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                    INGRESS_TUNNEL_TYPE_VXLAN,
                                    true, false,
                                    INPUT_MAPPING_TUNNELED_TUNNELED_IPV4_PACKET_ID,
                                    P4TBL_ID_INPUT_MAPPING_TUNNELED,
                                    &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    vrf_pd->gipo_imt_idx[0] = idx;
    /* Entry 2 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                    INGRESS_TUNNEL_TYPE_VXLAN,
                                    false, true,
                                    INPUT_MAPPING_TUNNELED_TUNNELED_IPV6_PACKET_ID,
                                    P4TBL_ID_INPUT_MAPPING_TUNNELED,
                                    &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    vrf_pd->gipo_imt_idx[1] = idx;
    /* Entry 3 */
    ret = pd_vrf_program_input_mapping_table(gipo_prefix,
                                    INGRESS_TUNNEL_TYPE_VXLAN,
                                    false, false,
                                    INPUT_MAPPING_TUNNELED_TUNNELED_NON_IP_PACKET_ID,
                                    P4TBL_ID_INPUT_MAPPING_TUNNELED,
                                    &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    vrf_pd->gipo_imt_idx[2] = idx;

    return HAL_RET_OK;

fail_flag:
    vrf_pd_deprogram_gipo_prefix(vrf_pd);
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate resources. 
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
        HAL_TRACE_ERR("pd-vrf:{}:failed to alloc vrf_hw_id err: {}", 
                      __FUNCTION__, rs);
        vrf_pd->vrf_hw_id = INVALID_INDEXER_INDEX;
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }

    vrf_pd->vrf_fl_lkup_id = vrf_pd->vrf_hw_id << HAL_PD_VRF_SHIFT;

    HAL_TRACE_DEBUG("pd-vrf:{}:allocated vrf_hw_id:{}, vrf_fl_lkup_id:{}", 
                    __FUNCTION__, vrf_pd->vrf_hw_id, vrf_pd->vrf_fl_lkup_id);

    ret = vrf_pd_alloc_cpuid(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-vrf:{}:failed to alloc cpuid", __FUNCTION__);
        goto end;
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources. 
//-----------------------------------------------------------------------------
hal_ret_t
vrf_pd_dealloc_res(pd_vrf_t *vrf_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    if (vrf_pd->vrf_hw_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->vrf_hwid_idxr()->free(vrf_pd->vrf_hw_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("pd-vrf:{}:failed to free vrf_hw_id err: {}", 
                          __FUNCTION__, vrf_pd->vrf_hw_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("pd-vrf:{}:freed vrf_hw_id: {}", 
                        __FUNCTION__, vrf_pd->vrf_hw_id);
    }

    ret = vrf_pd_dealloc_cpuid(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-vrf:{}:failed to dealloc cpuid", __FUNCTION__);
        goto end;
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Allocating cpuid used as vlan id for traffic coming from cpu
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
    HAL_TRACE_DEBUG("pd-vrf:{}: vrf_id:{} allocated from_cpu_vlan_id: {}", 
                    __FUNCTION__, 
                    ((vrf_t*)(pd_vrf->vrf))->vrf_id,
                    pd_vrf->vrf_fromcpu_vlan_id);

    return ret;
}


//-----------------------------------------------------------------------------
// De-Allocate cpuid
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
            HAL_TRACE_ERR("pd-vrf:{}:failed to free cpuid err: {}", 
                          __FUNCTION__, vrf_pd->vrf_fromcpu_vlan_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("pd-vrf:{}:freed from_cpu_vlan_id: {}", 
                        __FUNCTION__, vrf_pd->vrf_fromcpu_vlan_id);
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
        // Nothing to do
        goto end;
    }

    // Check if l2segs have been removed before vrf cleanup
    // index 0 is reserved.
    if (vrf_pd->l2seg_hw_id_idxr_->num_indices_allocated() > 1) {
        HAL_TRACE_ERR("pd-vrf:{}:l2seg idxr still in use. usage:{}", 
                      __FUNCTION__, vrf_pd->l2seg_hw_id_idxr_->
                      num_indices_allocated());
        ret = HAL_RET_INVALID_OP;
        goto end;
    }

    // Releasing resources
    ret = vrf_pd_dealloc_res(vrf_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-vrf:{}: unable to dealloc res for vrf: {}", 
                      __FUNCTION__, 
                      ((vrf_t *)(vrf_pd->vrf))->vrf_id);
        goto end;
    }

    // Delinking PI<->PD
    delink_pi_pd(vrf_pd, (vrf_t *)vrf_pd->vrf);

    // Freeing PD
    vrf_pd_free(vrf_pd);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate l2seg hwid per vrf
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
        HAL_TRACE_ERR("pd-vrf:{}:failed to alloc l2seg_hw_id err: {}", 
                      __FUNCTION__, rs);
        *l2seg_hw_id = INVALID_INDEXER_INDEX;
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }

    HAL_TRACE_DEBUG("pd-vrf:{}:allocated l2seg_hw_id: {} for vrf: {}", 
                    __FUNCTION__, *l2seg_hw_id, 
                    ((vrf_t *)(vrf_pd->vrf))->vrf_id);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Free l2seg hwid per vrf
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
            HAL_TRACE_ERR("pd-vrf:{}:Failed to free l2seg_hw_id:{} "
                    "err: {}", __FUNCTION__,
                    l2seg_hw_id, rs);
            ret = HAL_RET_NO_RESOURCE;
            goto end;
        }
        HAL_TRACE_DEBUG("pd-vrf:{}:freed l2seg_hw_id: {} for vrf: {}", 
                        __FUNCTION__, l2seg_hw_id, 
                        ((vrf_t *)(vrf_pd->vrf))->vrf_id);
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_vrf_t *pd_ten, vrf_t *pi_ten)
{
    pd_ten->vrf = pi_ten;
    pi_ten->pd = pd_ten;
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
delink_pi_pd(pd_vrf_t *pd_ten, vrf_t *pi_ten)
{
    if (pd_ten) {
        pd_ten->vrf = NULL;
    }
    if (pi_ten) {
        pi_ten->pd = NULL;
    }
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_vrf_make_clone(pd_vrf_make_clone_args_t *args)
{
    hal_ret_t     ret           = HAL_RET_OK;
    pd_vrf_t      *pd_ten_clone = NULL;
    vrf_t         *ten, *clone;

    ten = args->vrf;
    clone = args->clone;

    pd_ten_clone = vrf_pd_alloc_init();
    if (pd_ten_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_ten_clone, ten->pd, sizeof(pd_vrf_t));

    link_pi_pd(pd_ten_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_vrf_mem_free(pd_vrf_mem_free_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_vrf_t    *vrf_pd;

    vrf_pd = (pd_vrf_t *)args->vrf->pd;
    vrf_pd_mem_free(vrf_pd);

    return ret;
}

// ----------------------------------------------------------------------------
// Returns the vlan id for packets from CPU 
// Note: Currently being used only for IPSec packets.
// ----------------------------------------------------------------------------
hal_ret_t
// pd_vrf_get_fromcpu_vlanid(vrf_t *vrf, uint16_t *vid)
pd_vrf_get_fromcpu_vlanid(pd_vrf_get_fromcpu_vlanid_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;
    vrf_t *vrf = args->vrf;
    uint16_t *vid = args->vid;

    if (vrf == NULL || vid == NULL) {
        HAL_TRACE_ERR("{}:invalid args", __FUNCTION__);
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
#if 0
uint32_t
pd_vrf_get_lookup_id(vrf_t *vrf)
{
    return ((pd_vrf_t *)vrf->pd)->vrf_fl_lkup_id;
}
#endif

hal_ret_t
pd_vrf_get_lookup_id(pd_vrf_get_lookup_id_args_t *args)
{
    vrf_t *vrf = args->vrf;
    args->lkup_id = ((pd_vrf_t *)vrf->pd)->vrf_fl_lkup_id;
    return HAL_RET_OK;
}


}    // namespace pd
}    // namespace hal
