//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __AGENT_SVC_SPECS_HPP__
#define __AGENT_SVC_SPECS_HPP__

#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/asic/port.hpp"
#include "nic/sdk/platform/capri/capri_tm_utils.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/internal/pds_if.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_lif.hpp"
#include "nic/apollo/api/include/pds_dhcp.hpp"
#include "nic/apollo/api/include/pds_nat.hpp"
#include "nic/apollo/api/include/pds_flow.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/event.hpp"
#include "nic/apollo/agent/svc/tag.hpp"
#include "nic/apollo/agent/svc/meter.hpp"
#include "nic/apollo/agent/svc/vnic.hpp"
#include "nic/apollo/agent/svc/vpc.hpp"
#include "nic/apollo/agent/svc/tunnel.hpp"
#include "nic/apollo/agent/svc/session.hpp"
#include "nic/apollo/agent/svc/service.hpp"
#include "nic/apollo/agent/svc/port.hpp"
#include "nic/apollo/agent/svc/policy.hpp"
#include "nic/apollo/agent/svc/nh.hpp"
#include "nic/apollo/agent/svc/route.hpp"
#include "nic/apollo/agent/svc/debug.hpp"
#include "nic/apollo/agent/svc/device.hpp"
#include "nic/apollo/agent/svc/mirror.hpp"
#include "nic/apollo/agent/svc/subnet.hpp"
#include "nic/apollo/agent/svc/mapping.hpp"
#include "nic/apollo/agent/svc/interface.hpp"
#include "nic/apollo/agent/svc/nat.hpp"
#include "nic/apollo/agent/svc/dhcp.hpp"
#include "nic/apollo/agent/svc/event.hpp"
#include "nic/apollo/agent/svc/learn.hpp"
#include "nic/apollo/learn/learn.hpp"
#include "nic/apollo/learn/learn_state.hpp"
#include "gen/proto/types.pb.h"
#include "nic/sdk/include/sdk/qos.hpp"

using sdk::asic::pd::port_queue_credit_t;
using sdk::asic::pd::queue_credit_t;

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

static inline pds_if_state_t
proto_admin_state_to_pds_admin_state (pds::IfStatus state)
{
    switch (state) {
    case pds::IF_STATUS_DOWN:
        return PDS_IF_STATE_DOWN;
    case pds::IF_STATUS_UP:
        return PDS_IF_STATE_UP;
    default:
        return PDS_IF_STATE_NONE;
    }
}

