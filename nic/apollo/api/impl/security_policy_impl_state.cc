/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    security_policy_impl_state.cc
 *
 * @brief   This file contains security policy datapath database handling
 */

#include "nic/apollo/include/api/oci_security_policy.hpp"
#include "nic/apollo/api/impl/security_policy_impl_state.hpp"

namespace impl {

/**
 * @defgroup OCI_SECURITY_POLICY_IMPL_STATE - security policy database
 *                                            functionality
 * @ingroup OCI_SECURITY_POLICY
 * @{
 */

/**
 * @brief    constructor
 */
security_policy_impl_state::security_policy_impl_state(oci_state *state) {
    /**
     * we need max + 1 blocks, extra 1 block for processing updates for
     * routing table (with the assumption that more than one routing table
     * is not updated in any given batch
     */
    security_policy_idxr_ =
        indexer::factory(OCI_MAX_RULES_PER_SECURITY_POLICY + 1);
    SDK_ASSERT(security_policy_idxr_ != NULL);
    security_policy_region_addr_ =
        state->mempartition()->start_addr("slacl");
    security_policy_table_size_ =
        state->mempartition()->element_size("slacl");
}

/**
 * @brief    destructor
 */
security_policy_impl_state::~security_policy_impl_state() {
    indexer::destroy(security_policy_idxr_);
}

/** @} */    // end of OCI_SECURITY_POLICY_IMPL_STATE

}    // namespace impl
