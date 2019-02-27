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
    route_table_idxr_ = indexer::factory(PDS_MAX_ROUTE_PER_TABLE + 1);
    SDK_ASSERT(route_table_idxr_ != NULL);
    lpm_region_addr_ = state->mempartition()->start_addr("lpm_v4");
    lpm_table_size_ = state->mempartition()->element_size("lpm_v4");
}

/**
 * @brief    destructor
 */
route_table_impl_state::~route_table_impl_state() {
    indexer::destroy(route_table_idxr_);
}

/** @} */    // end of PDS_ROUTE_TABLE_IMPL_STATE

}    // namespace impl
}    // namespace api
