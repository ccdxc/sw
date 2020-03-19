//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// security policy datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/impl/apulu/security_policy_impl.hpp"
#include "nic/apollo/api/impl/apulu/security_policy_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_SECURITY_POLICY_IMPL_STATE - security policy database
///                                          functionality
/// \ingroup PDS_SECURITY_POLICY
/// @{

security_policy_impl_state::security_policy_impl_state(pds_state *state) {
    v4_idxr_ =
        rte_indexer::factory(state->mempartition()->block_count("sacl_v4"),
                             false, false);
    SDK_ASSERT(v4_idxr_ != NULL);
    v6_idxr_ =
        rte_indexer::factory(state->mempartition()->block_count("sacl_v6"),
                             false, false);
    SDK_ASSERT(v6_idxr_ != NULL);
    v4_region_addr_ = state->mempartition()->start_addr("sacl_v4");
    SDK_ASSERT(v4_region_addr_ != INVALID_MEM_ADDRESS);
    v4_table_size_ = state->mempartition()->block_size("sacl_v4");
    v4_max_rules_ = state->mempartition()->max_elements("sacl_v4") - 1;
    v6_region_addr_ = state->mempartition()->start_addr("sacl_v6");
    SDK_ASSERT(v6_region_addr_ != INVALID_MEM_ADDRESS);
    v6_table_size_ = state->mempartition()->block_size("sacl_v6");
    v6_max_rules_ = state->mempartition()->max_elements("sacl_v6") - 1;
}

security_policy_impl_state::~security_policy_impl_state() {
    rte_indexer::destroy(v4_idxr_);
    rte_indexer::destroy(v6_idxr_);
}

security_policy_impl *
security_policy_impl_state::alloc(void) {
    return (security_policy_impl *)
               SDK_CALLOC(SDK_MEM_ALLOC_PDS_SECURITY_POLICY_IMPL,
                          sizeof(security_policy_impl));
}

void
security_policy_impl_state::free(security_policy_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_PDS_SECURITY_POLICY_IMPL, impl);
}

sdk_ret_t
security_policy_impl_state::table_transaction_begin(void) {
    return SDK_RET_OK;
}

sdk_ret_t
security_policy_impl_state::table_transaction_end(void) {
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
