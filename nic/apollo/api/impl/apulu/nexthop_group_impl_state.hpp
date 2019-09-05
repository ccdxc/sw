//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop group implementation state
///
//----------------------------------------------------------------------------

#ifndef __NEXTHOP_GROUP_IMPL_STATE_HPP__
#define __NEXTHOP_GROUP_IMPL_STATEHPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_GROUP_IMPL_STATE    nexthop group functionality
/// \ingroup PDS_NEXTHOP
/// @{

// forward declaration
class nexthop_group_impl;

/// \brief  state maintained for nexthop groups
class nexthop_group_impl_state : public state_base {
public:
    /// \brief  constructor
    nexthop_group_impl_state(pds_state *state);

    /// \brief  destructor
    ~nexthop_group_impl_state();

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief  return the underlay nexthop group table instance
    /// \return pointer to the underlay nexthop group table instance
    directmap *underlay_nh_group_tbl(void) { return underlay_nh_group_tbl_; }

    /// \brief  return the overlay nexthop group table instance
    /// \return pointer to the overlay nexthop group table instance
    directmap *overlay_nh_group_tbl(void) { return overlay_nh_group_tbl_; }

private:
    // nexthop_group_impl is friend of nexthop_group_impl_state
    friend class nexthop_group_impl;

private:
    // directmap table for underlay ECMP
    directmap *underlay_nh_group_tbl_;
    // directmap table for overlay ECMP
    directmap *overlay_nh_group_tbl_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __NEXTHOP_GROUP_IMPL_STATE_HPP__
