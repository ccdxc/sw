//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Common Utilities used by all PDSA stub components
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include <ifaddrs.h>
#include <netpacket/packet.h>

namespace pdsa_stub {

// Utility function to get MAC address for interface from Linux
bool 
get_interface_mac_address (const std::string& if_name, mac_addr_t& if_mac)
{
    struct ifaddrs *ifaddr = NULL;
    struct ifaddrs *ifa = NULL;
    bool ret = false;

    if (getifaddrs(&ifaddr) == -1) {
        SDK_TRACE_ERR ("Failed to get MAC address for %s", if_name);
        return false;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        SDK_TRACE_VERBOSE ("Looping Linux interfaces - current %s", ifa->ifa_name);

        if (if_name == ifa->ifa_name) {
            SDK_TRACE_VERBOSE ("Found %s - get MAC", ifa->ifa_name);
            SDK_ASSERT(ifa->ifa_addr);
            SDK_ASSERT(ifa->ifa_addr->sa_family == AF_PACKET);

            struct sockaddr_ll *sock_addr = (struct sockaddr_ll*)ifa->ifa_addr;
            SDK_ASSERT(sock_addr->sll_halen == ETH_ADDR_LEN);
            memcpy (if_mac, sock_addr->sll_addr, ETH_ADDR_LEN);

            ret = true;
            goto exit;
        }
    }
exit:
    freeifaddrs(ifaddr);
    return ret;
}

} // End namespace
