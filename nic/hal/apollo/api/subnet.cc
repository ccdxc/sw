/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    subnet.cc
 *
 * @brief   This file deals with OCI subnet API handling
 */

#include <stdio.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/subnet.hpp"
#include "nic/hal/apollo/api/oci_state.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/framework/api_engine.hpp"

using namespace sdk;

namespace api {

/**
 * @defgroup OCI_SUBNET_ENTRY - subnet entry functionality
 * @ingroup OCI_SUBNET
 * @{
 */

/**
 * @brief    cleanup state maintained for this subnet including any hw entries
 *           we are holding on to
 */
void
subnet_entry::cleanup(void) {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
    // TODO: check if indices are valid before calling tbl mgmt. APIs
    subnet_db()->subnet_idxr()->free(hw_id_);
}

/**
 * @brief    release all the s/w state associate with the given subnet, if any,
 *           and free the memory
 * @param[in] subnet     subnet to be freed
 * NOTE: h/w entries should have been cleaned up (by calling cleanup_hw()
 * before calling this
 */
void
subnet_entry::destroy(subnet_entry *subnet) {
    subnet->cleanup();
    subnet->~subnet_entry();
}

/**
 * @brief     initialize subnet entry with the given config
 * @param[in] api_ctxt API context carrying the configuration
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::init_config(api_ctxt_t *api_ctxt) {
    oci_subnet_t *oci_subnet = &api_ctxt->subnet_info;

    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_SHARED);
    memcpy(&this->key_, &oci_subnet->key, sizeof(oci_subnet_key_t));
    this->ht_ctxt_.reset();
    return sdk::SDK_RET_OK;
}

/**
 * @brief    allocate h/w resources for this object
 * @return    SDK_RET_OK on success, failure status code on error
 */
// TODO: this should ideally go to impl class
sdk_ret_t
subnet_entry::alloc_resources(void) {
    if (subnet_db()->subnet_idxr()->alloc((uint32_t *)&this->hw_id_) !=
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
subnet_entry::update_config(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
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

#if 0
/**
 * @brief     handle a subnet create by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::process_create(api_ctxt_t *api_ctxt) {
    return init(&api_ctxt->subnet_info);
}

/**
 * @brief     handle a subnet update by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::process_update(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief     handle a subnet delete by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::process_delete(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}

/**
 * @brief     handle a subnet get by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::process_get(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}
#endif

/**
 * @brief    program all h/w tables relevant to this object except stage 0
 *           table(s), if any
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::program_hw(obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    cleanup all h/w tables relevant to this object except stage 0
 *           table(s), if any, by updating packed entries with latest epoch#
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::cleanup_hw(obj_ctxt_t *obj_ctxt) {
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
subnet_entry::update_hw(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

/**
 * @brief    activate the epoch in the dataplane
 * @param[in] api_op      api operation
 * @param[in] obj_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::activate_epoch(api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
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
subnet_entry::abort(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
}
#endif

/**
 * @brief add subnet to database
 *
 * @param[in] subnet subnet
 */
sdk_ret_t
subnet_entry::add_to_db(void) {
    return subnet_db()->subnet_ht()->insert_with_key(&key_, this,
                                                     &ht_ctxt_);
}

/**
 * @brief delete subnet from database
 *
 * @param[in] subnet_key subnet key
 */
sdk_ret_t
subnet_entry::del_from_db(void) {
    subnet_db()->subnet_ht()->remove(&key_);
    return sdk::SDK_RET_OK;
}

/**
 * @brief    initiate delay deletion of this object
 */
sdk_ret_t
subnet_entry::delay_delete(void) {
    return delay_delete_to_slab(OCI_SLAB_SUBNET, this);
}

/** @} */    // end of OCI_SUBNET_ENTRY

/**
 * @defgroup OCI_SUBNET_STATE - subnet database functionality
 * @ingroup OCI_SUBNET
 * @{
 */

/**
 * @brief    constructor
 */
subnet_state::subnet_state() {
    // TODO: need to tune multi-threading related params later
    subnet_ht_ = ht::factory(OCI_MAX_SUBNET >> 1,
                             subnet_entry::subnet_key_func_get,
                             subnet_entry::subnet_hash_func_compute,
                             subnet_entry::subnet_key_func_compare);
    SDK_ASSERT(subnet_ht_!= NULL);
    subnet_idxr_ = indexer::factory(OCI_MAX_SUBNET);
    SDK_ASSERT(subnet_idxr_ != NULL);
    subnet_slab_ = slab::factory("subnet", OCI_SLAB_SUBNET,
                                 sizeof(subnet_entry),
                                 16, true, true, NULL);
    SDK_ASSERT(subnet_slab_ != NULL);
}

/**
 * @brief    destructor
 */
subnet_state::~subnet_state() {
    ht::destroy(subnet_ht_);
    indexer::destroy(subnet_idxr_);
    slab::destroy(subnet_slab_);
}

/**
 * @brief     allocate subnet instance
 * @return    pointer to the allocated subnet, NULL if no memory
 */
subnet_entry *
subnet_state::subnet_alloc(void) {
    return ((subnet_entry *)subnet_slab_->alloc());
}

/**
 * @brief        lookup subnet in database with given key
 * @param[in]    subnet_key subnet  key
 * @return       pointer to the subnet instance found or NULL
 */
subnet_entry *
subnet_state::subnet_find(oci_subnet_key_t *subnet_key) const {
    return (subnet_entry *)(subnet_ht_->lookup(subnet_key));
}

/**
 * @brief        free subnet instance
 * @param[in] subnet subnet instance
 */
void
subnet_state::subnet_free(subnet_entry *subnet) {
    api::delay_delete_to_slab(OCI_SLAB_SUBNET, subnet);
}

/** @} */    // end of OCI_SUBNET_STATE

#if 0
/**
 * @brief handle subnet create message
 *
 * @param[in] subnet subnet information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_db::subnet_create(_In_ oci_subnet_t *oci_subnet) {
    subnet_t *subnet;

    if ((subnet = subnet_alloc_init(oci_subnet)) == NULL) {
        return sdk::SDK_RET_OOM;
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief handle subnet delete API
 *
 * @param[in] subnet_key subnet key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_db::subnet_delete(_In_ oci_subnet_key_t *subnet_key) {
    subnet_t *subnet;

    if ((subnet = subnet_del_from_db(subnet_key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    subnet_delete(subnet);

    return sdk::SDK_RET_OK;
}

/**
 * @brief Uninitialize and free internal subnet structure
 *
 * @param[in] subnet subnet
 */
void
subnet_db::subnet_delete(_In_ subnet_t *subnet) {
    if (subnet) {
        subnet_cleanup(subnet);
        subnet_free(subnet);
    }
}
#endif

}    // namespace api

/**
 * @defgroup OCI_SUBNET_API - First level of subnet API handling
 * @ingroup OCI_SUBNET
 * @{
 */

/**
 * @brief create subnet
 *
 * @param[in] subnet subnet information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_subnet_create (_In_ oci_subnet_t *subnet)
{
    api_ctxt_t    api_ctxt;
    sdk_ret_t     rv;

    memset(&api_ctxt, 0, sizeof(api_ctxt));
    api_ctxt.api_op = api::API_OP_CREATE;
    api_ctxt.obj_id = api::OBJ_ID_SUBNET;
    api_ctxt.subnet_info = *subnet;
    rv = api::g_api_engine.process_api(&api_ctxt);
    return rv;
}

/**
 * @brief delete subnet
 *
 * @param[in] subnet_key subnet key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_subnet_delete (_In_ oci_subnet_key_t *subnet_key)
{
    api_ctxt_t    api_ctxt;   // TODO: get this from slab ??
    sdk_ret_t     rv;

    memset(&api_ctxt, 0, sizeof(api_ctxt));
    api_ctxt.api_op = api::API_OP_DELETE;
    api_ctxt.obj_id = api::OBJ_ID_SUBNET;
    api_ctxt.subnet_key = *subnet_key;
    rv = api::g_api_engine.process_api(&api_ctxt);
    return rv;
}

/** @} */    // end of OCI_SUBNET_API
