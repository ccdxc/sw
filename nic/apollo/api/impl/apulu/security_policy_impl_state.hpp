//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// security policy implementation state
///
//----------------------------------------------------------------------------

#ifndef __SECURITY_POLICY_STATE_IMPL_HPP__
#define __SECURITY_POLICY_IMPL_STATE_HPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/table/hash/hash.hpp"
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

    /// \brief  allocate memory required for a security policy impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    security_policy_impl *alloc(void);

    /// \brief     free security policy impl instance back
    /// \param[in] impl pointer to the allocated impl instance
    void free(security_policy_impl *impl);

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief  return policy region's base/start address in memory
    mem_addr_t security_policy_region_addr(uint8_t af, rule_dir_t dir) const {
        if (af == IP_AF_IPV4) {
            if (dir == RULE_DIR_INGRESS) {
                return ing_v4_region_addr_;
            } else {
                return egr_v4_region_addr_;
            }
        } else {
            if (dir == RULE_DIR_INGRESS) {
                return ing_v6_region_addr_;
            } else {
                return egr_v6_region_addr_;
            }
        }
    }

    /// \brief return security policy table's size
    uint64_t security_policy_table_size(uint8_t af, rule_dir_t dir) const {
        if (af == IP_AF_IPV4) {
            if (dir == RULE_DIR_INGRESS) {
                return ing_v4_table_size_;
            } else {
                return egr_v4_table_size_;
            }
        } else {
            if (dir == RULE_DIR_INGRESS) {
                return ing_v6_table_size_;
            } else {
                return egr_v6_table_size_;
            }
        }
    }

private:
    indexer *security_policy_idxr(uint8_t af, rule_dir_t dir) {
        if (af == IP_AF_IPV4) {
            if (dir == RULE_DIR_INGRESS) {
                return ing_v4_idxr_;
            } else {
                return egr_v4_idxr_;
            }
        } else {
            if (dir == RULE_DIR_INGRESS) {
                return ing_v6_idxr_;
            } else {
                return egr_v6_idxr_;
            }
        }
    }
    friend class security_policy_impl;

private:
    // datapath tables for security policy
    indexer    *ing_v4_idxr_;          ///< indexer to allocate mem block for ingress policy tables
    mem_addr_t ing_v4_region_addr_;    ///< base address for the ingress policy region
    uint64_t   ing_v4_table_size_;     ///< size of each ingress policy table
    indexer    *egr_v4_idxr_;          ///< indexer to allocate mem block for egress policy tables
    mem_addr_t egr_v4_region_addr_;    ///< base address for the egress policy region
    uint64_t   egr_v4_table_size_;     ///< size of each egress policy table
    indexer    *ing_v6_idxr_;          ///< indexer to allocate mem block for ingress policy tables
    mem_addr_t ing_v6_region_addr_;    ///< base address for the ingress policy region
    uint64_t   ing_v6_table_size_;     ///< size of each ingress policy table
    indexer    *egr_v6_idxr_;          ///< indexer to allocate mem block for egress policy tables
    mem_addr_t egr_v6_region_addr_;    ///< base address for the egress policy region
    uint64_t   egr_v6_table_size_;     ///< size of each egress policy table
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __SECURITY_POLICY_IMPL_STATE_HPP__
