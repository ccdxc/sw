/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    route_impl_state.hpp
 *
 * @brief   route table implementation state
 */
#if !defined (__ROUTE_IMPL_STATE_HPP__)
#define __ROUTE_IMPL_STATEHPP__

#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/table/hash/hash.hpp"

namespace impl {

/**
 * @defgroup OCI_ROUTE_TABLE_IMPL_STATE - route table impl state functionality
 * @ingroup OCI_ROUTE
 * @{
 */

 /**< forward declaration */
class route_table_impl;

/**
 * @brief    state maintained for route tables
 */
class route_table_impl_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    route_table_impl_state();

    /**
     * @brief    destructor
     */
    ~route_table_impl_state();

private:
    indexer *route_table_idxr(void) { return route_table_idxr_; }
    friend class route_table_impl;

private:
    /**< datapath tables for route table */
    indexer      *route_table_idxr_;    /**< indexer to allocate lpm mem block */
};

/** * @} */    // end of OCI_ROUTE_TABLE_IMPL_STATE

}    // namespace impl

#endif    /** __ROUTE_IMPL_STATE_HPP__ */