static inline cli_cmd_t
pds_proto_cmd_to_api_cmd (pds::Command proto_cmd)
{
    switch (proto_cmd) {
    case pds::CMD_MAPPING_DUMP:
        return CLI_CMD_MAPPING_DUMP;
    case pds::CMD_NACL_DUMP:
        return CLI_CMD_NACL_DUMP;
    case pds::CMD_INTR_DUMP:
        return CLI_CMD_INTR_DUMP;
    case pds::CMD_INTR_CLEAR:
        return CLI_CMD_INTR_CLEAR;
    case pds::CMD_API_ENGINE_STATS_DUMP:
        return CLI_CMD_API_ENGINE_STATS_DUMP;
    case pds::CMD_FLOW_DUMP:
        return CLI_CMD_FLOW_DUMP;
    case pds::CMD_STORE_STATS_DUMP:
        return CLI_CMD_STORE_STATS_DUMP;
    case pds::CMD_NAT_PB_DUMP:
        return CLI_CMD_NAT_PB_DUMP;
    case pds::CMD_PORT_FSM_DUMP:
        return CLI_CMD_PORT_FSM_DUMP;
    default:
        return CLI_CMD_MAX;
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

static inline void
pds_cmd_proto_to_cmd_ctxt (cmd_ctxt_t *cmd_ctxt,
                           pds::CommandCtxt *proto_ctxt,
                           int fd)
{
    cmd_ctxt->fd = fd;
    cmd_ctxt->cmd = pds_proto_cmd_to_api_cmd(proto_ctxt->cmd());
    if (proto_ctxt->has_mappingdumpfilter()) {
        cmd_ctxt->args.valid = true;
        cmd_ctxt->args.mapping_dump.type =
                proto_mapping_dump_type_to_pds(
                proto_ctxt->mappingdumpfilter().type());
        if (proto_ctxt->mappingdumpfilter().has_key()) {
            auto key = proto_ctxt->mappingdumpfilter().key();
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
    } else if (proto_ctxt->cmd() == pds::CMD_PORT_FSM_DUMP) {
        cmd_ctxt->args.valid = true;
        pds_obj_key_proto_to_api_spec(&cmd_ctxt->args.port_id,
                                      proto_ctxt->id());
    }
}

static inline types::LifType
pds_lif_type_to_proto_lif_type (lif_type_t lif_type)
{
    switch (lif_type) {
    case lif_type_t::LIF_TYPE_HOST:
        return types::LIF_TYPE_HOST;
    case lif_type_t::LIF_TYPE_HOST_MGMT:
        return types::LIF_TYPE_HOST_MGMT;
    case lif_type_t::LIF_TYPE_MNIC_OOB_MGMT:
        return types::LIF_TYPE_OOB_MGMT;
    case lif_type_t::LIF_TYPE_MNIC_INBAND_MGMT:
        return types::LIF_TYPE_INBAND_MGMT;
    case lif_type_t::LIF_TYPE_MNIC_INTERNAL_MGMT:
        return types::LIF_TYPE_INTERNAL_MGMT;
    case lif_type_t::LIF_TYPE_MNIC_CPU:
        return types::LIF_TYPE_DATAPATH;
    case lif_type_t::LIF_TYPE_LEARN:
        return types::LIF_TYPE_LEARN;
    case lif_type_t::LIF_TYPE_CONTROL:
        return types::LIF_TYPE_CONTROL;
    default:
        break;
    }
    return types::LIF_TYPE_NONE;
}

static inline pds::PortLinkSM
pds_fsmstate_to_proto (sdk::types::port_link_sm_t fsm_state)
{
    switch (fsm_state) {
    case port_link_sm_t::PORT_LINK_SM_DISABLED:
        return pds::PORT_LINK_FSM_DISABLED;
    case port_link_sm_t::PORT_LINK_SM_ENABLED:
        return pds::PORT_LINK_FSM_ENABLED;
    case port_link_sm_t::PORT_LINK_SM_AN_CFG:
        return pds::PORT_LINK_FSM_AN_CFG;
    case port_link_sm_t::PORT_LINK_SM_AN_DISABLED:
        return pds::PORT_LINK_FSM_AN_DISABLED;
    case port_link_sm_t::PORT_LINK_SM_AN_START:
        return pds::PORT_LINK_FSM_AN_START;
    case port_link_sm_t::PORT_LINK_SM_AN_WAIT_HCD:
        return pds::PORT_LINK_FSM_AN_WAIT_HCD;
    case port_link_sm_t::PORT_LINK_SM_AN_COMPLETE:
        return pds::PORT_LINK_FSM_AN_COMPLETE;
    case port_link_sm_t::PORT_LINK_SM_SERDES_CFG:
        return pds::PORT_LINK_FSM_SERDES_CFG;
    case port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY:
        return pds::PORT_LINK_FSM_WAIT_SERDES_RDY;
    case port_link_sm_t::PORT_LINK_SM_MAC_CFG:
        return pds::PORT_LINK_FSM_MAC_CFG;
    case port_link_sm_t::PORT_LINK_SM_SIGNAL_DETECT:
        return pds::PORT_LINK_FSM_SIGNAL_DETECT;
    case port_link_sm_t::PORT_LINK_SM_AN_DFE_TUNING:
        return pds::PORT_LINK_FSM_AN_DFE_TUNING;
    case port_link_sm_t::PORT_LINK_SM_DFE_TUNING:
        return pds::PORT_LINK_FSM_DFE_TUNING;
    case port_link_sm_t::PORT_LINK_SM_DFE_DISABLED:
        return pds::PORT_LINK_FSM_DFE_DISABLED;
    case port_link_sm_t::PORT_LINK_SM_DFE_START_ICAL:
        return pds::PORT_LINK_FSM_DFE_START_ICAL;
    case port_link_sm_t::PORT_LINK_SM_DFE_WAIT_ICAL:
        return pds::PORT_LINK_FSM_DFE_WAIT_ICAL;
    case port_link_sm_t::PORT_LINK_SM_DFE_START_PCAL:
        return pds::PORT_LINK_FSM_DFE_START_PCAL;
    case port_link_sm_t::PORT_LINK_SM_DFE_WAIT_PCAL:
        return pds::PORT_LINK_FSM_DFE_WAIT_PCAL;
    case port_link_sm_t::PORT_LINK_SM_DFE_PCAL_CONTINUOUS:
        return pds::PORT_LINK_FSM_DFE_PCAL_CONTINUOUS;
    case port_link_sm_t::PORT_LINK_SM_CLEAR_MAC_REMOTE_FAULTS:
        return pds::PORT_LINK_FSM_CLEAR_MAC_REMOTE_FAULTS;
    case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_SYNC:
        return pds::PORT_LINK_FSM_WAIT_MAC_SYNC;
    case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR:
        return pds::PORT_LINK_FSM_WAIT_MAC_FAULTS_CLEAR;
    case port_link_sm_t::PORT_LINK_SM_UP:
        return pds::PORT_LINK_FSM_UP;
    default:
        return pds::PORT_LINK_FSM_DISABLED;
    }
}

static inline sdk_ret_t
pds_dhcp_relay_proto_to_api_spec (pds_dhcp_relay_spec_t *api_spec,
                                  const pds::DHCPRelaySpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->vpc, proto_spec.vpcid());
    ipaddr_proto_spec_to_api_spec(&api_spec->server_ip, proto_spec.serverip());
    ipaddr_proto_spec_to_api_spec(&api_spec->agent_ip, proto_spec.agentip());
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_proxy_proto_to_api_spec (pds_dhcp_proxy_spec_t *api_spec,
                                  const pds::DHCPProxySpec &proto_spec)
{
    ipaddr_proto_spec_to_api_spec(&api_spec->server_ip, proto_spec.serverip());
    api_spec->mtu = proto_spec.mtu();
    ipaddr_proto_spec_to_api_spec(&api_spec->gateway_ip,
                                  proto_spec.gatewayip());
    ipaddr_proto_spec_to_api_spec(&api_spec->dns_server_ip,
                                  proto_spec.dnsserverip());
    ipaddr_proto_spec_to_api_spec(&api_spec->ntp_server_ip,
                                  proto_spec.ntpserverip());
    if (proto_spec.domainname().empty()) {
        api_spec->domain_name[0] = '\0';
    } else {
        strncpy(api_spec->domain_name, proto_spec.domainname().c_str(),
                PDS_MAX_DOMAIN_NAME_LEN);
        api_spec->domain_name[PDS_MAX_DOMAIN_NAME_LEN] = '\0';
    }
    api_spec->lease_timeout = proto_spec.leasetimeout();
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_policy_proto_to_api_spec (pds_dhcp_policy_spec_t *api_spec,
                                   const pds::DHCPPolicySpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    if (proto_spec.has_relayspec()) {
        api_spec->type = PDS_DHCP_POLICY_TYPE_RELAY;
        pds_dhcp_relay_proto_to_api_spec(&api_spec->relay_spec,
                                         proto_spec.relayspec());
    } else if (proto_spec.has_proxyspec()) {
        api_spec->type = PDS_DHCP_POLICY_TYPE_PROXY;
        pds_dhcp_proxy_proto_to_api_spec(&api_spec->proxy_spec,
                                         proto_spec.proxyspec());
    } else {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_relay_api_spec_to_proto (pds::DHCPRelaySpec *proto_spec,
                                  const pds_dhcp_relay_spec_t *api_spec)
{
    //proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpcid(api_spec->vpc.id, PDS_MAX_KEY_LEN);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_serverip(),
                                  &api_spec->server_ip);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_agentip(),
                                  &api_spec->agent_ip);
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_proxy_api_spec_to_proto (pds::DHCPProxySpec *proto_spec,
                                  const pds_dhcp_proxy_spec_t *api_spec)
{
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_serverip(),
                                  &api_spec->server_ip);
    proto_spec->set_mtu(api_spec->mtu);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_gatewayip(),
                                  &api_spec->gateway_ip);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_dnsserverip(),
                                  &api_spec->dns_server_ip);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_ntpserverip(),
                                  &api_spec->ntp_server_ip);
    proto_spec->set_domainname(api_spec->domain_name);
    proto_spec->set_leasetimeout(api_spec->lease_timeout);
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_policy_api_spec_to_proto (pds::DHCPPolicySpec *proto_spec,
                                   const pds_dhcp_policy_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    if (api_spec->type == PDS_DHCP_POLICY_TYPE_RELAY) {
        pds_dhcp_relay_api_spec_to_proto(proto_spec->mutable_relayspec(),
                                         &api_spec->relay_spec);
    } else if (api_spec->type == PDS_DHCP_POLICY_TYPE_PROXY) {
        pds_dhcp_proxy_api_spec_to_proto(proto_spec->mutable_proxyspec(),
                                         &api_spec->proxy_spec);
    } else {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_relay_api_status_to_proto (pds::DHCPRelayStatus *proto_status,
                                    const pds_dhcp_relay_status_t *api_status)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_proxy_api_status_to_proto (pds::DHCPProxyStatus *proto_status,
                                    const pds_dhcp_proxy_status_t *api_status)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_relay_api_stats_to_proto (pds::DHCPRelayStats *proto_stats,
                                   const pds_dhcp_relay_stats_t *api_stats)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_proxy_api_stats_to_proto (pds::DHCPProxyStats *proto_stats,
                                   const pds_dhcp_proxy_stats_t *api_stats)
{
    return SDK_RET_OK;
}

// populate proto buf from route table API info
static inline void
pds_dhcp_policy_api_info_to_proto (const pds_dhcp_policy_info_t *api_info,
                                  void *ctxt)
{
    pds::DHCPPolicyGetResponse *proto_rsp = (pds::DHCPPolicyGetResponse *)ctxt;
    auto dhcp = proto_rsp->add_response();
    pds::DHCPPolicySpec *proto_spec = dhcp->mutable_spec();
    pds::DHCPPolicyStatus *proto_status = dhcp->mutable_status();
    pds::DHCPPolicyStats *proto_stats = dhcp->mutable_stats();

    pds_dhcp_policy_api_spec_to_proto(proto_spec, &api_info->spec);
    if (api_info->spec.type == PDS_DHCP_POLICY_TYPE_RELAY) {
        pds_dhcp_relay_api_status_to_proto(proto_status->mutable_relaystatus(),
                                           &api_info->status.relay_status);
        pds_dhcp_relay_api_stats_to_proto(proto_stats->mutable_relaystats(),
                                          &api_info->stats.relay_stats);
    } else if (api_info->spec.type == PDS_DHCP_POLICY_TYPE_PROXY) {
        pds_dhcp_proxy_api_status_to_proto(proto_status->mutable_proxystatus(),
                                           &api_info->status.proxy_status);
        pds_dhcp_proxy_api_stats_to_proto(proto_stats->mutable_proxystats(),
                                          &api_info->stats.proxy_stats);
    }
}

static inline void
pds_ipv4_flow_to_proto (ftlite::internal::ipv4_entry_t *ipv4_entry,
                        void *ctxt)
{
    flow_get_t *fget = (flow_get_t *)ctxt;
    auto flow = fget->msg.add_flow();
    auto ipflowkey = flow->mutable_key()->mutable_ipflowkey();
    auto srcaddr = ipflowkey->mutable_srcip();
    auto dstaddr = ipflowkey->mutable_dstip();
    auto tcpudpinfo = ipflowkey->mutable_l4info()->mutable_tcpudpinfo();

    srcaddr->set_af(types::IP_AF_INET);
    srcaddr->set_v4addr(ipv4_entry->src);
    dstaddr->set_af(types::IP_AF_INET);
    dstaddr->set_v4addr(ipv4_entry->dst);
    ipflowkey->set_ipprotocol(ipv4_entry->proto);
    tcpudpinfo->set_srcport(ipv4_entry->sport);
    tcpudpinfo->set_dstport(ipv4_entry->dport);

    flow->set_vpc(ipv4_entry->vpc_id);
    flow->set_flowrole(ipv4_entry->flow_role);
    flow->set_sessionidx(ipv4_entry->session_index);
    flow->set_epoch(ipv4_entry->epoch);

    fget->count ++;
    if (fget->count == FLOW_GET_STREAM_COUNT) {
        fget->msg.set_apistatus(types::ApiStatus::API_STATUS_OK);
        fget->writer->Write(fget->msg);
        fget->msg.Clear();
        fget->count = 0;
    }
}

static inline void
pds_ipv6_flow_to_proto (ftlite::internal::ipv6_entry_t *ipv6_entry,
                        void *ctxt)
{
    flow_get_t *fget = (flow_get_t *)ctxt;
    auto flow = fget->msg.add_flow();
    auto ipflowkey = flow->mutable_key()->mutable_ipflowkey();
    auto srcaddr = ipflowkey->mutable_srcip();
    auto dstaddr = ipflowkey->mutable_dstip();
    auto tcpudpinfo = ipflowkey->mutable_l4info()->mutable_tcpudpinfo();

    srcaddr->set_af(types::IP_AF_INET6);
    srcaddr->set_v6addr(std::string((const char *)(ipv6_entry->src),
                        IP6_ADDR8_LEN));
    dstaddr->set_af(types::IP_AF_INET6);
    srcaddr->set_v6addr(std::string((const char *)(ipv6_entry->dst),
                        IP6_ADDR8_LEN));
    ipflowkey->set_ipprotocol(ipv6_entry->proto);
    tcpudpinfo->set_srcport(ipv6_entry->sport);
    tcpudpinfo->set_dstport(ipv6_entry->dport);

    flow->set_vpc(ipv6_entry->vpc_id);
    flow->set_flowrole(ipv6_entry->flow_role);
    flow->set_sessionidx(ipv6_entry->session_index);
    flow->set_epoch(ipv6_entry->epoch);

    fget->count ++;
    if (fget->count == FLOW_GET_STREAM_COUNT) {
        fget->msg.set_apistatus(types::ApiStatus::API_STATUS_OK);
        fget->writer->Write(fget->msg);
        fget->msg.Clear();
        fget->count = 0;
    }
}

// populate proto buf spec from mirror session API spec
static inline sdk_ret_t
pds_mirror_session_api_spec_to_proto (pds::MirrorSessionSpec *proto_spec,
                                      const pds_mirror_session_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_snaplen(api_spec->snap_len);
    switch (api_spec->type) {
    case PDS_MIRROR_SESSION_TYPE_RSPAN:
        {
            pds::RSpanSpec *proto_rspan = proto_spec->mutable_rspanspec();
            pds_encap_to_proto_encap(proto_rspan->mutable_encap(),
                                     &api_spec->rspan_spec.encap);
            proto_rspan->set_interface(api_spec->rspan_spec.interface.id,
                                       PDS_MAX_KEY_LEN);
        }
        break;

    case PDS_MIRROR_SESSION_TYPE_ERSPAN:
        {
            pds::ERSpanSpec *proto_erspan = proto_spec->mutable_erspanspec();
            proto_erspan->set_tunnelid(api_spec->erspan_spec.tep.id,
                                       PDS_MAX_KEY_LEN);
            proto_erspan->set_dscp(api_spec->erspan_spec.dscp);
            proto_erspan->set_spanid(api_spec->erspan_spec.span_id);
            proto_erspan->set_vpcid(api_spec->erspan_spec.vpc.id,
                                    PDS_MAX_KEY_LEN);
        }
        break;

    default:
        PDS_TRACE_ERR("Unknown mirror session type {}", api_spec->type);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

// populate proto buf status from mirror session API status
static inline void
pds_mirror_session_api_status_to_proto (pds::MirrorSessionStatus *proto_status,
                                        const pds_mirror_session_status_t *api_status)
{
}

// populate proto buf stats from mirror session API stats
static inline void
pds_mirror_session_api_stats_to_proto (pds::MirrorSessionStats *proto_stats,
                                       const pds_mirror_session_stats_t *api_stats)
{
}

// populate proto buf from mirror session API info
static inline void
pds_mirror_session_api_info_to_proto (const pds_mirror_session_info_t *api_info,
                                      void *ctxt)
{
    pds::MirrorSessionGetResponse *proto_rsp =
        (pds::MirrorSessionGetResponse *)ctxt;
    auto mirror_session = proto_rsp->add_response();
    pds::MirrorSessionSpec *proto_spec = mirror_session->mutable_spec();
    pds::MirrorSessionStatus *proto_status = mirror_session->mutable_status();
    pds::MirrorSessionStats *proto_stats = mirror_session->mutable_stats();

    pds_mirror_session_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_mirror_session_api_status_to_proto(proto_status, &api_info->status);
    pds_mirror_session_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build mirror session API spec from protobuf spec
static inline sdk_ret_t
pds_mirror_session_proto_to_api_spec (pds_mirror_session_spec_t *api_spec,
                                      const pds::MirrorSessionSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    api_spec->snap_len = proto_spec.snaplen();
    if (proto_spec.has_rspanspec()) {
        api_spec->type = PDS_MIRROR_SESSION_TYPE_RSPAN;
        api_spec->rspan_spec.encap =
            proto_encap_to_pds_encap(proto_spec.rspanspec().encap());
        if (api_spec->rspan_spec.encap.type != PDS_ENCAP_TYPE_DOT1Q) {
            PDS_TRACE_ERR("Invalid encap type {} in RSPAN mirror session {} "
                          "spec, only PDS_ENCAP_TYPE_DOT1Q encap type is valid",
                          api_spec->rspan_spec.encap.type, api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        if (api_spec->rspan_spec.encap.val.vlan_tag == 0) {
            PDS_TRACE_ERR("Invalid vlan tag 0 in RSPAN mirror session {} spec",
                          api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        pds_obj_key_proto_to_api_spec(&api_spec->rspan_spec.interface,
                                      proto_spec.rspanspec().interface());
    } else if (proto_spec.has_erspanspec()) {
        api_spec->type = PDS_MIRROR_SESSION_TYPE_ERSPAN;
        pds_obj_key_proto_to_api_spec(&api_spec->erspan_spec.tep,
                                      proto_spec.erspanspec().tunnelid());
        api_spec->erspan_spec.dscp = proto_spec.erspanspec().dscp();
        api_spec->erspan_spec.span_id = proto_spec.erspanspec().spanid();
        pds_obj_key_proto_to_api_spec(&api_spec->erspan_spec.vpc,
                                      proto_spec.erspanspec().vpcid());
    } else {
        PDS_TRACE_ERR("rspan & erspan config missing in mirror session {} spec",
                      api_spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

// populate proto buf spec from device API spec
static inline void
pds_device_api_spec_to_proto (pds::DeviceSpec *proto_spec,
                              const pds_device_spec_t *api_spec)
{
    if ((api_spec->device_ip_addr.af == IP_AF_IPV4) ||
        (api_spec->device_ip_addr.af == IP_AF_IPV6)) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_ipaddr(),
                                      &api_spec->device_ip_addr);
    }
    proto_spec->set_macaddr(MAC_TO_UINT64(api_spec->device_mac_addr));
    if ((api_spec->gateway_ip_addr.af == IP_AF_IPV4) ||
        (api_spec->gateway_ip_addr.af == IP_AF_IPV6)) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_gatewayip(),
                                      &api_spec->gateway_ip_addr);
    }
    proto_spec->set_bridgingen(api_spec->bridging_en);
    proto_spec->set_learningen(api_spec->learning_en);
    proto_spec->set_learnagetimeout(api_spec->learn_age_timeout);
    proto_spec->set_overlayroutingen(api_spec->overlay_routing_en);
    switch (api_spec->dev_oper_mode) {
    case PDS_DEV_OPER_MODE_BITW:
        proto_spec->set_devopermode(pds::DEVICE_OPER_MODE_BITW);
        break;
    case PDS_DEV_OPER_MODE_HOST:
        proto_spec->set_devopermode(pds::DEVICE_OPER_MODE_HOST);
        break;
    default:
        proto_spec->set_devopermode(pds::DEVICE_OPER_MODE_NONE);
        break;
    }
    switch (api_spec->device_profile) {
    case PDS_DEVICE_PROFILE_2PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_2PF);
        break;
    case PDS_DEVICE_PROFILE_3PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_3PF);
        break;
    case PDS_DEVICE_PROFILE_4PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_4PF);
        break;
    case PDS_DEVICE_PROFILE_5PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_5PF);
        break;
    case PDS_DEVICE_PROFILE_6PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_6PF);
        break;
    case PDS_DEVICE_PROFILE_7PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_7PF);
        break;
    case PDS_DEVICE_PROFILE_8PF:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_8PF);
        break;
    default:
        proto_spec->set_deviceprofile(pds::DEVICE_PROFILE_DEFAULT);
        break;
    }
    switch (api_spec->memory_profile) {
    case PDS_MEMORY_PROFILE_DEFAULT:
    default:
        proto_spec->set_memoryprofile(pds::MEMORY_PROFILE_DEFAULT);
        break;
    }
}

// populate proto buf status from device API status
static inline void
pds_device_api_status_to_proto (pds::DeviceStatus *proto_status,
                                const pds_device_status_t *api_status)
{
    proto_status->set_systemmacaddress(MAC_TO_UINT64(api_status->fru_mac));
    proto_status->set_memory(api_status->memory_cap);
}

// populate proto buf stats from device API stats
static inline void
pds_device_api_stats_to_proto (pds::DeviceStats *proto_stats,
                               const pds_device_stats_t *api_stats)
{
    uint32_t i;
    pds::DeviceStatsEntry *entry;

    for (i = 0; i < api_stats->ing_drop_stats_count; i++) {
        entry = proto_stats->add_ingress();
        entry->set_name(api_stats->ing_drop_stats[i].name);
        entry->set_count(api_stats->ing_drop_stats[i].count);
    }

    for (i = 0; i < api_stats->egr_drop_stats_count; i++) {
        entry = proto_stats->add_egress();
        entry->set_name(api_stats->egr_drop_stats[i].name);
        entry->set_count(api_stats->egr_drop_stats[i].count);
    }
}

static inline sdk_ret_t
pds_device_proto_to_api_spec (pds_device_spec_t *api_spec,
                              const pds::DeviceSpec &proto_spec)
{
    types::IPAddress ipaddr = proto_spec.ipaddr();
    types::IPAddress gwipaddr = proto_spec.gatewayip();
    uint64_t macaddr = proto_spec.macaddr();

    memset(api_spec, 0, sizeof(pds_device_spec_t));
    if ((ipaddr.af() == types::IP_AF_INET) ||
        (ipaddr.af() == types::IP_AF_INET6)) {
        ipaddr_proto_spec_to_api_spec(&api_spec->device_ip_addr, ipaddr);
    }
    MAC_UINT64_TO_ADDR(api_spec->device_mac_addr, macaddr);
    if ((gwipaddr.af() == types::IP_AF_INET) ||
        (gwipaddr.af() == types::IP_AF_INET6)) {
        ipaddr_proto_spec_to_api_spec(&api_spec->gateway_ip_addr, gwipaddr);
    }
    api_spec->bridging_en = proto_spec.bridgingen();
    api_spec->learning_en = proto_spec.learningen();
    api_spec->learn_age_timeout = proto_spec.learnagetimeout();
    api_spec->overlay_routing_en = proto_spec.overlayroutingen();
    switch (proto_spec.devopermode()) {
    case pds::DEVICE_OPER_MODE_BITW:
        api_spec->dev_oper_mode = PDS_DEV_OPER_MODE_BITW;
        break;
    case pds::DEVICE_OPER_MODE_HOST:
        api_spec->dev_oper_mode = PDS_DEV_OPER_MODE_HOST;
        break;
    default:
        api_spec->dev_oper_mode = PDS_DEV_OPER_MODE_NONE;
        break;
    }
    switch (proto_spec.deviceprofile()) {
    case pds::DEVICE_PROFILE_2PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_2PF;
        break;
    case pds::DEVICE_PROFILE_3PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_3PF;
        break;
    case pds::DEVICE_PROFILE_4PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_4PF;
        break;
    case pds::DEVICE_PROFILE_5PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_5PF;
        break;
    case pds::DEVICE_PROFILE_6PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_6PF;
        break;
    case pds::DEVICE_PROFILE_7PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_7PF;
        break;
    case pds::DEVICE_PROFILE_8PF:
        api_spec->device_profile = PDS_DEVICE_PROFILE_8PF;
        break;
    default:
        api_spec->device_profile = PDS_DEVICE_PROFILE_DEFAULT;
        break;
    }
    switch (proto_spec.memoryprofile()) {
    case pds::MEMORY_PROFILE_DEFAULT:
    default:
        api_spec->memory_profile = PDS_MEMORY_PROFILE_DEFAULT;
        break;
    }
    return SDK_RET_OK;
}

static inline pds::EventId
pds_event_id_api_to_proto_event_id (pds_event_id_t event_id)
{
    switch (event_id) {
    case PDS_EVENT_ID_LIF_CREATE:
        return pds::EVENT_ID_LIF_CREATE;
    case PDS_EVENT_ID_LIF_UPDATE:
        return pds::EVENT_ID_LIF_UPDATE;
    case PDS_EVENT_ID_LIF_UP:
        return pds::EVENT_ID_LIF_UP;
    case PDS_EVENT_ID_LIF_DOWN:
        return pds::EVENT_ID_LIF_DOWN;
    case PDS_EVENT_ID_PORT_CREATE:
        return pds::EVENT_ID_PORT_CREATE;
    case PDS_EVENT_ID_PORT_UP:
        return pds::EVENT_ID_PORT_UP;
    case PDS_EVENT_ID_PORT_DOWN:
        return pds::EVENT_ID_PORT_DOWN;
    default:
        break;
    }
    return pds::EVENT_ID_NONE;
}

static inline pds_event_id_t
pds_proto_event_id_to_api_event_id (pds::EventId proto_event_id)
{
    switch (proto_event_id) {
    case pds::EVENT_ID_PORT_CREATE:
        return PDS_EVENT_ID_PORT_CREATE;
    case pds::EVENT_ID_PORT_UP:
        return PDS_EVENT_ID_PORT_UP;
    case pds::EVENT_ID_PORT_DOWN:
        return PDS_EVENT_ID_PORT_DOWN;
    case pds::EVENT_ID_LIF_CREATE:
        return PDS_EVENT_ID_LIF_CREATE;
    case pds::EVENT_ID_LIF_UPDATE:
        return PDS_EVENT_ID_LIF_UPDATE;
    case pds::EVENT_ID_LIF_UP:
        return PDS_EVENT_ID_LIF_UP;
    case pds::EVENT_ID_LIF_DOWN:
        return PDS_EVENT_ID_LIF_DOWN;
    default:
        break;
    }
    return PDS_EVENT_ID_NONE;
}

static inline void
pds_learn_mackey_proto_to_api (learn::ep_mac_key_t *mac_key,
                               const pds::LearnMACKey &key)
{
    MAC_UINT64_TO_ADDR(mac_key->mac_addr, key.macaddr());
    pds_obj_key_proto_to_api_spec(&mac_key->subnet, key.subnetid());
}

static inline void
pds_learn_ipkey_proto_to_api (learn::ep_ip_key_t *ip_key,
                              const pds::LearnIPKey &key)
{
    ipaddr_proto_spec_to_api_spec(&ip_key->ip_addr, key.ipaddr());
    pds_obj_key_proto_to_api_spec(&ip_key->vpc, key.vpcid());
}

static inline pds::EpState
pds_learn_state_to_proto (learn::ep_state_t state)
{
    pds::EpState proto_state;

    switch (state) {
    case learn::EP_STATE_LEARNING:
        proto_state = pds::EP_STATE_LEARNING;
        break;
    case learn::EP_STATE_CREATED:
        proto_state = pds::EP_STATE_CREATED;
        break;
    case learn::EP_STATE_PROBING:
        proto_state = pds::EP_STATE_PROBING;
        break;
    case learn::EP_STATE_UPDATING:
        proto_state = pds::EP_STATE_UPDATING;
        break;
    case learn::EP_STATE_DELETING:
        proto_state =  pds::EP_STATE_DELETING;
        break;
    case learn::EP_STATE_DELETED:
        proto_state = pds::EP_STATE_DELETED;
        break;
    default:
        proto_state = pds::EP_STATE_NONE;
        break;
    }
    return proto_state;
}

static inline pds::LearnPktDropReason
pds_learn_pkt_drop_reason_to_proto (uint8_t reason)
{
    pds::LearnPktDropReason proto_reason;

    switch (reason) {
    case learn::PKT_DROP_REASON_PARSE_ERR:
        proto_reason = pds::LEARN_PKTDROP_REASON_PARSE_ERR;
        break;
    case learn::PKT_DROP_REASON_RES_ALLOC_FAIL:
        proto_reason = pds::LEARN_PKTDROP_REASON_RES_ALLOC_FAIL;
        break;
    case learn::PKT_DROP_REASON_LEARNING_FAIL:
        proto_reason = pds::LEARN_PKTDROP_REASON_LEARNING_FAIL;
        break;
    case learn::PKT_DROP_REASON_MBUF_ERR:
        proto_reason = pds::LEARN_PKTDROP_REASON_MBUF_ERR;
        break;
    case learn::PKT_DROP_REASON_TX_FAIL:
        proto_reason = pds::LEARN_PKTDROP_REASON_TX_FAIL;
        break;
    case learn::PKT_DROP_REASON_ARP_REPLY:
        proto_reason = pds::LEARN_PKTDROP_REASON_ARP_REPLY;
        break;
    default:
        proto_reason = pds::LEARN_PKTDROP_REASON_NONE;
        break;
    }
    return proto_reason;
}

static inline pds::LearnEventType
pds_ep_learn_type_to_proto (uint8_t learn_type)
{
    switch (learn_type) {
    case learn::LEARN_TYPE_NEW_LOCAL:
        return pds::LEARN_EVENT_NEW_LOCAL;
    case learn::LEARN_TYPE_NEW_REMOTE:
        return pds::LEARN_EVENT_NEW_REMOTE;
    case learn::LEARN_TYPE_MOVE_L2L:
        return pds::LEARN_EVENT_L2L_MOVE;
    case learn::LEARN_TYPE_MOVE_R2L:
        return pds::LEARN_EVENT_R2L_MOVE;
    case learn::LEARN_TYPE_MOVE_L2R:
        return pds::LEARN_EVENT_L2R_MOVE;
    case learn::LEARN_TYPE_MOVE_R2R:
        return pds::LEARN_EVENT_R2R_MOVE;
    case learn::LEARN_TYPE_DELETE:
        return pds::LEARN_EVENT_DELETE;
    default:
        break;
    }
    return pds::LEARN_EVENT_NONE;
}

static inline pds::LearnValidationType
pds_learn_validation_type_to_proto (uint8_t validation_type)
{
    switch (validation_type) {
    case learn::UNTAGGED_MAC_LIMIT:
        return pds::LEARN_CHECK_UNTAGGED_MAC_LIMIT;
    case MAC_LIMIT:
        return pds::LEARN_CHECK_MAC_LIMIT;
    case IP_LIMIT:
        return pds::LEARN_CHECK_IP_LIMIT;
    case IP_ADDR_SUBNET_MISMATCH:
        return pds::LEARN_CHECK_IP_IN_SUBNET;
    default:
        break;
    }
    return pds::LEARN_CHECK_NONE;
}

static inline pds::LearnApiOpType
pds_learn_api_op_to_proto (uint8_t op_type)
{
    switch (op_type) {
    case OP_CREATE:
        return pds::LEARN_API_OP_CREATE;
    case OP_DELETE:
        return pds::LEARN_API_OP_DELETE;
    case OP_UPDATE:
        return pds::LEARN_API_OP_UPDATE;
    default:
        break;
    }
    return pds::LEARN_API_OP_NONE;
}

#endif    // __AGENT_SVC_SPECS_HPP__
