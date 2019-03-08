/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    route_impl_state.cc
 *
 * @brief   This file contains route table datapath database handling
 */

#include "nic/apollo/include/api/pds_route.hpp"
#include "nic/apollo/api/impl/route_impl_state.hpp"

namespace api {
namespace impl {

/**
 * @defgroup PDS_ROUTE_TABLE_IMPL_STATE - route table database functionality
 * @ingroup PDS_ROUTE
 * @{
 */

/**
 * @brief    constructor
 */
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
    v4_region_addr_ = state->mempartition()->start_addr("lpm_v4");
    v4_table_size_ = state->mempartition()->element_size("lpm_v4");
    v6_region_addr_ = state->mempartition()->start_addr("lpm_v6");
    v6_table_size_ = state->mempartition()->element_size("lpm_v6");
}

/**
 * @brief    destructor
 */
route_table_impl_state::~route_table_impl_state() {
    indexer::destroy(v4_idxr_);
    indexer::destroy(v6_idxr_);
}

/** @} */    // end of PDS_ROUTE_TABLE_IMPL_STATE

}    // namespace impl
}    // namespace api
