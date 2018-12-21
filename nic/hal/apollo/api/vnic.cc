/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.cc
 *
 * @brief   This file deals with vnic api handling
 */

#include <stdio.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/vnic.hpp"
#include "nic/hal/apollo/api/utils.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"

// TODO: push p4 programming to impl class
// TODO: HACK !!!, remove
void table_health_monitor_cb(uint32_t table_id,
                             char *name,
                             table_health_state_t curr_state,
                             uint32_t capacity,
                             uint32_t usage,
                             table_health_state_t *new_state) {
}

namespace api {

/**
 * @defgroup OCI_VNIC_ENTRY - vnic entry functionality
 * @ingroup OCI_VNIC
 * @{
 */

/**< @brief    constructor */
vnic_entry::vnic_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
    hw_id_ = 0xFFFF;
    vnic_by_slot_hash_idx_ = 0xFFFF;
}

/**
 * @brief    factory method to allocate and initialize a vnic entry
 * @param[in] oci_vnic    vnic information
 * @return    new instance of vnic or NULL, in case of error
 */
vnic_entry *
vnic_entry::factory(oci_vnic_t *oci_vnic) {
    vnic_entry *vnic;

    /**< create vnic entry with defaults, if any */
    vnic = vnic_db()->vnic_alloc();
    if (vnic) {
        new (vnic) vnic_entry();
    }
    return vnic;

}

/**< @brief    destructor */
vnic_entry::~vnic_entry() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
}

