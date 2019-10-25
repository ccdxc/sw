//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop implementation state
///
//----------------------------------------------------------------------------

#ifndef __NEXTHOP_IMPL_STATE_HPP__
#define __NEXTHOP_IMPL_STATEHPP__

#include "nic/sdk/lib/table/sldirectmap/sldirectmap.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_IMPL_STATE    nexthop state functionality
/// \ingroup PDS_NEXTHOP
/// @{

// forward declaration
class nexthop_impl;

/// \brief  state maintained for nexthops
class nexthop_impl_state : public state_base {
public:
    /// \brief  constructor
    nexthop_impl_state(pds_state *state);

    /// \brief  destructor
    ~nexthop_impl_state();

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief  return the nexthop table instance
    /// \return pointer to the nexthop table instance
    sldirectmap *nh_tbl(void) { return nh_tbl_; }

private:
    friend class nexthop_impl;   // nexthop_impl is friend of nexthop_impl_state

private:
    sldirectmap *nh_tbl_;    // sldirectmap table for nexthop
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __NEXTHOP_IMPL_STATE_HPP__
