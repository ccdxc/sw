/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.cc
 *
 * @brief   This file deals with OCI VNIC API handling
 */

#include <stdio.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/hal/apollo/api/mem.hpp"
#include "nic/hal/apollo/api/vnic.hpp"

namespace api {

/**
 * @defgroup OCI_VNIC_DB - VNIC database functionality
 * @ingroup OCI_VNIC
 * @{
 */

vnic_state g_vnic_state;

/**
 * constructor
 */
vnic_state::vnic_state() {
    // TODO: need to tune multi-threading related params later
    ht_ = ht::factory(OCI_MAX_VNIC >> 2,
                      vnic_key_func_get,
                      vnic_hash_func_compute,
                      vnic_key_func_compare);
    slab_ = slab::factory("vnic", OCI_SLAB_VNIC, sizeof(vnic_t), 16,
                          true, true, true, NULL);
}

/**
 * destructor
 */
vnic_state::~vnic_state() {
    ht::destroy(ht_);
    slab::destroy(slab_);
}

/**
 * @brief Handle VNIC create message
 *
 * @param[in] vnic VNIC information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_state::vnic_create(_In_ oci_vnic_t *oci_vnic) {
    vnic_t *vnic;

    if ((vnic = vnic_alloc_init(oci_vnic)) == NULL) {
        return sdk::SDK_RET_OOM;
    }

    // TODO: handle all the table programming here !!
    return sdk::SDK_RET_OK;
}

/**
 * @brief Handle VNIC delete API
 *
 * @param[in] vnic_key VNIC key information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
vnic_state::vnic_delete(_In_ oci_vnic_key_t *vnic_key) {
    vnic_t *vnic;

    if ((vnic = vnic_del_from_db(vnic_key)) == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    // TODO: clear all mappings and delete all p4pd table entires etc.
    vnic_delete(vnic);

    return sdk::SDK_RET_OK;
}

/**
 * @brief Add VNIC to database
 *
 * @param[in] vnic VNIC
 */
sdk_ret_t
vnic_state::vnic_add_to_db(vnic_t *vnic) {
    return ht_->insert_with_key(&vnic->key, vnic,
                                &vnic->ht_ctxt);
}

/**
 * @brief Delete VNIC from database
 *
 * @param[in] vnic_key VNIC key
 */
vnic_t *
vnic_state::vnic_del_from_db(oci_vnic_key_t *vnic_key) {
    return (vnic_t *)(ht_->remove(vnic_key));
}

/**
 * @brief Lookup VNIC in database
 *
 * @param[in] vnic_key VNIC key
 */
vnic_t *
vnic_state::vnic_find(oci_vnic_key_t *vnic_key) const {
    return (vnic_t *)(ht_->lookup(vnic_key));
}

/**
 * @brief Allocate VNIC structure
 *
 * @return Pointer to the allocated internal VNIC, NULL if no memory
 */
vnic_t *
vnic_state::vnic_alloc(void) {
    return ((vnic_t *)slab_->alloc());
}

/**
 * @brief Initialize internal VNIC structure
 *
 * @param[in] vnic VNIC structure to store the state
 * @param[in] oci_vnic VNIC specific information
 */
sdk_ret_t
vnic_state::vnic_init(vnic_t *vnic, oci_vnic_t *oci_vnic) {
    //SDK_SPINLOCK_INIT(&vnic->slock, PTHREAD_PROCESS_SHARED);
    memcpy(&vnic->key, &oci_vnic->key, sizeof(oci_vnic_key_t));
    vnic->ht_ctxt.reset();
    if (idxr_->alloc(&vnic->id) != sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return sdk::SDK_RET_OK;
}

/**
 * @brief Allocate and initialize internal VNIC structure
 *
 * @return Pointer to the allocated and initialized internal VNIC,
 *         NULL if no memory
 */
vnic_t *
vnic_state::vnic_alloc_init(oci_vnic_t *oci_vnic) {
    vnic_t *vnic;

    if ((vnic = vnic_alloc()) == NULL) {
        return NULL;
    }
    vnic_init(vnic, oci_vnic);
    return vnic;
}

/**
 * @brief Cleanup state maintained for given VNIC
 *
 * @param[in] vnic VNIC
 */
void
vnic_state::vnic_cleanup(vnic_t *vnic) {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&vnic->slock);
    idxr_->free(vnic->id);
}

/**
 * @brief Free VNIC structure
 *
 * @param[in] vnic VNIC
 */
void
vnic_state::vnic_free(vnic_t *vnic) {
    api::delay_delete_to_slab(OCI_SLAB_VNIC, vnic);
}

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
/** @} */ // end of OCI_VNIC_DB

/**
 * @brief Create VNIC
 *
 * @param[in] vnic VNIC information
 * @return #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
oci_vnic_create (_In_ oci_vnic_t *vnic)
{
    sdk_ret_t rv;

    rv = g_vnic_state.vnic_create(vnic);
    return rv;
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
    sdk_ret_t rv;

    rv = g_vnic_state.vnic_delete(vnic_key);
    return rv;
}

/** @} */ // end of OCI_VNIC_API

}    // namespace api
