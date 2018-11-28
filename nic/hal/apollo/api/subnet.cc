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

using namespace sdk;

namespace api {

/**
 * @defgroup OCI_SUBNET_DB - subnet database functionality
 * @ingroup OCI_SUBNET
 * @{
 */

/**
 * constructor
 */
subnet_state::subnet_state() {
    // TODO: create slab_ as well !!
    subnet_ht_ = sdk::lib::ht::factory(OCI_MAX_SUBNET>> 1,
                                       subnet_key_func_get,
                                       subnet_hash_func_compute,
                                       subnet_key_func_compare);
    SDK_ASSERT(subnet_ht_ != NULL);
    subnet_idxr_ = sdk::lib::indexer::factory(OCI_MAX_SUBNET);
    SDK_ASSERT(subnet_idxr_ != NULL);
}

/**
 * destructor
 */
subnet_state::~subnet_state() {
    ht::destroy(subnet_ht_);
    indexer::destroy(subnet_idxr_);
    slab::destroy(subnet_slab_);
}

/**
 * @brief handle subnet create message
 *
 * @param[in] subnet subnet information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
subnet_state::subnet_create(_In_ oci_subnet_t *oci_subnet) {
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
subnet_state::subnet_delete(_In_ oci_subnet_key_t *subnet_key) {
    subnet_t *subnet;

    if ((subnet = subnet_del_from_db(subnet_key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    subnet_delete(subnet);

    return sdk::SDK_RET_OK;
}

/**
 * @brief add subnet to database
 *
 * @param[in] subnet subnet
 */
sdk_ret_t
subnet_state::subnet_add_to_db(subnet_t *subnet) {
    return subnet_ht_->insert_with_key(&subnet->key, subnet,
                                       &subnet->ht_ctxt);
}

/**
 * @brief delete subnet from database
 *
 * @param[in] subnet_key subnet key
 */
subnet_t *
subnet_state::subnet_del_from_db(oci_subnet_key_t *subnet_key) {
    return (subnet_t *)(subnet_ht_->remove(subnet_key));
}

/**
 * @brief Lookup subnet in database
 *
 * @param[in] subnet_key subnet key
 */
subnet_t *
subnet_state::subnet_find(oci_subnet_key_t *subnet_key) const {
    return (subnet_t *)(subnet_ht_->lookup(subnet_key));
}

/**
 * @brief Allocate subnet structure
 *
 * @return Pointer to the allocated internal subnet, NULL if no memory
 */
subnet_t *
subnet_state::subnet_alloc(void) {
    return ((subnet_t *)subnet_slab_->alloc());
}

/**
 * @brief Initialize internal subnet structure
 *
 * @param[in] subnet subnet structure to store the state
 * @param[in] oci_subnet subnet specific information
 */
sdk_ret_t
subnet_state::subnet_init(subnet_t *subnet, oci_subnet_t *oci_subnet) {
    //SDK_SPINLOCK_INIT(&subnet->slock, PTHREAD_PROCESS_SHARED);
    memcpy(&subnet->key, &oci_subnet->key, sizeof(oci_subnet_key_t));
    subnet->ht_ctxt.reset();
    if (subnet_idxr_->alloc((uint32_t *)&subnet->hw_id) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief Allocate and initialize internal subnet structure
 *
 * @return Pointer to the allocated and initialized internal subnet,
 *         NULL if no memory
 */
subnet_t *
subnet_state::subnet_alloc_init(oci_subnet_t *oci_subnet) {
    subnet_t *subnet;

    if ((subnet = subnet_alloc()) == NULL) {
        return NULL;
    }
    subnet_init(subnet, oci_subnet);
    return subnet;
}

/**
 * @brief cleanup state maintained for given subnet
 *
 * @param[in] subnet subnet
 */
void
subnet_state::subnet_cleanup(subnet_t *subnet) {
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
subnet_state::subnet_free(subnet_t *subnet) {
    api::delay_delete_to_slab(OCI_SLAB_SUBNET, subnet);
}

/**
 * @brief Uninitialize and free internal subnet structure
 *
 * @param[in] subnet subnet
 */
void
subnet_state::subnet_delete(_In_ subnet_t *subnet) {
    if (subnet) {
        subnet_cleanup(subnet);
        subnet_free(subnet);
    }
}

/** @} */ // end of OCI_SUBNET_DB

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
    sdk_ret_t rv;

    if ((rv = g_subnet_state.subnet_create(subnet)) != sdk::SDK_RET_OK) {
        return rv;
    }

    return sdk::SDK_RET_OK;
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
    sdk_ret_t rv;

    if ((rv = g_subnet_state.subnet_delete(subnet_key)) != sdk::SDK_RET_OK) {
        return rv;
    }

    return sdk::SDK_RET_OK;
}

/** @} */ // end of OCI_SUBNET_API

}  // namespace api 
