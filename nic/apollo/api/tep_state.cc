/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    tep_state.cc
 *
 * @brief   Tunnel EndPoint (TEP) database handling
 */

#include <stdio.h>
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/tep_state.hpp"

using sdk::lib::ht;

namespace api {

/**
 * @defgroup OCI_TEP_STATE - tep database functionality
 * @ingroup OCI_TEP
 * @{
 */

/**
 * @brief    constructor
 */
tep_state::tep_state() {
    // TODO: need to tune multi-threading related params later
    tep_ht_ = ht::factory(OCI_MAX_TEP >> 2,
                          tep_entry::tep_key_func_get,
                          tep_entry::tep_hash_func_compute,
                          tep_entry::tep_key_func_compare);
    SDK_ASSERT(tep_ht_ != NULL);

    tep_slab_ = slab::factory("tep", OCI_SLAB_ID_TEP, sizeof(tep_entry),
                               16, true, true, true, NULL);
    SDK_ASSERT(tep_slab_ != NULL);
}

/**
 * @brief    destructor
 */
tep_state::~tep_state() {
    ht::destroy(tep_ht_);
    slab::destroy(tep_slab_);
}

/**
 * @brief     allocate tep instance
 * @return    pointer to the allocated tep , NULL if no memory
 */
tep_entry *
tep_state::tep_alloc(void) {
    return ((tep_entry *)tep_slab_->alloc());
}

/**
 * @brief      free tep instance back to slab
 * @param[in]  tep   pointer to the allocated tep
 */
void
tep_state::tep_free(tep_entry *tep) {
    tep_slab_->free(tep);
}

/**
 * @brief        lookup tep in database with given key
 * @param[in]    tep_key tep key
 * @return       pointer to the tep instance found or NULL
 */
tep_entry *
tep_state::tep_find(oci_tep_key_t *tep_key) const {
    return (tep_entry *)(tep_ht_->lookup(tep_key));
}

/** @} */    // end of OCI_TEP_STATE

}    // namespace api
