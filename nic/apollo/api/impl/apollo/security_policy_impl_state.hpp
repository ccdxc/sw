//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// security policy implementation state
///
//----------------------------------------------------------------------------

#ifndef __SECURITY_POLICY_STATE_IMPL_HPP__
#define __SECURITY_POLICY_IMPL_STATE_HPP__

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_SECURITY_POLICY_IMPL_STATE - security policy impl state
///           functionality
/// \ingroup PDS_SECURITY_POLICY
/// @{

// forward declaration
class security_policy_impl;

/// \brief state maintained for security policies
class security_policy_impl_state : public state_base {
public:
    /// \brief constructor
    security_policy_impl_state(pds_state *state);

    /// \brief destructor
    ~security_policy_impl_state();

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief  return policy region's base/start address in memory
    mem_addr_t security_policy_region_addr(uint8_t af) const {
        return (af == IP_AF_IPV4) ? v4_region_addr_ : v6_region_addr_;
    }

    /// \brief return security policy table's size
    uint64_t security_policy_table_size(uint8_t af) const {
        return (af == IP_AF_IPV4) ? v4_table_size_ : v6_table_size_;
    }

private:
    rte_indexer *security_policy_idxr(uint8_t af) {
        return (af == IP_AF_IPV4) ? v4_idxr_ : v6_idxr_;
    }
    friend class security_policy_impl;

private:
    // datapath tables for security policy
    rte_indexer *v4_idxr_;          ///< indexer to allocate mem block for v4 policy tables
    mem_addr_t  v4_region_addr_;    ///< base address for v4 policy region
    uint64_t    v4_table_size_;     ///< size of each v4 policy table

    rte_indexer *v6_idxr_;          ///< indexer to allocate mem block for v6 policy tables
    mem_addr_t  v6_region_addr_;    ///< base address for v6 policy region
    uint64_t    v6_table_size_;     ///< size of each v6 policy table
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __SECURITY_POLICY_IMPL_STATE_HPP__
