/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    route_impl_state.cc
 *
 * @brief   This file contains route table datapath database handling
 */

#include "nic/hal/apollo/include/api/oci_route.hpp"
#include "nic/hal/apollo/api/impl/route_impl_state.hpp"

namespace impl {

/**
 * @defgroup OCI_ROUTE_TABLE_IMPL_STATE - route table database functionality
 * @ingroup OCI_ROUTE
 * @{
 */

/**
 * @brief    constructor
 */
route_table_impl_state::route_table_impl_state() {
    route_table_idxr_ = indexer::factory(OCI_MAX_ROUTE_PER_TABLE);
    SDK_ASSERT(route_table_idxr_ != NULL);
}

/**
 * @brief    destructor
 */
route_table_impl_state::~route_table_impl_state() {
    indexer::destroy(route_table_idxr_);
}

/** @} */    // end of OCI_ROUTE_TABLE_IMPL_STATE

}    // namespace impl
