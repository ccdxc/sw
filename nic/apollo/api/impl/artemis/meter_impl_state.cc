//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// meter datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/impl/artemis/meter_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_METER_IMPL_STATE - meter database functionality
/// \ingroup PDS_METER
/// \@{

meter_impl_state::meter_impl_state(pds_state *state) {
    /**
     * we need max + 1 blocks, extra 1 block for processing updates for
     * meter tables (with the assumption that more than one meter table
     * is not updated in any given batch
     */
    v4_lpm_idxr_ = indexer::factory(PDS_MAX_METER + 1);
    SDK_ASSERT(v4_lpm_idxr_ != NULL);
    v6_lpm_idxr_ = indexer::factory(PDS_MAX_METER + 1);
    SDK_ASSERT(v6_lpm_idxr_ != NULL);
    v4_lpm_region_addr_ = state->mempartition()->start_addr("meter_v4");
    SDK_ASSERT(v4_lpm_region_addr_ != INVALID_MEM_ADDRESS);
    v4_lpm_table_size_ = state->mempartition()->block_size("meter_v4");
    v4_lpm_max_prefixes_ = state->mempartition()->max_elements("meter_v4") - 1;
    v6_lpm_region_addr_ = state->mempartition()->start_addr("meter_v6");
    SDK_ASSERT(v6_lpm_region_addr_ != INVALID_MEM_ADDRESS);
    v6_lpm_table_size_ = state->mempartition()->block_size("meter_v6");
    v6_lpm_max_prefixes_ = state->mempartition()->max_elements("meter_v6") - 1;
    policer_idxr_ = indexer::factory(PDS_MAX_METER_POLICER);
}

meter_impl_state::~meter_impl_state() {
    indexer::destroy(v4_lpm_idxr_);
    indexer::destroy(v6_lpm_idxr_);
    indexer::destroy(policer_idxr_);
}

sdk_ret_t
meter_impl_state::table_transaction_begin(void) {
    //v4_idxr_->txn_start();
    //v6_idxr_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
meter_impl_state::table_transaction_end(void) {
    //v4_idxr_->txn_end();
    //v6_idxr_->txn_end();
    return SDK_RET_OK;
}

/** @} */    // end of PDS_METER_IMPL_STATE

}    // namespace impl
}    // namespace api
