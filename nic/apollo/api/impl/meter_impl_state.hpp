/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    meter_impl_state.hpp
 *
 * @brief   meter implementation state
 */

#ifndef __METER_IMPL_STATE_HPP__
#define __METER_IMPL_STATEHPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

// reserved meter id
#define PDS_RESERVED_METER_HW_ID    0
#define PDS_MAX_METER_POLICER       8192

namespace api {
namespace impl {

/// \defgroup PDS_METER_IMPL_STATE - meter impl state functionality
/// \ingroup PDS_METER
/// @{

///< forward declaration
class meter_impl;

/// \brief    state maintained for meters
class meter_impl_state : public state_base {
public:
    /// \brief    constructor
    meter_impl_state(pds_state *state);

    /// \@brief    destructor
    ~meter_impl_state();

    /// \brief    API to initiate transaction over all the table manamgement
    ///           library instances
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief     API to end transaction over all the table manamgement
    ///            library instances
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief    return lpm region's base/start address in memory based on IP
    ///           address family
    /// \param[in] af    IP address family
    /// \return    IPv4/IPv6 meter lpm region's base address
    mem_addr_t region_addr(uint8_t af) const {
        if (af == IP_AF_IPV4) {
            return v4_lpm_region_addr_;
        }
        return v6_lpm_region_addr_;
    }

    /// \brief    return lpm table's size based on IP address family
    /// \param[in] af    IP address family
    /// \return    size of single IPv4/IPv6 meter lpm table's size
    uint32_t table_size(uint8_t af) const {
        if (af == IP_AF_IPV4) {
            return v4_lpm_table_size_;
        }
        return v6_lpm_table_size_;
    }

    /// \brief    return max. number of prefixes per meter based on IP address
    ///           family
    /// \param[in] af    IP address family
    /// \return    max. number of IPv4/IPv6 prefixes per meter
    uint32_t max_prefixes(uint8_t af) const {
        if (af == IP_AF_IPV4) {
            return v4_lpm_max_prefixes_;
        }
        return v6_lpm_max_prefixes_;
    }

private:
    indexer *lpm_idxr(uint8_t af) {
        if (af == IP_AF_IPV4) {
            return v4_lpm_idxr_;
        }
        return v6_lpm_idxr_;
    }
    indexer *policer_idxr(void) {
        return policer_idxr_;
    }
    friend class meter_impl;

private:
    ///< datapath tables for meter
    indexer       *v4_lpm_idxr_;           ///< indexer to allocate v4 lpm mem block
    indexer       *v6_lpm_idxr_;           ///< indexer to allocate v6 lpm mem block
    mem_addr_t    v4_lpm_region_addr_;     ///< base address for the v4 lpm region
    uint32_t      v4_lpm_table_size_;      ///< size of each v4 lpm table
    uint32_t      v4_lpm_max_prefixes_;    ///< max IPv4 routes per meter
    mem_addr_t    v6_lpm_region_addr_;     ///< base address for the v6 lpm region
    uint32_t      v6_lpm_table_size_;      ///< size of each v6 lpm table
    uint32_t      v6_lpm_max_prefixes_;    ///< max IPv6 routes per meter
    indexer       *policer_idxr_;          ///< indexer to allocate policer
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    // __METER_IMPL_STATE_HPP__
