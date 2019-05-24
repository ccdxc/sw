//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// route table datapath databae handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/impl/artemis/route_impl_state.hpp"

namespace api {
namespace impl {

// 1K routes, 3 stages, 16-way tree => 64 + 16 * 64 + 16 * 16 * 64 = 17472 bytes
#define PDS_ROUTE_LPM_IPV4_BLOCK_SIZE     17472
// region size = (64 + 1) * PDS_ROUTE_LPM_IPV4_BLOCK_SIZE
#define PDS_ROUTE_LPM_IPV4_REGION_SIZE   (1 << 21)
// 1K routes, 4 stages, 8-way tree =>
// 64 + 8 * 64 + 8 * 8 * 64 + 8 * 8 * 8 * 64 = 37440 bytes
#define PDS_ROUTE_LPM_IPV6_BLOCK_SIZE    37440
// region size = (64 + 1) * PDS_ROUTE_LPM_IPV6_BLOCK_SIZE
#define PDS_ROUTE_LPM_IPV6_REGION_SIZE    (3 * (1 << 20))

route_table_impl_state::route_table_impl_state(pds_state *state) {
    /**
     * we need max + 1 blocks, extra 1 block for processing updates for
     * routing table (with the assumption that more than one routing table
     * is not updated in any given batch
     */
    v4_idxr_ = indexer::factory(PDS_MAX_ROUTE_TABLE + 1);
    SDK_ASSERT(v4_idxr_ != NULL);
    v6_idxr_ = indexer::factory(PDS_MAX_ROUTE_TABLE + 1);
    SDK_ASSERT(v6_idxr_ != NULL);
    v4_region_addr_ = state->mempartition()->start_addr("lpm");
    SDK_ASSERT(v4_region_addr_ != INVALID_MEM_ADDRESS);
    v4_table_size_ = PDS_ROUTE_LPM_IPV4_BLOCK_SIZE;
    v4_max_routes_ = state->mempartition()->max_elements("lpm") - 1;
    v6_region_addr_ = v4_region_addr_ + PDS_ROUTE_LPM_IPV4_REGION_SIZE;
    v6_table_size_ = PDS_ROUTE_LPM_IPV6_BLOCK_SIZE;
    v6_max_routes_ = v4_max_routes_;
}

route_table_impl_state::~route_table_impl_state() {
    indexer::destroy(v4_idxr_);
    indexer::destroy(v6_idxr_);
}

sdk_ret_t
route_table_impl_state::table_transaction_begin(void) {
    //v4_idxr_->txn_start();
    //v6_idxr_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
route_table_impl_state::table_transaction_end(void) {
    //v4_idxr_->txn_end();
    //v6_idxr_->txn_end();
    return SDK_RET_OK;
}

}    // namespace impl
}    // namespace api
