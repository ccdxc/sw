/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    route_impl_state.hpp
 *
 * @brief   route table implementation state
 */
#if !defined (__ROUTE_IMPL_STATE_HPP__)
#define __ROUTE_IMPL_STATEHPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/table/hash/hash.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/core/oci_state.hpp"

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
    /**< @brief    constructor */
    route_table_impl_state(oci_state *state);

    /**< @brief    destructor */
    ~route_table_impl_state();

    /**< @brief    return LPM region's base/start address in memory */
    mem_addr_t lpm_region_addr(void) const { return lpm_region_addr_; }

    /**< @brief    return per LPM table's size */
    mem_addr_t lpm_table_size(void) const { return lpm_table_size_; }

private:
    indexer *route_table_idxr(void) { return route_table_idxr_; }
    friend class route_table_impl;

private:
    /**< datapath tables for route table */
    indexer       *route_table_idxr_;    /**< indexer to allocate lpm mem block */
    mem_addr_t    lpm_region_addr_;      /**< base address for the LPM region */
    uint32_t      lpm_table_size_;       /**< size of each LPM table */
};

/** * @} */    // end of OCI_ROUTE_TABLE_IMPL_STATE

}    // namespace impl

#endif    /** __ROUTE_IMPL_STATE_HPP__ */
