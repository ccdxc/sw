/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.cc
 *
 * @brief   This file deals with vnic api handling
 */

#include <stdio.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/vnic.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"

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
 *       cleanup_hw() before calling this
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

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
// TODO: this should ideally go to impl class
sdk_ret_t
vnic_entry::alloc_resources(void) {
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
 * @brief     update/override the subnet object with given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::update_config(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::program_hw(obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::free_resources_(void) {
    if (hw_id_ != 0xFF) {
        vnic_db()->vnic_idxr()->free(hw_id_);
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
vnic_entry::cleanup_hw(obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    update all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::update_hw(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
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

/**
 * @brief    activate the epoch in the dataplane
 * @param[in] api_op      api operation
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_entry::activate_epoch(api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
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
    local_vnic_by_vlan_tx_ =
        directmap::factory(tinfo.tablename, P4TBL_ID_LOCAL_VNIC_BY_VLAN_TX,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, table_health_monitor_cb);
    SDK_ASSERT(local_vnic_by_vlan_tx_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX, &tinfo);
    p4pd_table_properties_get(P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX_OTCAM,
                              &oflow_tinfo);
    local_vnic_by_slot_rx_ =
        sdk_hash::factory(tinfo.tablename, P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX,
                          P4TBL_ID_LOCAL_VNIC_BY_SLOT_RX_OTCAM,
                          tinfo.tabledepth,
                          oflow_tinfo.tabledepth,
                          tinfo.key_struct_size,
                          tinfo.actiondata_struct_size,
                          static_cast<sdk_hash::HashPoly>(tinfo.hash_type),
                          true, table_health_monitor_cb);
    SDK_ASSERT(local_vnic_by_slot_rx_ != NULL);

    p4pd_table_properties_get(P4TBL_ID_EGRESS_LOCAL_VNIC_INFO_RX, &tinfo);
    egress_local_vnic_info_rx_ =
        directmap::factory(tinfo.tablename, P4TBL_ID_EGRESS_LOCAL_VNIC_INFO_RX,
                           tinfo.tabledepth, tinfo.actiondata_struct_size,
                           false, true, table_health_monitor_cb);
    SDK_ASSERT(egress_local_vnic_info_rx_ != NULL);
}

/**
 * @brief    destructor
 */
vnic_state::~vnic_state() {
    ht::destroy(vnic_ht_);
    indexer::destroy(vnic_idxr_);
    slab::destroy(vnic_slab_);
    directmap::destroy(local_vnic_by_vlan_tx_);
    sdk_hash::destroy(local_vnic_by_slot_rx_);
    directmap::destroy(egress_local_vnic_info_rx_);
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

#if 0
/**
 * @defgroup OCI_VNIC_DB - VNIC database functionality
 * @ingroup OCI_VNIC
 * @{
 */

/**
 * @brief Uninitialize and free internal VNIC structure
 *
 * @param[in] vnic VNIC
 */
void
vnic_state::vnic_delete(_In_ vnic_t *vnic) {
    if (vnic) {
        vnic_cleanup(vnic);
        vnic_free(vnic);
    }
}

/**
 * @brief program the P4/P4+ datapath for given vnic
 *
 * @param[in] oci_vnic VNIC configuration
 * @param[in] vnic     VNIC information to be stored in the db
 * @return #SDK_RET_OK on success, failure status code on error
 */
// TODO: these should be generated by NCC
#define local_vnic_by_vlan_tx_info local_vnic_by_vlan_tx_action_u.local_vnic_by_vlan_tx_local_vnic_info_tx
#define local_vnic_by_slot_rx_info local_vnic_by_slot_rx_action_u.local_vnic_by_slot_rx_local_vnic_info_rx
#define egress_local_vnic_info_rx egress_local_vnic_info_rx_action_u.egress_local_vnic_info_rx_egress_local_vnic_info_rx
sdk_ret_t
vnic_state::program_datapath(In_ oci_vnic_t *oci_vnic, In_ vnic_t *vnic) {
    sdk_ret_t                                   rv;
    subnet_t                                    *subnet;
    vcn_t                                       *vcn;
    local_vnic_by_vlan_tx_actiondata            vnic_by_vlan_data = { 0 };
    local_vnic_by_slot_rx_swkey_t               vnic_by_slot_key = { 0 };
    local_vnic_by_slot_rx_otcam_swkey_mask_t    vnic_by_slot_key_mask = { 0 };
    local_vnic_by_slot_rx_actiondata            vnic_by_slot_data = { 0 };
    egress_local_vnic_info_rx_actiondata        egress_vnic_data = { 0 };
    vnic_rx_stats_actiondata                    vnic_rx_stats_data = { 0 };
    vnic_tx_stats_actiondata                    vnic_tx_stats_data = { 0 };

    /**< find subnet state for this VNIC */
    subnet = g_subnet_state.subnet_find(oci_vnic->subnet_id);
    if (subnet == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    /**< find VCN state for this VNIC */
    vcn = g_vcn_db.vcn_find(oic_vnic->vcn_id);
    if (vcn == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    // TODO: not doing RMW operation yet (will do that when we come to update,
    //       right now table lib APIs have an issue with this)
    //
    /**< initialize rx & tx stats tables for this vnic */
    vnic_rx_stats_data.actionid = VNIC_RX_STATS_VNIC_RX_STATS_ID;
    pd_err = p4pd_global_entry_write(P4TBL_ID_VNIC_RX_STATS,
                                     vnic->hw_id, NULL, NULL,
                                     &vnic_rx_stats_data);
    if (pd_err != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    vnic_tx_stats_data.actionid = VNIC_TX_STATS_VNIC_TX_STATS_ID;
    pd_err = p4pd_global_entry_write(P4TBL_ID_VNIC_TX_STATS,
                                     vnic->hw_id, NULL, NULL,
                                     &vnic_tx_stats_data);
    if (pd_err != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    /**< initialize mapping table for this vnic */

    /**< initialize NAT table for this vnic */

    /**< initialize egress_local_vnic_info_rx table entry */
    egress_vnic_data.actionid =
        EGRESS_LOCAL_VNIC_INFO_RX_EGRESS_LOCAL_VNIC_INFO_RX_ID;
    egress_vnic_data.egress_local_vnic_info_rx.vr_mac = subnet->vr_mac;
    egress_vnic_data.egress_local_vnic_info_rx.overlay_mac = oci_vnic->mac_addr;
    egress_vnic_data.egress_local_vnic_info_rx.overlay_vlan_id =
        oci_vnic->vlan_id;
    egress_vnic_data.egress_local_vnic_info_rx.subnet_id = subnet->hw_id;
    rv = egress_local_vnic_info_rx_->insert_with_id(&egress_vnic_data,
                                                    vnic->hw_id);
    SDK_ASSERT_GOTO((rv == SDK_RET_OK), error);

    /**< initialize local_vnic_by_slot_rx table entry */
    vnic_by_slot_key.mpls_dst_label = oci_vnic->slot_id;
    vnic_by_slot_data.actionid = LOCAL_VNIC_BY_SLOT_RX_LOCAL_VNIC_INFO_RX_ID;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.local_vnic_tag =
        oci_vnic->hw_id;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.vcn_id = vcn->hw_id;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.skip_src_dst_check =
        oci_vnic->skip_src_dst_check;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.resource_group_1 =
        oci_vnic->rsc_pool_id;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.slacl_addr_1 =
        subnet->policy_base_addr;
    vnic_by_slot_data.local_vnic_by_slot_rx_info.epoch1 = 0;  // TODO: later !!
    rv = local_vnic_by_slot_rx_->insert(&key, &vnic_by_slot_data,
                                        &vnic->vnic_by_slot_hash_idx,
                                        &key_mask);  // TODO: fill in key_mask
    SDK_ASSERT_GOTO((rv == SDK_RET_OK), error);

    /**< initialize local_vnic_by_vlan_tx table entry */
    vnic_by_vlan_data.actionid = LOCAL_VNIC_BY_VLAN_TX_LOCAL_VNIC_INFO_TX_ID;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.local_vnic_tag =
        oci_vnic->hw_id;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.vcn_id = vcn->hw_id;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.skip_src_dst_check =
        oci_vnic->skip_src_dst_check;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.resource_group_1 =
        oci_vnic->rsc_pool_id;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.lpm_addr_1 =
        subnet->lpm_base_addr;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.slacl_addr_1 =
        subnet->policy_base_addr;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.epoch1 = 0;  // TODO: later !!
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.overlay_mac =
        oci_vnic->mac_addr;
    vnic_by_vlan_data.local_vnic_by_vlan_tx_info.src_slot_id = oci_vnic->slot;
    rv = local_vnic_by_vlan_tx_->insert_with_id(&vnic_by_vlan_data,
                                                oci_vnic->vlan_id);
    SDK_ASSERT_GOTO((rv == SDK_RET_OK), error);

error:

    cleanup_datapath(vcn);
    return rv;
}
/** @} */ // end of OCI_VNIC_DB

#endif

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
