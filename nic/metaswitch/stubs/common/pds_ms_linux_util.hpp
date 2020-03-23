//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Utilities to interface with Linux OS
//---------------------------------------------------------------

#ifndef __PDS_MS_LINUX_UTIL_HPP__
#define __PDS_MS_LINUX_UTIL_HPP__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include <string>

namespace pds_ms {

struct in_ipx_addr_t {
    uint8_t af; // AF_INET or AF_INET6
    union {
        in_addr   v4;
        in6_addr  v6;
    } addr;
};

// Fetch Linux parameters (IfIndex, MAC) for given interface
bool get_linux_intf_params(const char* ifname,      // In
                           uint32_t*   lnx_ifindex, // Out
                           mac_addr_t& mac);        // Out

void config_linux_intf_ip (uint32_t lnx_ifindex, const in_ipx_addr_t& ip,
                                uint32_t prefix_len, bool del = false);
void config_linux_loopback_ip (const in_ipx_addr_t& ip, uint32_t prefix_len, 
                               bool del = false);

} // End namespace
#endif
