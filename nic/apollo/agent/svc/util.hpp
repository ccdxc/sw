//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __AGENT_SVC_UTIL_HPP__
#define __AGENT_SVC_UTIL_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "gen/proto/types/types.pb.h"

//----------------------------------------------------------------------------
// convert IP address spec in proto to ip_addr
//----------------------------------------------------------------------------
static inline void
pds_agent_util_ipaddr_fill (const types::IPAddress &in_ipaddr, ip_addr_t *out_ipaddr)
{
    memset(out_ipaddr, 0, sizeof(ip_addr_t));
    if (in_ipaddr.af() == types::IP_AF_INET) {
        out_ipaddr->af = IP_AF_IPV4;
        out_ipaddr->addr.v4_addr = in_ipaddr.v4addr();
    } else if (in_ipaddr.af() == types::IP_AF_INET6) {
        out_ipaddr->af = IP_AF_IPV6;
        memcpy(out_ipaddr->addr.v6_addr.addr8,
               in_ipaddr.v6addr().c_str(),
               IP6_ADDR8_LEN);
    } else {
        return;
    }

    return;
}

static inline sdk_ret_t
pds_agent_util_ip_pfx_fill (const types::IPPrefix& in_ippfx, ip_prefix_t *ip_pfx)
{
    ip_pfx->len = in_ippfx.len();
    if (((in_ippfx.addr().af() == types::IP_AF_INET) &&
             (ip_pfx->len > 32)) ||
        ((in_ippfx.addr().af() == types::IP_AF_INET6) &&
             (ip_pfx->len > 128))) {
        return SDK_RET_INVALID_ARG;
    } else {
        pds_agent_util_ipaddr_fill(in_ippfx.addr(), &ip_pfx->addr);
    }
    return SDK_RET_OK;
}

static inline pds_encap_t
proto_encap_to_pds_encap (types::Encap encap)
{
    pds_encap_t    pds_encap;

    memset(&pds_encap, 0, sizeof(pds_encap));
    switch (encap.type()) {
    case types::ENCAP_TYPE_NONE:
        pds_encap.type = PDS_ENCAP_TYPE_NONE;
        break;

    case types::ENCAP_TYPE_DOT1Q:
        pds_encap.type = PDS_ENCAP_TYPE_DOT1Q;
        pds_encap.val.vlan_tag = encap.value().vlanid();
        break;

    case types::ENCAP_TYPE_QINQ:
        pds_encap.type = PDS_ENCAP_TYPE_QINQ;
        break;

    case types::ENCAP_TYPE_MPLSoUDP:
        pds_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
        pds_encap.val.mpls_tag = encap.value().mplstag();
        break;

    case types::ENCAP_TYPE_VXLAN:
        pds_encap.type = PDS_ENCAP_TYPE_VXLAN;
        pds_encap.val.vnid = encap.value().vnid();
        break;
    default:
        break;
    }
    return pds_encap;
}

#endif    // __AGENT_SVC_UTIL_HPP__
