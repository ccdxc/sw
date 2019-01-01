/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic_impl.cc
 *
 * @brief   datapath implementation of vnic
 */

#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"
#include "nic/hal/apollo/api/vcn.hpp"
#include "nic/hal/apollo/api/subnet.hpp"
#include "nic/hal/apollo/api/vnic.hpp"
#include "nic/hal/apollo/api/impl/vnic_impl.hpp"
#include "nic/hal/apollo/api/impl/oci_impl_state.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace impl {

/**
 * @defgroup OCI_VNIC_IMPL - vnic entry datapath implementation
 * @ingroup OCI_VNIC
 * @{
 */

/**
 * @brief    factory method to allocate & initialize vnic impl instance
 * @param[in] oci_vnic    vnic information
 * @return    new instance of vnic or NULL, in case of error
 */
vnic_impl *
vnic_impl::factory(oci_vnic_t *oci_vnic) {
    vnic_impl *impl;

    // TODO: move to slab later
    impl = (vnic_impl *)SDK_CALLOC(SDK_MEM_ALLOC_OCI_VNIC_IMPL,
                                  sizeof(vnic_impl));
    new (impl) vnic_impl();
    return impl;
}

/**
 * @brief    release all the s/w state associated with the given vnic,
 *           if any, and free the memory
 * @param[in] vnic     vnic to be freed
 * NOTE: h/w entries should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
vnic_impl::destroy(vnic_impl *impl) {
    impl->~vnic_impl();
    SDK_FREE(SDK_MEM_ALLOC_OCI_VNIC_IMPL, impl);
}

/**
 * @brief    allocate/reserve h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl::alloc_resources(api_base *api_obj) {
    /**
     * allocate hw id for this vnic, vnic specific index tables in the p4
     * datapath are indexed by this
     */
    if (vnic_impl_db()->vnic_idxr()->alloc((uint32_t *)&hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
#define egress_local_vnic_info_rx_action    action_u.egress_local_vnic_info_rx_egress_local_vnic_info_rx
sdk_ret_t
vnic_impl::free_resources(api_base *api_obj) {
    egress_local_vnic_info_rx_actiondata_t    egress_vnic_data = { 0 };

    if (hw_id_ != 0xFF) {
        vnic_impl_db()->vnic_idxr()->free(hw_id_);
        // TODO: how do we know whether we actually programmed h/w or not ?
        vnic_impl_db()->egress_local_vnic_info_rx_tbl()->retrieve(hw_id_,
                                                                  &egress_vnic_data);
        vnic_impl_db()->egress_local_vnic_info_rx_tbl()->remove(hw_id_);
        vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->remove(
            egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id);
        if (vnic_by_slot_hash_idx_ != 0xFFFF) {
            vnic_impl_db()->local_vnic_by_slot_rx_tbl()->remove(vnic_by_slot_hash_idx_);
        }
    }
    return SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
// TODO: undo stuff if something goes wrong here !!
sdk_ret_t
vnic_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t                                 ret;
    oci_vnic_t                                *vnic_info;
    p4pd_error_t                              p4pd_ret;
    subnet_entry                              *subnet;
    egress_local_vnic_info_rx_actiondata_t    egress_vnic_data = { 0 };
    vnic_rx_stats_actiondata_t                vnic_rx_stats_data = { 0 };
    vnic_tx_stats_actiondata_t                vnic_tx_stats_data = { 0 };

    vnic_info = &obj_ctxt->api_params->vnic_info;
    subnet = subnet_db()->subnet_find(&vnic_info->subnet_id);
    if (subnet == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    /**< initialize tx stats tables for this vnic */
    vnic_tx_stats_data.action_id = VNIC_TX_STATS_VNIC_TX_STATS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_TX_STATS,
                                       hw_id_, NULL, NULL,
                                       &vnic_tx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    /**< initialize egress_local_vnic_info_rx table entry */
    egress_vnic_data.action_id =
        EGRESS_LOCAL_VNIC_INFO_RX_EGRESS_LOCAL_VNIC_INFO_RX_ID;
    memcpy(egress_vnic_data.egress_local_vnic_info_rx_action.vr_mac,
           subnet->vr_mac(), ETH_ADDR_LEN);
    memcpy(egress_vnic_data.egress_local_vnic_info_rx_action.overlay_mac,
           vnic_info->mac_addr, ETH_ADDR_LEN);
    egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id =
        vnic_info->wire_vlan;
    egress_vnic_data.egress_local_vnic_info_rx_action.subnet_id =
        subnet->hw_id();
    ret = vnic_impl_db()->egress_local_vnic_info_rx_tbl()->insert_withid(&egress_vnic_data,
                                                                         hw_id_);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /**< initialize tx stats tables for this vnic */
    vnic_rx_stats_data.action_id = VNIC_RX_STATS_VNIC_RX_STATS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_RX_STATS,
                                       hw_id_, NULL, NULL,
                                       &vnic_rx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    return SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] curr_obj    cloned and updated version of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                     obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    activate the epoch in the dataplane by programming stage 0
 *           tables, if any
 * @param[in] epoch       epoch being activated
 * @param[in] api_op      api operation
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
#define local_vnic_by_vlan_tx_info    action_u.local_vnic_by_vlan_tx_local_vnic_info_tx
#define local_vnic_by_slot_rx_info    action_u.local_vnic_by_slot_rx_local_vnic_info_rx
sdk_ret_t
vnic_impl::activate_hw(api_base *api_obj, oci_epoch_t epoch,
                       api_op_t api_op, obj_ctxt_t *obj_ctxt)
{
    sdk_ret_t                                   ret;
    vcn_entry                                   *vcn;
    subnet_entry                                *subnet;
    local_vnic_by_vlan_tx_actiondata_t          vnic_by_vlan_data = { 0 };
    local_vnic_by_slot_rx_swkey_t               vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_actiondata_t          vnic_by_slot_data = { 0 };
    oci_vnic_t                                  *vnic_info;

    vnic_info = &obj_ctxt->api_params->vnic_info;
    vcn = vcn_db()->vcn_find(&vnic_info->vcn_id);
    if (vcn == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    subnet = subnet_db()->subnet_find(&vnic_info->subnet_id);
    if (subnet == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    switch (api_op) {
    case api::API_OP_CREATE:
        /**< initialize local_vnic_by_vlan_tx table entry */
        vnic_by_vlan_data.action_id =
            LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.local_vnic_tag = hw_id_;
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.vcn_id = vcn->hw_id();
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.skip_src_dst_check =
            vnic_info->src_dst_check ? false : true;
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.resource_group_1 =
            vnic_info->rsc_pool_id;
        // TODO: do we need to enhance the vnic API here to take this ?
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.resource_group_2 = 0;
        //vnic_by_vlan_data.local_vnic_by_vlan_tx_info.lpm_addr_1 =
            //subnet->lpm_root();
        //vnic_by_vlan_data.local_vnic_by_vlan_tx_info.slacl_addr_1 =
            //subnet->policy_tree_root();
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch1 = epoch;
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch2 = OCI_EPOCH_INVALID;
        memcpy(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.overlay_mac,
               vnic_info->mac_addr, ETH_ADDR_LEN);
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.src_slot_id =
            vnic_info->slot;
        ret = vnic_impl_db()->local_vnic_by_vlan_tx_tbl()->insert_withid(&vnic_by_vlan_data,
                                                                         vnic_info->wire_vlan);
        SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);

        /**< initialize local_vnic_by_slot_rx table entry */
        vnic_by_slot_key.mpls_dst_label = vnic_info->slot;
        vnic_by_slot_data.action_id =
            LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID;
        vnic_by_slot_data.local_vnic_by_slot_rx_info.local_vnic_tag = hw_id_;
        vnic_by_slot_data.local_vnic_by_slot_rx_info.vcn_id = vcn->hw_id();
        vnic_by_slot_data.local_vnic_by_slot_rx_info.skip_src_dst_check =
            vnic_info->src_dst_check ? false : true;
        vnic_by_slot_data.local_vnic_by_slot_rx_info.resource_group_1 =
            vnic_info->rsc_pool_id;
        // TODO: do we need to enhance the vnic API here to take this ?
        vnic_by_slot_data.local_vnic_by_slot_rx_info.resource_group_2 = 0;
        //vnic_by_slot_data.local_vnic_by_slot_rx_info.slacl_addr_1 =
            //subnet->policy_tree_root();
        vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch1 = epoch;
        vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch2 = OCI_EPOCH_INVALID;
        ret = vnic_impl_db()->local_vnic_by_slot_rx_tbl()->insert(&vnic_by_slot_key,
                                                                  &vnic_by_slot_data,
                                                                  (uint32_t *)&vnic_by_slot_hash_idx_);
        SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);
        break;

    case api::API_OP_UPDATE:
#if 0
        /**< read the current entry in h/w */
        p4pd_entry_read(P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX,
                        vnic_info->vlan_id, NULL, NULL,
                        &vnic_by_vlan_data);
        epoch_idx =
            compute_epoch_idx(vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch1,
                              vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch2);
#endif

    default:
        break;
    }
    return SDK_RET_OK;

error:

    // we can undo (makes rollback easy) things right here !!
    SDK_ASSERT(FALSE);
    return ret;
}

/** @} */    // end of OCI_VNIC_IMPL

}    // namespace impl
