//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// tag implementation state
///
//----------------------------------------------------------------------------


#ifndef __TAG_IMPL_STATE_HPP__
#define __TAG_IMPL_STATEHPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_TAG_IMPL_STATE - tag impl state functionality
/// \ingroup PDS_TAG
/// @{

// forward declaration
class tag_impl;

/// \brief  state maintained for tags
class tag_impl_state : public state_base {
public:
    /// \brief  constructor
    tag_impl_state(pds_state *state);

    /// \brief destructor
    ~tag_impl_state();

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief      return lpm region's base/start address in memory based on IP
    ///             address family
    /// \param[in]  af IP address family
    /// \return     IPv4/IPv6 tag lpm region's base address
    mem_addr_t region_addr(uint8_t af) const {
        if (af == IP_AF_IPV4) {
            return v4_lpm_region_addr_;
        }
        return v6_lpm_region_addr_;
    }

    /// \brief      return lpm table's size based on IP address family
    /// \param[in]  af IP address family
    /// \return     size of single IPv4/IPv6 tag lpm table's size
    uint32_t table_size(uint8_t af) const {
        if (af == IP_AF_IPV4) {
            return v4_lpm_table_size_;
        }
        return v6_lpm_table_size_;
    }

    /// \brief      return max. number of prefixes per tag LPM based on
    ///             IP address family
    /// \param[in]  af IP address family
    /// \return     max. number of IPv4/IPv6 prefixes per tag tree
    uint32_t max_prefixes(uint8_t af) const {
        if (af == IP_AF_IPV4) {
            return v4_lpm_max_prefixes_;
        }
        return v6_lpm_max_prefixes_;
    }

private:
    friend class tag_impl;

private:
    mem_addr_t    v4_lpm_region_addr_;     ///< base address for the v4 lpm reg
    uint32_t      v4_lpm_table_size_;      ///< size of each v4 lpm table
    uint32_t      v4_lpm_max_prefixes_;    ///< max IPv4 routes per tag tree
    mem_addr_t    v6_lpm_region_addr_;     ///< base address for the v6 lpm reg
    uint32_t      v6_lpm_table_size_;      ///< size of each v6 lpm table
    uint32_t      v6_lpm_max_prefixes_;    ///< max IPv6 routes per tag tree
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __TAG_IMPL_STATE_HPP__
