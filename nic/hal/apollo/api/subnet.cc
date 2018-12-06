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
 * @brief    release all the s/w state associate with the given subnet, if any,
 *           and free the memory
 * @param[in] subnet     subnet to be freed
 * NOTE: h/w entries should have been cleaned up (by calling cleanup_hw()
 * before calling this
 */
void
subnet_entry::destroy(subnet_entry *subnet) {
    subnet->~subnet_entry();
    subnet_db()->subnet_free(subnet);
}

/**
 * @brief     initialize subnet entry with the given config
 * @param[in] oci_subnet    subnet information
 * @return    SDK_RET_OK on success, failure status code on error
 *
 * NOTE:     allocate all h/w resources (i.e., table indices as well here, we
 *           can always release them in abort phase if something goes wrong
 */
sdk_ret_t
subnet_entry::init(oci_subnet_t *oci_subnet) {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_SHARED);
    memcpy(&this->key_, &oci_subnet->key, sizeof(oci_subnet_key_t));
    this->ht_ctxt_.reset();
    if (subnet_db()->subnet_idxr()->alloc((uint32_t *)&this->hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
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

    subnet = subnet_db()->subnet_alloc();
    if (subnet) {
        new (subnet) subnet_entry();
        if (subnet->init(oci_subnet) == sdk::SDK_RET_OK) {
            return subnet;
        } else {
            subnet_entry::destroy(subnet);
            return NULL;
        }
    }
    return NULL;
}

/**
 * @brief    process a create/delete/update/get operation on a subnet
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::process_api(api_ctxt_t *api_ctxt) {
    switch (api_ctxt->op) {
    case API_OP_CREATE:
        return process_create(api_ctxt);
        break;
    case API_OP_UPDATE:
        return process_delete(api_ctxt);
        break;
    case API_OP_DELETE:
        return process_delete(api_ctxt);
        break;
    case API_OP_GET:
        return process_get(api_ctxt);
        break;
    default:
        return sdk::SDK_RET_INVALID_OP;
    }
}

/**
 * @brief     handle a subnet create by allocating all required resources
 *            and keeping them ready for commit phase
 * @param[in] api_ctxt    transient state associated with this API
 * @return   SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_entry::process_create(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_OK;
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
 * @brief cleanup state maintained for given subnet
 *
 * @param[in] subnet subnet
 */
void
subnet_db::subnet_cleanup(subnet_t *subnet) {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&subnet->slock);
    subnet_idxr_->free(subnet->hw_id);
}

/**
 * @brief Free subnet structure
 *
 * @param[in] subnet subnet
 */
void
subnet_db::subnet_free(subnet_t *subnet) {
    api::delay_delete_to_slab(OCI_SLAB_SUBNET, subnet);
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
    api_ctxt.op = API_OP_CREATE;
    api_ctxt.id = API_ID_SUBNET_CREATE;
    api_ctxt.api_info = subnet;    // TODO: this may not be acceptable usage if caller passed this from stack
    rv = g_api_engine.process_api(&api_ctxt);
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
    api_ctxt.op = API_OP_DELETE;
    api_ctxt.id = API_ID_SUBNET_DELETE;
    api_ctxt.api_info = subnet_key;    // TODO: this may not be acceptable usage if caller passed this from stack
    rv = g_api_engine.process_api(&api_ctxt);
    return rv;
}

/** @} */    // end of OCI_SUBNET_API

}    // namespace api
