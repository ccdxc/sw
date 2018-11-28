/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vcn.cc
 *
 * @brief   This file deals with OCI vcn API handling
 */

#include <stdio.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/hal/apollo/core/mem.hpp"
#include "nic/hal/apollo/api/vcn.hpp"

namespace api {

/**
 * @defgroup OCI_VCN_DB - vcn database functionality
 * @ingroup OCI_VCN
 * @{
 */

vcn_state g_vcn_state;

/**
 * constructor
 */
vcn_state::vcn_state() {
    // TODO: need to tune multi-threading related params later
    vcn_ht_ = ht::factory(OCI_MAX_VCN >> 1,
                          vcn_key_func_get,
                          vcn_hash_func_compute,
                          vcn_key_func_compare);
    vcn_idxr_ = indexer::factory(OCI_MAX_VCN);
    vcn_slab_ = slab::factory("vcn", OCI_SLAB_VCN, sizeof(vcn_t),
                              16, true, true, NULL);
}

/**
 * destructor
 */
vcn_state::~vcn_state() {
    ht::destroy(vcn_ht_);
    indexer::destroy(vcn_idxr_);
    slab::destroy(vcn_slab_);
}

/**
 * @brief handle vcn create message
 *
 * @param[in] vcn vcn information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_state::vcn_create(_In_ oci_vcn_t *oci_vcn) {
    vcn_t *vcn;

    if ((vcn = vcn_alloc_init(oci_vcn)) == NULL) {
        return sdk::SDK_RET_OOM;
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief handle vcn delete API
 *
 * @param[in] vcn_key vcn key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vcn_state::vcn_delete(_In_ oci_vcn_key_t *vcn_key) {
    vcn_t *vcn;

    if ((vcn = vcn_del_from_db(vcn_key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    vcn_delete(vcn);

    return sdk::SDK_RET_OK;
}

/**
 * @brief add vcn to database
 *
 * @param[in] vcn vcn
 */
sdk_ret_t
vcn_state::vcn_add_to_db(vcn_t *vcn) {
    return vcn_ht_->insert_with_key(&vcn->key, vcn,
                                    &vcn->ht_ctxt);
}

/**
 * @brief delete vcn from database
 *
 * @param[in] vcn_key vcn key
 */
vcn_t *
vcn_state::vcn_del_from_db(oci_vcn_key_t *vcn_key) {
    return (vcn_t *)(vcn_ht_->remove(vcn_key));
}

/**
 * @brief Lookup vcn in database
 *
 * @param[in] vcn_key vcn key
 */
vcn_t *
vcn_state::vcn_find(oci_vcn_key_t *vcn_key) const {
    return (vcn_t *)(vcn_ht_->lookup(vcn_key));
}

/**
 * @brief Allocate vcn structure
 *
 * @return Pointer to the allocated internal vcn, NULL if no memory
 */
vcn_t *
vcn_state::vcn_alloc(void) {
    return ((vcn_t *)vcn_slab_->alloc());
}

/**
 * @brief Initialize internal vcn structure
 *
 * @param[in] vcn vcn structure to store the state
 * @param[in] oci_vcn vcn specific information
 */
sdk_ret_t
vcn_state::vcn_init(vcn_t *vcn, oci_vcn_t *oci_vcn) {
    //SDK_SPINLOCK_INIT(&vcn->slock, PTHREAD_PROCESS_SHARED);
    memcpy(&vcn->key, &oci_vcn->key, sizeof(oci_vcn_key_t));
    vcn->ht_ctxt.reset();
    if (vcn_idxr_->alloc((uint32_t *)&vcn->hw_id) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief Allocate and initialize internal vcn structure
 *
 * @return Pointer to the allocated and initialized internal vcn,
 *         NULL if no memory
 */
vcn_t *
vcn_state::vcn_alloc_init(oci_vcn_t *oci_vcn) {
    vcn_t *vcn;

    if ((vcn = vcn_alloc()) == NULL) {
        return NULL;
    }
    vcn_init(vcn, oci_vcn);
    return vcn;
}

/**
 * @brief cleanup state maintained for given vcn
 *
 * @param[in] vcn vcn
 */
void
vcn_state::vcn_cleanup(vcn_t *vcn) {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&vcn->slock);
    vcn_idxr_->free(vcn->hw_id);
}

/**
 * @brief Free vcn structure
 *
 * @param[in] vcn vcn
 */
void
vcn_state::vcn_free(vcn_t *vcn) {
    api::delay_delete_to_slab(OCI_SLAB_VCN, vcn);
}

/**
 * @brief Uninitialize and free internal vcn structure
 *
 * @param[in] vcn vcn
 */
void
vcn_state::vcn_delete(_In_ vcn_t *vcn) {
    if (vcn) {
        vcn_cleanup(vcn);
        vcn_free(vcn);
    }
}
/** @} */ // end of OCI_VCN_DB

/**
 * @defgroup OCI_VCN_API - First level of SUBNET API handling
 * @ingroup OCI_VCN
 * @{
 */

/**
 * @brief create vcn
 *
 * @param[in] vcn vcn information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vcn_create (_In_ oci_vcn_t *vcn)
{
    sdk_ret_t rv;

    if ((rv = g_vcn_state.vcn_create(vcn)) != sdk::SDK_RET_OK) {
        return rv;
    }

    return sdk::SDK_RET_OK;
}

/**
 * @brief delete vcn
 *
 * @param[in] vcn_key vcn key
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vcn_delete (_In_ oci_vcn_key_t *vcn_key)
{
    sdk_ret_t rv;

    if ((rv = g_vcn_state.vcn_delete(vcn_key)) != sdk::SDK_RET_OK) {
        return rv;
    }

    return sdk::SDK_RET_OK;
}

/** @} */ // end of OCI_VCN_API

}    // namespace api
