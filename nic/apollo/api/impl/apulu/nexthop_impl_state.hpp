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

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
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

    /// \brief  allocate memory required for a nexthop impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    nexthop_impl *alloc(void);

    /// \brief     free nexthop impl instance back
    /// \param[in] impl pointer to the allocated impl instance
    void free(nexthop_impl *impl);

    /// \brief  return the nexthop table indexer
    /// \return pointer to the nexthop table indexer
    rte_indexer *nh_idxr(void) { return nh_idxr_; }

private:
    friend class nexthop_impl;   // nexthop_impl is friend of nexthop_impl_state

private:
    // indexer for nexthops
    rte_indexer *nh_idxr_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __NEXTHOP_IMPL_STATE_HPP__
