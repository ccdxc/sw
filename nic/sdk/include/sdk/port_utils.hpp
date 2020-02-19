//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// util methods for port
///
///----------------------------------------------------------------------------

#ifndef __SDK_PORT_UTILS_HPP__
#define __SDK_PORT_UTILS_HPP__

#include "include/sdk/if.hpp"

#define PORT_MAC_STAT_REPORT_SIZE 1024

// @brief     get offset for port stats in hbm given ifindex of port
// @param[in] ifindex     ifindex of the port
// @return    offset of the port stats region
static inline sdk::types::mem_addr_t
port_stats_addr_offset (uint32_t ifindex)
{
    uint32_t parent_port;

    // Base: CAPRI_HBM_REG_PORT_STATS ("port_stats")
    // 1K MAC stats size per port
    // First 1K: Port 1
    // Next 1K: Port 2
    // Next 1K: Port 3
    parent_port = ETH_IFINDEX_TO_PARENT_PORT(ifindex);
    switch (parent_port) {
    case 1:
        // mx0_dhs_mac_stats_entry
        return 0;
    case 2:
        // mx1_dhs_mac_stats_entry
        return PORT_MAC_STAT_REPORT_SIZE;
    case 3:
        // bx_dhs_mac_stats_entry
        return PORT_MAC_STAT_REPORT_SIZE * 2;
    default:
        SDK_ASSERT(0);
    }
    return 0;
}

#endif    // __SDK_PORT_UTILS_HPP__
