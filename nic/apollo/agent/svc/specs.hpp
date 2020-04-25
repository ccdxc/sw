//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __AGENT_SVC_SPECS_HPP__
#define __AGENT_SVC_SPECS_HPP__

#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "gen/proto/types.pb.h"
#include "gen/proto/debug.grpc.pb.h"

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
        //SDK_ASSERT(FALSE);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
ippfx_proto_spec_to_api_spec (ip_prefix_t *ip_pfx,
                              const types::IPPrefix& in_ippfx)
{
    ip_pfx->len = in_ippfx.len();
    if (in_ippfx.addr().af() == types::IP_AF_INET) {
        if (ip_pfx->len > 32) {
            return SDK_RET_INVALID_ARG;
        }
    } else if (in_ippfx.addr().af() == types::IP_AF_INET6) {
        if (ip_pfx->len > 128) {
            return SDK_RET_INVALID_ARG;
        }
    } else {
        //SDK_ASSERT(FALSE);
        ip_pfx->addr.af = IP_AF_NIL;
        return SDK_RET_INVALID_ARG;
    }
    ipaddr_proto_spec_to_api_spec(&ip_pfx->addr, in_ippfx.addr());
    return SDK_RET_OK;
}

static inline sdk_ret_t
ippfx_proto_spec_to_ipvx_range (ipvx_range_t *ip_range,
                                const types::IPPrefix& in_ippfx)
{
    ip_prefix_t ippfx;
    ip_addr_t lo_addr;
    ip_addr_t hi_addr;

    ippfx_proto_spec_to_api_spec(&ippfx, in_ippfx);
    ip_prefix_ip_low(&ippfx, &lo_addr);
    ip_prefix_ip_high(&ippfx, &hi_addr);
    if (ippfx.addr.af == IP_AF_IPV4) {
        ip_range->af = IP_AF_IPV4;
        ip_range->ip_lo.v4_addr = lo_addr.addr.v4_addr;
        ip_range->ip_hi.v4_addr = hi_addr.addr.v4_addr;
    } else if (ippfx.addr.af == IP_AF_IPV6) {
        ip_range->af = IP_AF_IPV6;
        memcpy(&ip_range->ip_lo.v6_addr, &lo_addr.addr.v6_addr,
               sizeof(ipv6_addr_t));
        memcpy(&ip_range->ip_hi.v6_addr, &hi_addr.addr.v6_addr,
               sizeof(ipv6_addr_t));
    } else {
        SDK_ASSERT(FALSE);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
ipsubnet_proto_spec_to_ipvx_range (ipvx_range_t *ip_range,
                                   const types::IPSubnet& in_ipsubnet)
{
    if (in_ipsubnet.has_ipv4subnet()) {
        return ippfx_proto_spec_to_ipvx_range(ip_range,
                                              in_ipsubnet.ipv4subnet());
    } else if (in_ipsubnet.has_ipv6subnet()) {
        return ippfx_proto_spec_to_ipvx_range(ip_range,
                                              in_ipsubnet.ipv6subnet());
    } else {
        SDK_ASSERT(FALSE);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
ipv6pfx_proto_spec_to_ippfx_api_spec (ip_prefix_t *ip_pfx,
                                      const types::IPv6Prefix& in_ippfx)
{
    if ((ip_pfx->len = in_ippfx.len()) > 128) {
        return SDK_RET_INVALID_ARG;
    }
    memcpy(ip_pfx->addr.addr.v6_addr.addr8, in_ippfx.addr().c_str(),
           IP6_ADDR8_LEN);
    return SDK_RET_OK;
}

static inline sdk_ret_t
ipv4pfx_proto_spec_to_ippfx_api_spec (ipv4_prefix_t *ip_pfx,
                                      const types::IPPrefix& in_ippfx)
{
    ip_pfx->len = in_ippfx.len();
    if ((in_ippfx.addr().af() == types::IP_AF_INET) &&
             (ip_pfx->len > 32)) {
        return SDK_RET_INVALID_ARG;
    } else {
        ip_pfx->v4_addr = in_ippfx.addr().v4addr();
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
ipv4pfx_proto_spec_to_api_spec (ipv4_prefix_t *ip_pfx,
                                const types::IPv4Prefix& in_ippfx)
{
    if ((ip_pfx->len = in_ippfx.len()) > 32) {
        return SDK_RET_INVALID_ARG;
    }
    ip_pfx->v4_addr = in_ippfx.addr();
    return SDK_RET_OK;
}

static inline sdk_ret_t
iprange_proto_spec_to_api_spec (ipvx_range_t *ip_range,
                                const types::AddressRange& in_iprange)
{
    if (in_iprange.has_ipv4range()) {
        ip_range->af = IP_AF_IPV4;
        ip_range->ip_lo.v4_addr = in_iprange.ipv4range().low().v4addr();
        ip_range->ip_hi.v4_addr = in_iprange.ipv4range().high().v4addr();
    } else if (in_iprange.has_ipv6range()) {
        ip_range->af = IP_AF_IPV6;
        memcpy(ip_range->ip_lo.v6_addr.addr8,
               in_iprange.ipv6range().low().v6addr().c_str(),
               IP6_ADDR8_LEN);
        memcpy(ip_range->ip_hi.v6_addr.addr8,
               in_iprange.ipv6range().high().v6addr().c_str(),
               IP6_ADDR8_LEN);
    } else {
        return SDK_RET_INVALID_ARG;
    }
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

static inline void
ipv4addr_api_spec_to_proto_spec (types::IPAddress *out_ipaddr,
                                 const ipv4_addr_t *in_ipaddr)
{
    out_ipaddr->set_af(types::IP_AF_INET);
    out_ipaddr->set_v4addr(*in_ipaddr);
}

static inline void
ipv6addr_api_spec_to_proto_spec (types::IPAddress *out_ipaddr,
                                 const ipv6_addr_t *in_ipaddr)
{
    out_ipaddr->set_af(types::IP_AF_INET6);
    out_ipaddr->set_v6addr((const char *)&(in_ipaddr->addr8),
                           IP6_ADDR8_LEN);
}

static inline sdk_ret_t
ippfx_api_spec_to_proto_spec (types::IPPrefix *out_ippfx,
                              const ip_prefix_t *in_ippfx)
{
    out_ippfx->set_len(in_ippfx->len);
    return ipaddr_api_spec_to_proto_spec(out_ippfx->mutable_addr(), &in_ippfx->addr);
}

static inline sdk_ret_t
ippfx_api_spec_to_subnet_proto_spec (types::IPSubnet *out_subnet,
                                     const ip_prefix_t *in_ippfx)
{
    if (in_ippfx->addr.af == IP_AF_IPV4) {
        ippfx_api_spec_to_proto_spec(out_subnet->mutable_ipv4subnet(),
                                     in_ippfx);
    } else if (in_ippfx->addr.af == IP_AF_IPV6) {
        ippfx_api_spec_to_proto_spec(out_subnet->mutable_ipv6subnet(),
                                     in_ippfx);
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
ippfx_api_spec_to_ipv6pfx_proto_spec (types::IPv6Prefix *out_ippfx,
                                      const ip_prefix_t *in_ippfx)
{
    out_ippfx->set_len(in_ippfx->len);
    out_ippfx->set_addr(
                   std::string((const char *)&in_ippfx->addr.addr.v6_addr.addr8,
                               IP6_ADDR8_LEN));
    return SDK_RET_OK;
}

static inline sdk_ret_t
ipv4pfx_api_spec_to_ippfx_proto_spec (types::IPPrefix *out_ippfx,
                                      const ipv4_prefix_t *in_ippfx)
{
    auto out_addr = out_ippfx->mutable_addr();
    out_ippfx->set_len(in_ippfx->len);
    out_addr->set_af(types::IP_AF_INET);
    out_addr->set_v4addr(in_ippfx->v4_addr);
    return SDK_RET_OK;
}

static inline sdk_ret_t
ipv4pfx_api_spec_to_proto_spec (types::IPv4Prefix *out_ippfx,
                                const ipv4_prefix_t *in_ippfx)
{
    out_ippfx->set_len(in_ippfx->len);
    out_ippfx->set_addr(in_ippfx->v4_addr);
    return SDK_RET_OK;
}

static inline sdk_ret_t
iprange_api_spec_to_proto_spec (types::AddressRange *out_iprange,
                                const ipvx_range_t *in_iprange)
{
    switch (in_iprange->af) {
    case IP_AF_IPV4:
        {
            auto out_range = out_iprange->mutable_ipv4range();
            ipv4addr_api_spec_to_proto_spec(out_range->mutable_low(),
                                            &in_iprange->ip_lo.v4_addr);
            ipv4addr_api_spec_to_proto_spec(out_range->mutable_high(),
                                            &in_iprange->ip_hi.v4_addr);
        }
        break;
    case IP_AF_IPV6:
        {
            auto out_range = out_iprange->mutable_ipv4range();
            ipv6addr_api_spec_to_proto_spec(out_range->mutable_low(),
                                            &in_iprange->ip_lo.v6_addr);
            ipv6addr_api_spec_to_proto_spec(out_range->mutable_high(),
                                            &in_iprange->ip_hi.v6_addr);
        }
        break;
    default:
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

// TODO: optimize to avoid to full walk on all key bytes
static inline sdk_ret_t
pds_obj_key_proto_to_api_spec (pds_obj_key_t *api_spec,
                               const ::std::string& proto_key)
{
    if (proto_key.length() > PDS_MAX_KEY_LEN) {
        return SDK_RET_INVALID_ARG;
    }
    // set all the key bytes to 0 1st and hash on the full key can't include
    // uninitialized memory
    memset(api_spec->id, 0, sizeof(api_spec->id));
    // set the key bytes
    memcpy(api_spec->id, proto_key.data(),
           MIN(proto_key.length(), PDS_MAX_KEY_LEN));
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_encap_to_proto_encap (types::Encap *proto_encap,
                          const pds_encap_t *pds_encap)
{
    switch (pds_encap->type) {
    case PDS_ENCAP_TYPE_NONE:
        proto_encap->set_type(types::ENCAP_TYPE_NONE);
        break;

    case PDS_ENCAP_TYPE_DOT1Q:
        proto_encap->set_type(types::ENCAP_TYPE_DOT1Q);
        proto_encap->mutable_value()->set_vlanid(pds_encap->val.vlan_tag);
        break;

    case PDS_ENCAP_TYPE_QINQ:
        proto_encap->set_type(types::ENCAP_TYPE_QINQ);
        proto_encap->mutable_value()->mutable_qinqtag()->
            set_ctag(pds_encap->val.qinq_tag.c_tag);
        proto_encap->mutable_value()->mutable_qinqtag()->
            set_stag(pds_encap->val.qinq_tag.s_tag);
        break;

    case PDS_ENCAP_TYPE_MPLSoUDP:
        proto_encap->set_type(types::ENCAP_TYPE_MPLSoUDP);
        proto_encap->mutable_value()->set_mplstag(pds_encap->val.mpls_tag);
        break;

    case PDS_ENCAP_TYPE_VXLAN:
        proto_encap->set_type(types::ENCAP_TYPE_VXLAN);
        proto_encap->mutable_value()->set_vnid(pds_encap->val.vnid);
        break;

    default:
        return SDK_RET_INVALID_ARG;
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
        pds_encap.val.qinq_tag.c_tag = encap.value().qinqtag().ctag();
        pds_encap.val.qinq_tag.s_tag = encap.value().qinqtag().stag();
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

static inline types::ApiStatus
sdk_ret_to_api_status (sdk_ret_t ret)
{
    switch (ret) {
    case SDK_RET_OK:
        return types::ApiStatus::API_STATUS_OK;

    case sdk::SDK_RET_OOM:
        return types::ApiStatus::API_STATUS_OUT_OF_MEM;

    case SDK_RET_INVALID_ARG:
        return types::ApiStatus::API_STATUS_INVALID_ARG;

    case sdk::SDK_RET_ENTRY_NOT_FOUND:
        return types::ApiStatus::API_STATUS_NOT_FOUND;

    case sdk::SDK_RET_ENTRY_EXISTS:
        return types::ApiStatus::API_STATUS_EXISTS_ALREADY;

    case sdk::SDK_RET_NO_RESOURCE:
        return types::ApiStatus::API_STATUS_OUT_OF_RESOURCE;

    default:
        return types::ApiStatus::API_STATUS_ERR;
    }
}

static inline sdk_ret_t
pds_af_proto_spec_to_api_spec (uint8_t *af, const types::IPAF &addrfamily)
{
    if (addrfamily == types::IP_AF_INET) {
        *af = IP_AF_IPV4;
    } else if (addrfamily == types::IP_AF_INET6) {
        *af = IP_AF_IPV6;
    } else {
        PDS_TRACE_ERR("IP_AF_NONE passed in proto");
        SDK_ASSERT(FALSE);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline types::IPAF
pds_af_api_spec_to_proto_spec (uint8_t af)
{
    if (af == IP_AF_IPV4) {
        return types::IP_AF_INET;
    } else if (af == IP_AF_IPV6) {
        return types::IP_AF_INET6;
    } else {
        return types::IP_AF_NONE;
    }
}

static inline cmd_msg_t
pds_proto_cmd_to_api_cmd (types::Command proto_cmd)
{
    switch (proto_cmd) {
    case types::CMD_MAPPING_DUMP:
        return CMD_MSG_MAPPING_DUMP;
    case types::CMD_NACL_DUMP:
        return CMD_MSG_NACL_DUMP;
    case types::CMD_INTR_DUMP:
        return CMD_MSG_INTR_DUMP;
    case types::CMD_INTR_CLEAR:
        return CMD_MSG_INTR_CLEAR;
    case types::CMD_API_ENGINE_STATS_DUMP:
        return CMD_MSG_API_ENGINE_STATS_DUMP;
    case types::CMD_FLOW_DUMP:
        return CMD_MSG_FLOW_DUMP;
    case types::CMD_STORE_STATS_DUMP:
        return CMD_MSG_STORE_STATS_DUMP;
    case types::CMD_NAT_PB_DUMP:
        return CMD_MSG_NAT_PB_DUMP;
    case types::CMD_PORT_FSM_DUMP:
        return CMD_MSG_PORT_FSM_DUMP;
    default:
        return CMD_MSG_MAX;
    }
}

static inline mapping_dump_type_t
proto_mapping_dump_type_to_pds (pds::MappingDumpType type)
{
    switch (type) {
    case pds::MAPPING_DUMP_REMOTE_L2:
        return MAPPING_DUMP_TYPE_REMOTE_L2;
    case pds::MAPPING_DUMP_REMOTE_L3:
        return MAPPING_DUMP_TYPE_REMOTE_L3;
    case pds::MAPPING_DUMP_LOCAL:
    default:
        return MAPPING_DUMP_TYPE_LOCAL;
    }
}

const static std::map<std::pair<std::string, types::ServiceRequestOp>, cfg_msg_t>  g_svc_cfg_map =
{
    {std::make_pair("pds.VPCRequest", types::SERVICE_OP_CREATE),                   CFG_MSG_VPC_CREATE},
    {std::make_pair("pds.VPCRequest", types::SERVICE_OP_UPDATE),                   CFG_MSG_VPC_UPDATE},
    {std::make_pair("pds.VPCDeleteRequest", types::SERVICE_OP_DELETE),             CFG_MSG_VPC_DELETE},
    {std::make_pair("pds.VPCGetRequest", types::SERVICE_OP_READ),                  CFG_MSG_VPC_GET},
    {std::make_pair("pds.VPCPeerRequest", types::SERVICE_OP_CREATE),               CFG_MSG_VPC_PEER_CREATE},
    {std::make_pair("pds.VPCPeerDeleteRequest", types::SERVICE_OP_DELETE),         CFG_MSG_VPC_PEER_DELETE},
    {std::make_pair("pds.VPCPeerGetRequest", types::SERVICE_OP_READ),              CFG_MSG_VPC_PEER_GET},
    {std::make_pair("pds.VnicRequest", types::SERVICE_OP_CREATE),                  CFG_MSG_VNIC_CREATE},
    {std::make_pair("pds.VnicRequest", types::SERVICE_OP_UPDATE),                  CFG_MSG_VNIC_UPDATE},
    {std::make_pair("pds.VnicDeleteRequest", types::SERVICE_OP_DELETE),            CFG_MSG_VNIC_DELETE},
    {std::make_pair("pds.VnicGetRequest", types::SERVICE_OP_READ),                 CFG_MSG_VNIC_GET},
    {std::make_pair("pds.SubnetRequest", types::SERVICE_OP_CREATE),                CFG_MSG_SUBNET_CREATE},
    {std::make_pair("pds.SubnetRequest", types::SERVICE_OP_UPDATE),                CFG_MSG_SUBNET_UPDATE},
    {std::make_pair("pds.SubnetDeleteRequest", types::SERVICE_OP_DELETE),          CFG_MSG_SUBNET_DELETE},
    {std::make_pair("pds.SubnetGetRequest", types::SERVICE_OP_READ),               CFG_MSG_SUBNET_GET},
    {std::make_pair("pds.SecurityPolicyRequest", types::SERVICE_OP_CREATE),        CFG_MSG_SECURITY_POLICY_CREATE},
    {std::make_pair("pds.SecurityPolicyRequest", types::SERVICE_OP_UPDATE),        CFG_MSG_SECURITY_POLICY_UPDATE},
    {std::make_pair("pds.SecurityPolicyDeleteRequest", types::SERVICE_OP_DELETE),  CFG_MSG_SECURITY_POLICY_DELETE},
    {std::make_pair("pds.SecurityPolicyGetRequest", types::SERVICE_OP_READ),       CFG_MSG_SECURITY_POLICY_GET},
    {std::make_pair("pds.SecurityRuleRequest", types::SERVICE_OP_CREATE),          CFG_MSG_SECURITY_RULE_CREATE},
    {std::make_pair("pds.SecurityRuleRequest", types::SERVICE_OP_UPDATE),          CFG_MSG_SECURITY_RULE_UPDATE},
    {std::make_pair("pds.SecurityRuleDeleteRequest", types::SERVICE_OP_DELETE),    CFG_MSG_SECURITY_RULE_DELETE},
    {std::make_pair("pds.SecurityRuleGetRequest", types::SERVICE_OP_READ),         CFG_MSG_SECURITY_RULE_GET},
    {std::make_pair("pds.SecurityProfileRequest", types::SERVICE_OP_CREATE),       CFG_MSG_SECURITY_PROFILE_CREATE},
    {std::make_pair("pds.SecurityProfileRequest", types::SERVICE_OP_UPDATE),       CFG_MSG_SECURITY_PROFILE_UPDATE},
    {std::make_pair("pds.SecurityProfileDeleteRequest", types::SERVICE_OP_DELETE), CFG_MSG_SECURITY_PROFILE_DELETE},
    {std::make_pair("pds.SecurityProfileGetRequest", types::SERVICE_OP_READ),      CFG_MSG_SECURITY_PROFILE_GET},
    {std::make_pair("pds.PolicerRequest", types::SERVICE_OP_CREATE),               CFG_MSG_POLICER_CREATE},
    {std::make_pair("pds.PolicerRequest", types::SERVICE_OP_UPDATE),               CFG_MSG_POLICER_UPDATE},
    {std::make_pair("pds.PolicerDeleteRequest", types::SERVICE_OP_DELETE),         CFG_MSG_POLICER_DELETE},
    {std::make_pair("pds.PolicerGetRequest", types::SERVICE_OP_READ),              CFG_MSG_POLICER_GET},
    {std::make_pair("pds.MappingRequest", types::SERVICE_OP_CREATE),               CFG_MSG_MAPPING_CREATE},
    {std::make_pair("pds.MappingRequest", types::SERVICE_OP_UPDATE),               CFG_MSG_MAPPING_UPDATE},
    {std::make_pair("pds.MappingDeleteRequest", types::SERVICE_OP_DELETE),         CFG_MSG_MAPPING_DELETE},
    {std::make_pair("pds.MappingGetRequest", types::SERVICE_OP_READ),              CFG_MSG_MAPPING_GET},
    {std::make_pair("pds.InterfaceRequest", types::SERVICE_OP_CREATE),             CFG_MSG_INTERFACE_CREATE},
    {std::make_pair("pds.InterfaceRequest", types::SERVICE_OP_UPDATE),             CFG_MSG_INTERFACE_UPDATE},
    {std::make_pair("pds.InterfaceDeleteRequest", types::SERVICE_OP_DELETE),       CFG_MSG_INTERFACE_DELETE},
    {std::make_pair("pds.InterfaceGetRequest", types::SERVICE_OP_READ),            CFG_MSG_INTERFACE_GET},
    {std::make_pair("pds.LifGetRequest", types::SERVICE_OP_READ),                  CFG_MSG_LIF_GET},
    {std::make_pair("pds.DHCPPolicyRequest", types::SERVICE_OP_CREATE),            CFG_MSG_DHCP_POLICY_CREATE},
    {std::make_pair("pds.DHCPPolicyRequest", types::SERVICE_OP_UPDATE),            CFG_MSG_DHCP_POLICY_UPDATE},
    {std::make_pair("pds.DHCPPolicyDeleteRequest", types::SERVICE_OP_DELETE),      CFG_MSG_DHCP_POLICY_DELETE},
    {std::make_pair("pds.DHCPPolicyGetRequest", types::SERVICE_OP_READ),           CFG_MSG_DHCP_POLICY_GET},
    {std::make_pair("pds.NatPortBlockRequest", types::SERVICE_OP_CREATE),          CFG_MSG_NAT_PORT_BLOCK_CREATE},
    {std::make_pair("pds.NatPortBlockDeleteRequest", types::SERVICE_OP_DELETE),    CFG_MSG_NAT_PORT_BLOCK_DELETE},
    {std::make_pair("pds.NatPortBlockGetRequest", types::SERVICE_OP_READ),         CFG_MSG_NAT_PORT_BLOCK_GET},
    {std::make_pair("pds.NexthopRequest", types::SERVICE_OP_CREATE),               CFG_MSG_NEXTHOP_CREATE},
    {std::make_pair("pds.NexthopRequest", types::SERVICE_OP_UPDATE),               CFG_MSG_NEXTHOP_UPDATE},
    {std::make_pair("pds.NexthopDeleteRequest", types::SERVICE_OP_DELETE),         CFG_MSG_NEXTHOP_DELETE},
    {std::make_pair("pds.NexthopGetRequest", types::SERVICE_OP_READ),              CFG_MSG_NEXTHOP_GET},
    {std::make_pair("pds.NhGroupRequest", types::SERVICE_OP_CREATE),               CFG_MSG_NHGROUP_CREATE},
    {std::make_pair("pds.NhGroupRequest", types::SERVICE_OP_UPDATE),               CFG_MSG_NHGROUP_UPDATE},
    {std::make_pair("pds.NhGroupDeleteRequest", types::SERVICE_OP_DELETE),         CFG_MSG_NHGROUP_DELETE},
    {std::make_pair("pds.NhGroupGetRequest", types::SERVICE_OP_READ),              CFG_MSG_NHGROUP_GET},
    {std::make_pair("pds.MeterRequest", types::SERVICE_OP_CREATE),                 CFG_MSG_METER_CREATE},
    {std::make_pair("pds.MeterRequest", types::SERVICE_OP_UPDATE),                 CFG_MSG_METER_UPDATE},
    {std::make_pair("pds.MeterDeleteRequest", types::SERVICE_OP_DELETE),           CFG_MSG_METER_DELETE},
    {std::make_pair("pds.MeterGetRequest", types::SERVICE_OP_READ),                CFG_MSG_METER_GET},
    {std::make_pair("pds.RouteTableRequest", types::SERVICE_OP_CREATE),            CFG_MSG_ROUTE_TABLE_CREATE},
    {std::make_pair("pds.RouteTableRequest", types::SERVICE_OP_UPDATE),            CFG_MSG_ROUTE_TABLE_UPDATE},
    {std::make_pair("pds.RouteTableDeleteRequest", types::SERVICE_OP_DELETE),      CFG_MSG_ROUTE_TABLE_DELETE},
    {std::make_pair("pds.RouteTableGetRequest", types::SERVICE_OP_READ),           CFG_MSG_ROUTE_TABLE_GET},
    {std::make_pair("pds.RouteRequest", types::SERVICE_OP_CREATE),                 CFG_MSG_ROUTE_CREATE},
    {std::make_pair("pds.RouteRequest", types::SERVICE_OP_UPDATE),                 CFG_MSG_ROUTE_UPDATE},
    {std::make_pair("pds.RouteDeleteRequest", types::SERVICE_OP_DELETE),           CFG_MSG_ROUTE_DELETE},
    {std::make_pair("pds.RouteGetRequest", types::SERVICE_OP_READ),                CFG_MSG_ROUTE_GET},
    {std::make_pair("pds.SvcMappingRequest", types::SERVICE_OP_CREATE),            CFG_MSG_SVC_MAPPING_CREATE},
    {std::make_pair("pds.SvcMappingRequest", types::SERVICE_OP_UPDATE),            CFG_MSG_SVC_MAPPING_UPDATE},
    {std::make_pair("pds.SvcMappingDeleteRequest", types::SERVICE_OP_DELETE),      CFG_MSG_SVC_MAPPING_DELETE},
    {std::make_pair("pds.SvcMappingGetRequest", types::SERVICE_OP_READ),           CFG_MSG_SVC_MAPPING_GET},
    {std::make_pair("pds.TunnelRequest", types::SERVICE_OP_CREATE),                CFG_MSG_TUNNEL_CREATE},
    {std::make_pair("pds.TunnelRequest", types::SERVICE_OP_UPDATE),                CFG_MSG_TUNNEL_UPDATE},
    {std::make_pair("pds.TunnelDeleteRequest", types::SERVICE_OP_DELETE),          CFG_MSG_TUNNEL_DELETE},
    {std::make_pair("pds.TunnelGetRequest", types::SERVICE_OP_READ),               CFG_MSG_TUNNEL_GET},
    {std::make_pair("pds.DeviceRequest", types::SERVICE_OP_CREATE),                CFG_MSG_DEVICE_CREATE},
    {std::make_pair("pds.DeviceRequest", types::SERVICE_OP_UPDATE),                CFG_MSG_DEVICE_UPDATE},
    {std::make_pair("pds.DeviceDeleteRequest", types::SERVICE_OP_DELETE),          CFG_MSG_DEVICE_DELETE},
    {std::make_pair("pds.DeviceGetRequest", types::SERVICE_OP_READ),               CFG_MSG_DEVICE_GET},
};

static inline cfg_msg_t
pds_proto_cfg_to_api_cfg (std::string cfg_msg,
                          types::ServiceRequestOp op)
{
    auto iter = g_svc_cfg_map.find(std::make_pair(cfg_msg, op));
    if (iter != g_svc_cfg_map.end()) {
        return iter->second;
    }
    return CFG_MSG_MAX;
}

static inline void
pds_cfg_proto_to_cfg_ctxt (cfg_ctxt_t *cfg_ctxt,
                           const types::ConfigMessage *proto_msg)
{
    google::protobuf::Any *any_msg;
    std::string cfg_msg;
    std::size_t delim_pos;

    cfg_ctxt->req = (void *)&proto_msg->configmsg();
    any_msg = (google::protobuf::Any *)cfg_ctxt->req;
    cfg_msg = any_msg->type_url();
    delim_pos = cfg_msg.find("/");
    if (delim_pos != std::string::npos) {
        cfg_msg = cfg_msg.substr(delim_pos + 1);
        cfg_ctxt->cfg = pds_proto_cfg_to_api_cfg(cfg_msg,
                                                 proto_msg->configop());
    } else {
        cfg_ctxt->cfg = CFG_MSG_MAX;
    }
}

static inline void
pds_cmd_proto_to_cmd_ctxt (cmd_ctxt_t *cmd_ctxt,
                           const types::CommandMessage *proto_msg,
                           int fd)
{
    google::protobuf::Any *any_msg;
    pds::MappingDumpFilter filter;
    pds::CommandUUID uuid;
    std::string cmd_msg;
    std::size_t delim_pos;

    cmd_ctxt->fd = fd;
    cmd_ctxt->cmd = pds_proto_cmd_to_api_cmd(proto_msg->command());

    if (proto_msg->has_commandmsg()) {
        any_msg = (google::protobuf::Any *)&proto_msg->commandmsg();
        cmd_msg = any_msg->type_url();
        delim_pos = cmd_msg.find("/");
        if (delim_pos != std::string::npos) {
            cmd_msg = cmd_msg.substr(delim_pos + 1);
            if (!cmd_msg.compare("pds.MappingDumpFilter")) {
                any_msg->UnpackTo(&filter);
                cmd_ctxt->args.valid = true;
                cmd_ctxt->args.mapping_dump.type =
                        proto_mapping_dump_type_to_pds(filter.type());
                if (filter.has_key()) {
                    auto key = filter.key();
                    cmd_ctxt->args.mapping_dump.key_valid = true;
                    switch (cmd_ctxt->args.mapping_dump.type) {
                    case MAPPING_DUMP_TYPE_LOCAL:
                    case MAPPING_DUMP_TYPE_REMOTE_L3:
                        cmd_ctxt->args.mapping_dump.skey.type = PDS_MAPPING_TYPE_L3;
                        cmd_ctxt->args.mapping_dump.skey.vpc = key.ipkey().vpcid();
                        ipaddr_proto_spec_to_api_spec(&cmd_ctxt->args.mapping_dump.skey.ip_addr,
                                                      key.ipkey().ipaddr());
                        break;
                    case MAPPING_DUMP_TYPE_REMOTE_L2:
                        cmd_ctxt->args.mapping_dump.skey.type = PDS_MAPPING_TYPE_L2;
                        cmd_ctxt->args.mapping_dump.skey.subnet = key.mackey().subnetid();
                        MAC_UINT64_TO_ADDR(cmd_ctxt->args.mapping_dump.skey.mac_addr,
                                           key.mackey().macaddr());
                        break;
                    default:
                        break;
                    }
                }
            } else if (!cmd_msg.compare("pds.CommandUUID")) {
                any_msg->UnpackTo(&uuid);
                cmd_ctxt->args.valid = true;
                pds_obj_key_proto_to_api_spec(&cmd_ctxt->args.port_id, uuid.id());
            }
        }
    }
}

static inline void
pds_svc_req_proto_to_svc_req_ctxt (svc_req_ctxt_t *svc_req,
                                   types::ServiceRequestMessage *proto_req,
                                   int fd)
{
    switch (proto_req->request_case()) {
    case types::ServiceRequestMessage::kCommand:
        {
            cmd_ctxt_t *ctxt = &svc_req->cmd_ctxt;
            svc_req->type = SVC_REQ_TYPE_CMD;
            return pds_cmd_proto_to_cmd_ctxt(ctxt,
                                             &proto_req->command(),
                                             fd);
        }
    case types::ServiceRequestMessage::kConfig:
        {
            cfg_ctxt_t *ctxt = &svc_req->cfg_ctxt;
            svc_req->type = SVC_REQ_TYPE_CFG;
            return pds_cfg_proto_to_cfg_ctxt(ctxt,
                                             &proto_req->config());
        }
    default:
        {
            svc_req->type = SVC_REQ_TYPE_NONE;
        }
        return;
    }
}

#endif    // __AGENT_SVC_SPECS_HPP__
