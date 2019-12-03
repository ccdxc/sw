//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// route table implementation state
///
//----------------------------------------------------------------------------

#ifndef __ROUTE_IMPL_STATE_HPP__
#define __ROUTE_IMPL_STATEHPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/table/hash/hash.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_ROUTE_TABLE_IMPL_STATE - route table impl state functionality
/// \ingroup PDS_ROUTE
/// @{

/// \brief forward declaration
class route_table_impl;

/// \brief state maintained for route tables
class route_table_impl_state : public state_base {
public:
    /// \brief constructor
    route_table_impl_state(pds_state *state);

    /// \brief destructor
    ~route_table_impl_state();

    /// \brief  allocate memory required for a route table impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    route_table_impl *alloc(void);

    /// \brief     free route table impl instance back
    /// \param[in] impl pointer to the allocated impl instance
    void free(route_table_impl *impl);

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief return v4 lpm region's base/start address in memory
    mem_addr_t v4_region_addr(void) const { return v4_region_addr_; }

    /// \brief return per v4 lpm table's size
    uint64_t v4_table_size(void) const { return v4_table_size_; }

    /// \brief return number of v4 routes per table
    uint32_t v4_max_routes(void) const { return v4_max_routes_; }

    /// \brief return v6 lpm region's base/start address in memory
    mem_addr_t v6_region_addr(void) const { return v6_region_addr_; }

    /// \brief return per v6 lpm table's size
    uint64_t v6_table_size(void) const { return v6_table_size_; }

    /// \brief return number of v6 routes per table
    uint32_t v6_max_routes(void) const { return v6_max_routes_; }

private:
    indexer *v4_idxr(void) { return v4_idxr_; }
    indexer *v6_idxr(void) { return v6_idxr_; }
    friend class route_table_impl;

private:
    // datapath tables for route table
    indexer    *v4_idxr_;          ///< indexer to allocate v4 lpm mem block
    indexer    *v6_idxr_;          ///< indexer to allocate v6 lpm mem block
    mem_addr_t v4_region_addr_;    ///< base address for the v4 lpm region
    uint64_t   v4_table_size_;     ///< size of each v4 lpm table
    uint32_t   v4_max_routes_;     ///< max IPv4 routes per route table
    mem_addr_t v6_region_addr_;    ///< base address for the v6 lpm region
    uint64_t   v6_table_size_;     ///< size of each v6 lpm table
    uint32_t   v6_max_routes_;     ///< max IPv6 routes per route table
};

/// \}

}    // namespace impl
}    // namespace api

#endif    // __ROUTE_IMPL_STATE_HPP__
