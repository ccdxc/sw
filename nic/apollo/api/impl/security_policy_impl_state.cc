/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    security_policy_impl_state.cc
 *
 * @brief   This file contains security policy datapath database handling
 */

#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/impl/security_policy_impl_state.hpp"

namespace api {
namespace impl {

/**
 * @defgroup PDS_SECURITY_POLICY_IMPL_STATE - security policy database
 *                                            functionality
 * @ingroup PDS_SECURITY_POLICY
 * @{
 */

/**
 * @brief    constructor
 */
security_policy_impl_state::security_policy_impl_state(pds_state *state) {
    /**
     * we need max + 1 blocks, extra 1 block for processing updates for
     * routing table (with the assumption that more than one routing table
     * is not updated in any given batch
     */
    v4_idxr_ =
        indexer::factory(PDS_MAX_RULES_PER_SECURITY_POLICY + 1);
    SDK_ASSERT(v4_idxr_ != NULL);
    v4_region_addr_ =
        state->mempartition()->start_addr("sacl_v4");
    v4_table_size_ =
        state->mempartition()->block_size("sacl_v4");
}

/**
 * @brief    destructor
 */
security_policy_impl_state::~security_policy_impl_state() {
    indexer::destroy(v4_idxr_);
}

/**
 * @brief    API to initiate transaction over all the table manamgement
 *           library instances
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
security_policy_impl_state::table_transaction_begin(void) {
    //v4_idxr_->txn_start();
    //v6_idxr_->txn_start();
    return SDK_RET_OK;
}

/**
 * @brief    API to end transaction over all the table manamgement
 *           library instances
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
security_policy_impl_state::table_transaction_end(void) {
    //v4_idxr_->txn_end();
    //v6_idxr_->txn_end();
    return SDK_RET_OK;
}

/** @} */    // end of PDS_SECURITY_POLICY_IMPL_STATE

}    // namespace impl
}    // namespace api
