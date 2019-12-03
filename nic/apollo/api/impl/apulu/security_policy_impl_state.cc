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
    // we need max + 1 blocks, extra 1 block for processing updates for
    // routing table (with the assumption that more than one routing table
    // is not updated in any given batch
    egr_v4_idxr_ =
        indexer::factory(PDS_MAX_SECURITY_POLICY + 1);
    SDK_ASSERT(egr_v4_idxr_ != NULL);
    egr_v4_region_addr_ =
        state->mempartition()->start_addr("sacl_egress_v4");
    SDK_ASSERT(egr_v4_region_addr_ != INVALID_MEM_ADDRESS);
    egr_v4_table_size_ =
        state->mempartition()->block_size("sacl_egress_v4");
    ing_v4_idxr_ =
        indexer::factory(PDS_MAX_SECURITY_POLICY + 1);
    SDK_ASSERT(ing_v4_idxr_ != NULL);
    ing_v4_region_addr_ =
        state->mempartition()->start_addr("sacl_ingress_v4");
    SDK_ASSERT(ing_v4_region_addr_ != INVALID_MEM_ADDRESS);
    ing_v4_table_size_ =
        state->mempartition()->block_size("sacl_ingress_v4");
    egr_v6_idxr_ =
        indexer::factory(PDS_MAX_SECURITY_POLICY + 1);
    SDK_ASSERT(egr_v6_idxr_ != NULL);
    egr_v6_region_addr_ =
        state->mempartition()->start_addr("sacl_egress_v6");
    SDK_ASSERT(egr_v6_region_addr_ != INVALID_MEM_ADDRESS);
    egr_v6_table_size_ =
        state->mempartition()->block_size("sacl_egress_v6");
    ing_v6_idxr_ =
        indexer::factory(PDS_MAX_SECURITY_POLICY + 1);
    SDK_ASSERT(ing_v6_idxr_ != NULL);
    ing_v6_region_addr_ =
        state->mempartition()->start_addr("sacl_ingress_v6");
    SDK_ASSERT(ing_v6_region_addr_ != INVALID_MEM_ADDRESS);
    ing_v6_table_size_ =
        state->mempartition()->block_size("sacl_ingress_v6");
}

security_policy_impl_state::~security_policy_impl_state() {
    indexer::destroy(egr_v4_idxr_);
    indexer::destroy(ing_v4_idxr_);
    indexer::destroy(egr_v6_idxr_);
    indexer::destroy(ing_v6_idxr_);
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
