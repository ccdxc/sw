//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// DHCP implementation state
///
//----------------------------------------------------------------------------

#ifndef __DHCP_IMPL_STATE_HPP__
#define __DHCP_IMPL_STATEHPP__

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_DHCP_IMPL_STATE    DHCP state functionality
/// \ingroup PDS_DHCP
/// @{

// forward declaration
class dhcp_policy_impl;

/// \brief  state maintained for DHCP objects
class dhcp_impl_state : public state_base {
public:
    /// \brief  constructor
    dhcp_impl_state(pds_state *state) {}

    /// \brief  destructor
    ~dhcp_impl_state() {}

    /// \brief  allocate memory required for a DHCP policy impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    dhcp_policy_impl *alloc(void);

    /// \brief     free DHCP policy impl instance back
    /// \param[in] impl pointer to the allocated impl instance
    void free(dhcp_policy_impl *impl);

private:
    /// dhcp_policy_impl is friend of dhcp_impl_state
    friend class dhcp_policy_impl;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __DHCP_IMPL_STATE_HPP__
