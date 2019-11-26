//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Utilities to interface with Linux OS
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pdsa_linux_util.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include <net/if.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <cstring>

namespace pdsa_stub {

// Utility function to get MAC address for interface from Linux
static bool 
get_linux_intf_mac_addr (const std::string& if_name, mac_addr_t& if_mac)
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

bool
get_linux_intf_params (const char* ifname,
                       uint32_t*   lnx_ifindex,
                       mac_addr_t& mac)
{
    *lnx_ifindex = if_nametoindex(ifname);
    if (*lnx_ifindex == 0) return false;
    return get_linux_intf_mac_addr(ifname, mac);
}

} // End namespace
