//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __AGENT_SVC_UTIL_HPP__
#define __AGENT_SVC_UTIL_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "gen/proto/types/types.pb.h"

static inline void
pds_agent_util_ipaddr_fill (types::IPAddress proto_addr, ip_addr_t *addr)
{
    if (proto_addr.af() == types::IP_AF_INET) {
        addr->af = IP_AF_IPV4;
        addr->addr.v4_addr = proto_addr.v4addr();
    } else if (proto_addr.af() == types::IP_AF_INET6) {
        addr->af = IP_AF_IPV6;
        // proto_addr.v6aaddr()
        // TODO
    }
}

static inline void
pds_agent_util_ip_pfx_fill (types::IPPrefix proto_pfx, ip_prefix_t *pfx)
{
    types::IPAddress addr;

    addr = proto_pfx.addr();
    pfx->len = proto_pfx.len();
    pfx->addr.af = addr.af();
    if (addr.af() == IP_AF_IPV4) {
        pds_agent_util_ipaddr_fill(addr, &pfx->addr);
    } else {
        // TODO
        // std::string v6addr = addr.v6aadr();
    }
}

#endif    // __AGENT_SVC_UTIL_HPP__
