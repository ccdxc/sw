/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    vnic_state.cc
 *
 * @brief   vnic database handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/vnic_state.hpp"

namespace api {

/**
 * @defgroup OCI_VNIC_STATE - vnic database functionality
 * @ingroup OCI_VNIC
 * @{
 */

/**
 * @brief    constructor
 */
vnic_state::vnic_state() {
    // TODO: need to tune multi-threading related params later
    vnic_ht_ = ht::factory(OCI_MAX_VNIC >> 2,
                           vnic_entry::vnic_key_func_get,
                           vnic_entry::vnic_hash_func_compute,
                           vnic_entry::vnic_key_func_compare);
    SDK_ASSERT(vnic_ht_ != NULL);

    vnic_slab_ = slab::factory("vnic", OCI_SLAB_ID_VNIC, sizeof(vnic_entry),
                               16, true, true, true, NULL);
    SDK_ASSERT(vnic_slab_ != NULL);
}

/**
 * @brief    destructor
 */
vnic_state::~vnic_state() {
    ht::destroy(vnic_ht_);
    slab::destroy(vnic_slab_);
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
 * @brief      free vnic instance back to slab
 * @param[in]  vnic   pointer to the allocated vnic
 */
void
vnic_state::vnic_free(vnic_entry *vnic) {
    vnic_slab_->free(vnic);
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

/** @} */    // end of OCI_VNIC_STATE

}    // namespace api
