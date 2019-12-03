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

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
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

    /// \brief  allocate memory required for a nexthop group impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    nexthop_group_impl *alloc(void);

    /// \brief     free nexthop group impl instance back
    /// \param[in] impl pointer to the allocated impl instance
    void free(nexthop_group_impl *impl);

    /// \brief  return the nexthop group table indexer
    /// \return pointer to the nexthop group table indexer
    rte_indexer *nhgroup_idxr(void) { return nhgroup_idxr_; }

private:
    // nexthop_group_impl is friend of nexthop_group_impl_state
    friend class nexthop_group_impl;

private:
    // indexer for nexthop group
    rte_indexer *nhgroup_idxr_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __NEXTHOP_GROUP_IMPL_STATE_HPP__
