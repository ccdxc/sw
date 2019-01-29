/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    subnet.cc
 *
 * @brief   subnet entry handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"

namespace api {

/**
 * @defgroup OCI_SUBNET_ENTRY - subnet entry functionality
 * @ingroup OCI_SUBNET
 * @{
 */

/**< @brief    constructor */
subnet_entry::subnet_entry() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
    hw_id_ = 0xFFFF;
    policy_base_addr_ = 0XFFFFFFFFFFFFFFFF;
}

/**
 * @brief    factory method to allocate and initialize a subnet entry
 * @param[in] oci_subnet    subnet information
 * @return    new instance of subnet or NULL, in case of error
 */
subnet_entry *
subnet_entry::factory(oci_subnet_t *oci_subnet) {
    subnet_entry *subnet;

    /**< create subnet entry with defaults, if any */
    subnet = subnet_db()->subnet_alloc();
    if (subnet) {
        new (subnet) subnet_entry();
    }
    return subnet;
}

/**< @brief    destructor */
subnet_entry::~subnet_entry() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
}

/**
 * @brief    release all the s/w & h/w state associated with this object, if
 *           any, and free the memory
 * @param[in] subnet     subnet to be freed
 * NOTE: h/w entries themselves should have been cleaned up (by calling
 *       impl->cleanup_hw() before calling this
 */
void
subnet_entry::destroy(subnet_entry *subnet) {
    subnet->release_resources_();
    subnet->~subnet_entry();
    subnet_db()->subnet_free(subnet);
}

/**
 * @brief     initialize subnet entry with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::init_config(api_ctxt_t *api_ctxt) {
    oci_subnet_t *oci_subnet = &api_ctxt->api_params->subnet_info;

    memcpy(&key_, &oci_subnet->key, sizeof(oci_subnet_key_t));
    memcpy(&route_table_, &oci_subnet->route_table,
           sizeof(oci_route_table_key_t));
    memcpy(&vr_mac_, &oci_subnet->vr_mac, sizeof(mac_addr_t));
    this->ht_ctxt_.reset();
    return SDK_RET_OK;
}

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::reserve_resources_(void) {
    if (subnet_db()->subnet_idxr()->alloc((uint32_t *)&this->hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return SDK_RET_OK;
}

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any, during creation of the object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::program_config(obj_ctxt_t *obj_ctxt) {
    /**
     * there is no h/w programming for subnet config but a h/w id is needed so
     * we can use while programming vnics, routes etc.
     */
    oci_subnet_t *oci_subnet = &obj_ctxt->api_params->subnet_info;
    OCI_TRACE_DEBUG("Creating subnet (vcn %u, subnet %u), pfx %s, vr ip %s, "
                    "vr_mac %s, route table %u", key_.vcn_id, key_.id,
                    ippfx2str(&oci_subnet->pfx), ipaddr2str(&oci_subnet->vr_ip),
                    macaddr2str(oci_subnet->vr_mac),
                    oci_subnet->route_table.id);
    return reserve_resources_();
}

/**
 * @brief     free h/w resources used by this object, if any
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::release_resources_(void) {
    if (hw_id_ != 0xFF) {
        subnet_db()->subnet_idxr()->free(hw_id_);
    }
    if (policy_base_addr_ != 0xFFFFFFFFFFFFFFFF) {
        // TODO: free this block
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
subnet_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    // impl->cleanup_hw();
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
subnet_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // impl->update_hw();
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
sdk_ret_t
subnet_entry::activate_config(oci_epoch_t epoch, api_op_t api_op,
                              obj_ctxt_t *obj_ctxt) {
    /**< there is no h/w programming for subnet config, so nothing to activate */
    OCI_TRACE_DEBUG("Created subnet (vcn %u, subnet %u)", key_.vcn_id, key_.id);
    return SDK_RET_OK;
}

/**
 * @brief    this method is called on new object that needs to replace the
 *           old version of the object in the DBs
 * @param[in] orig_obj    old version of the unmodified object
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
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
subnet_entry::commit(api_ctxt_t *api_ctxt) {
    return SDK_RET_OK;
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
subnet_entry::abort(api_ctxt_t *api_ctxt) {
    return SDK_RET_OK;
}
#endif

/**
 * @brief add subnet to database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::add_to_db(void) {
    return subnet_db()->subnet_ht()->insert_with_key(&key_, this,
                                                     &ht_ctxt_);
}

/**
 * @brief delete subnet from database
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::del_from_db(void) {
    subnet_db()->subnet_ht()->remove(&key_);
    return SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
subnet_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_ID_SUBNET, this);
}

/** @} */    // end of OCI_SUBNET_ENTRY

}    // namespace api
