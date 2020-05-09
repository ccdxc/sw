//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
///
/// \file
/// apulu pipeline protobuf common utility methods
///
//------------------------------------------------------------------------------

#ifndef __SPECS_IMPL_HPP__
#define __SPECS_IMPL_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "gen/proto/types.pb.h"

// build obj key from protobuf spec
static inline sdk_ret_t
pds_obj_key_proto_to_api_spec (pds_obj_key_t *api_spec,
                               const ::std::string& proto_key)
{
    if (proto_key.length() > PDS_MAX_KEY_LEN) {
        return SDK_RET_INVALID_ARG;
    }
    // set all the key bytes to 0 1st and hash on the full key can't include
    // uninitialized memory
    api_spec->reset();
    // set the key bytes
    memcpy(api_spec->id, proto_key.data(),
           MIN(proto_key.length(), PDS_MAX_KEY_LEN));
    return SDK_RET_OK;
}

//----------------------------------------------------------------------------
// convert ip_addr_t to IP address proto spec
//----------------------------------------------------------------------------
static inline sdk_ret_t
ipaddr_api_spec_to_proto_spec (types::IPAddress *out_ipaddr,
                               const ip_addr_t *in_ipaddr)
{
    if (in_ipaddr->af == IP_AF_IPV4) {
        out_ipaddr->set_af(types::IP_AF_INET);
        out_ipaddr->set_v4addr(in_ipaddr->addr.v4_addr);
    } else if (in_ipaddr->af == IP_AF_IPV6) {
        out_ipaddr->set_af(types::IP_AF_INET6);
        out_ipaddr->set_v6addr(
                    std::string((const char *)&in_ipaddr->addr.v6_addr.addr8,
                                IP6_ADDR8_LEN));
    } else {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

//----------------------------------------------------------------------------
// convert IP address spec in proto to ip_addr
//----------------------------------------------------------------------------
static inline sdk_ret_t
ipaddr_proto_spec_to_api_spec (ip_addr_t *out_ipaddr,
                               const types::IPAddress &in_ipaddr)
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
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

#endif    // __SPECS_IMPL_HPP__