/**
 * @brief    release all the s/w & h/w resources associated with this object,
 *           if any, and free the memory
 * @param[in] vnic     vnic to be freed
 * NOTE: h/w entries themselves should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
vnic_entry::destroy(vnic_entry *vnic) {
    vnic->free_resources_();
    vnic->~vnic_entry();
}

#if 0
/**
 * @brief     handle a vnic create by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::process_create(api_ctxt_t *api_ctxt) {
    return init(&api_ctxt->vnic_info);
}

/**
 * @brief     handle a vnic update by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::process_update(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief     handle a vnic delete by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::process_delete(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief     handle a vnic get by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::process_get(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}
#endif

/**
 * @brief     initialize vnic entry with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::init_config(api_ctxt_t *api_ctxt) {
    oci_vnic_t *oci_vnic = &api_ctxt->api_params->vnic_info;

    memcpy(&this->key_, &oci_vnic->key, sizeof(oci_vnic_key_t));
    return sdk::SDK_RET_OK;
}

#if 0
/**
 * @brief     update/override the subnet object with given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::update_config(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}
#endif

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
// TODO: 1. this should ideally go to impl class
//       2. we don't need an indexer here if we can use directmap here to
//          "reserve" an index
sdk_ret_t
vnic_entry::alloc_resources_(void) {
    /**
     * allocate hw id for this vnic, vnic specific index tables in the p4
     * datapath are indexed by this
     */
    if (vnic_db()->vnic_idxr()->alloc((uint32_t *)&this->hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
// TODO: this should ideally go to impl class (other than subnet_db lookup
#define egress_local_vnic_info_rx_action    action_u.egress_local_vnic_info_rx_egress_local_vnic_info_rx
sdk_ret_t
vnic_entry::program_config(obj_ctxt_t *obj_ctxt) {
    sdk_ret_t                                 ret;
    p4pd_error_t                              p4pd_ret;
    subnet_entry                              *subnet;
    egress_local_vnic_info_rx_actiondata_t    egress_vnic_data = { 0 };
    vnic_rx_stats_actiondata_t                vnic_rx_stats_data = { 0 };
    vnic_tx_stats_actiondata_t                vnic_tx_stats_data = { 0 };
    oci_vnic_t                                *vnic_info;

    //impl->program_hw();
    alloc_resources_();

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
    ret = vnic_db()->egress_local_vnic_info_rx_tbl()->insert_withid(&egress_vnic_data,
                                                                    hw_id_);
    if (ret != sdk::SDK_RET_OK) {
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

    return sdk::SDK_RET_OK;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
// TODO: we need to free tbl mgmt. lib entries as well !!
sdk_ret_t
vnic_entry::free_resources_(void) {
    egress_local_vnic_info_rx_actiondata_t    egress_vnic_data = { 0 };

    if (hw_id_ != 0xFF) {
        vnic_db()->vnic_idxr()->free(hw_id_);
        // TODO: how do we know whether we actually programmed h/w or not ?
        vnic_db()->egress_local_vnic_info_rx_tbl()->retrieve(hw_id_,
                                                             &egress_vnic_data);
        vnic_db()->egress_local_vnic_info_rx_tbl()->remove(hw_id_);
        vnic_db()->local_vnic_by_vlan_tx_tbl()->remove(egress_vnic_data.egress_local_vnic_info_rx_action.overlay_vlan_id);
        if (vnic_by_slot_hash_idx_ != 0xFFFF) {
            vnic_db()->local_vnic_by_slot_rx_tbl()->remove(vnic_by_slot_hash_idx_);
        }
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    // impl->cleanup_hw();
    return sdk::SDK_RET_OK;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // impl->update_hw();
    return sdk::SDK_RET_OK;
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
vnic_entry::activate_config(oci_epoch_t epoch, api_op_t api_op,
                           obj_ctxt_t *obj_ctxt) {
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
    case API_OP_CREATE:
        /**< initialize local_vnic_by_vlan_tx table entry */
        vnic_by_vlan_data.action_id =
            LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.local_vnic_tag = hw_id_;
        vnic_by_vlan_data.local_vnic_by_vlan_tx_info.vcn_id = vcn->hwid();
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
        ret = vnic_db()->local_vnic_by_vlan_tx_tbl()->insert_withid(&vnic_by_vlan_data,
                                                                    vnic_info->wire_vlan);
        SDK_ASSERT_GOTO((ret == sdk::SDK_RET_OK), error);

        /**< initialize local_vnic_by_slot_rx table entry */
        vnic_by_slot_key.mpls_dst_label = vnic_info->slot;
        vnic_by_slot_data.action_id =
            LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID;
        vnic_by_slot_data.local_vnic_by_slot_rx_info.local_vnic_tag = hw_id_;
        vnic_by_slot_data.local_vnic_by_slot_rx_info.vcn_id = vcn->hwid();
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
        ret = vnic_db()->local_vnic_by_slot_rx_tbl()->insert(&vnic_by_slot_key,
                                                             &vnic_by_slot_data,
                                                             (uint32_t *)&vnic_by_slot_hash_idx_);
        SDK_ASSERT_GOTO((ret == sdk::SDK_RET_OK), error);
        break;

    case API_OP_UPDATE:
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
    return sdk::SDK_RET_OK;

error:

    // we can undo (makes rollback easy) things right here !!
    SDK_ASSERT(FALSE);
    return ret;
}

/**
 * @brief    this method is called on new object that needs to replace the
 *           old version of the object in the DBs
 * @param[in] orig_obj    old version of the object being swapped out
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

#if 0
/**
 * @brief    commit() is invokved during commit phase of the API processing and
 *           is not expected to fail as all required resources are already
 *           allocated by now. Based on the API operation, this API is expected
 *           to process either create/retrieve/update/delete. If any temporary
 *           state was stashed in the api_ctxt while processing this API, it
 *           should be freed here
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 *
 * NOTE:     commit() is not expected to fail
 */
sdk_ret_t
vnic_entry::commit(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief     abort() is invoked during abort phase of the API processing and is
 *            not expected to fail. During this phase, all associated resources
 *            must be freed and global DBs need to be restored back to their
 *            original state and any transient state stashed in api_ctxt while
 *            processing this API should also be freed here
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::abort(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}
#endif

/**
 * @brief add vnic to database
 *
 * @param[in] vnic vnic
 */
sdk_ret_t
vnic_entry::add_to_db(void) {
    return vnic_db()->vnic_ht()->insert_with_key(&key_, this,
                                                 &ht_ctxt_);
}

/**
 * @brief delete vnic from database
 *
 * @param[in] vnic_key vnic key
 */
sdk_ret_t
vnic_entry::del_from_db(void) {
    vnic_db()->vnic_ht()->remove(&key_);
    return sdk::SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
vnic_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_VNIC, this);
}

/** @} */    // end of OCI_VNIC_ENTRY

/**
 * @defgroup OCI_VNIC_STATE - vnic database functionality
 * @ingroup OCI_VNIC
 * @{
 */

/**
 * @brief    constructor
 */
vnic_state::vnic_state() {
    p4pd_table_properties_t    tinfo, oflow_tinfo;

    // TODO: need to tune multi-threading related params later
    vnic_ht_ = ht::factory(OCI_MAX_VNIC >> 2,
                           vnic_entry::vnic_key_func_get,
                           vnic_entry::vnic_hash_func_compute,
                           vnic_entry::vnic_key_func_compare);
    SDK_ASSERT(vnic_ht_ != NULL);

    vnic_idxr_ = indexer::factory(OCI_MAX_VNIC);
    SDK_ASSERT(vnic_idxr_ != NULL);

    vnic_slab_ = slab::factory("vnic", OCI_SLAB_VNIC, sizeof(vnic_entry),
                               16, true, true, true, NULL);
    SDK_ASSERT(vnic_slab_ != NULL);

    /**< instantiate P4 tables for bookkeeping */
    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX, &tinfo);
    local_vnic_by_vlan_tx_tbl_ =
        directmap::factory(tinfo.tablename, P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, table_health_monitor_cb);
    SDK_ASSERT(local_vnic_by_vlan_tx_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX, &tinfo);
    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX_OTCAM,
                              &oflow_tinfo);
    local_vnic_by_slot_rx_tbl_ =
        sdk_hash::factory(tinfo.tablename, P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX,
                          P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX_OTCAM,
                          tinfo.tabledepth,
                          oflow_tinfo.tabledepth,
                          tinfo.key_struct_size,
                          tinfo.actiondata_struct_size,
                          static_cast<sdk_hash::HashPoly>(tinfo.hash_type),
                          true, table_health_monitor_cb);
    SDK_ASSERT(local_vnic_by_slot_rx_tbl_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_EGRESS_LOCAL_VNIC_INFO_RX, &tinfo);
    egress_local_vnic_info_rx_tbl_ =
        directmap::factory(tinfo.tablename, P4TBL_ID_EGRESS_LOCAL_VNIC_INFO_RX,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, table_health_monitor_cb);
    SDK_ASSERT(egress_local_vnic_info_rx_tbl_ != NULL);
}

/**
 * @brief    destructor
 */
vnic_state::~vnic_state() {
    ht::destroy(vnic_ht_);
    indexer::destroy(vnic_idxr_);
    slab::destroy(vnic_slab_);
    directmap::destroy(local_vnic_by_vlan_tx_tbl_);
    sdk_hash::destroy(local_vnic_by_slot_rx_tbl_);
    directmap::destroy(egress_local_vnic_info_rx_tbl_);
}

/**
 * @brief     allocate vnic instance
 * @return    pointer to the allocated vnic , NULL if no memory
 */
vnic_entry *
vnic_state::vnic_alloc(void) {
    return ((vnic_entry *)vnic_slab_->alloc());
}

/**
 * @brief        lookup vnic in database with given key
 * @param[in]    vnic_key vnic key
 * @return       pointer to the vnic instance found or NULL
 */
vnic_entry *
vnic_state::vnic_find(oci_vnic_key_t *vnic_key) const {
    return (vnic_entry *)(vnic_ht_->lookup(vnic_key));
}

/**
 * @brief free vnic instance
 * @param[in] vnic vnic instance
 */
void
vnic_state::vnic_free(vnic_entry *vnic) {
    api::delay_delete_to_slab(OCI_SLAB_VNIC, vnic);
}

/** @} */    // end of OCI_VNIC_STATE

}    // namespace api

/**
 * @brief Create VNIC
 *
 * @param[in] vnic VNIC information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vnic_create (_In_ oci_vnic_t *vnic)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_VNIC;
        api_ctxt.api_params->vnic_info = *vnic;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/**
 * @brief Delete VNIC
 *
 * @param[in] vnic_key VNIC key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vnic_delete (_In_ oci_vnic_key_t *vnic_key)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    api_ctxt.api_params = (api_params_t *)api::api_params_slab()->alloc();
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_VNIC;
        api_ctxt.api_params->vnic_key = *vnic_key;
        rv = api::g_api_engine.process_api(&api_ctxt);
        return rv;
    }
    return sdk::SDK_RET_OOM;
}

/** @} */ // end of OCI_VNIC_API
