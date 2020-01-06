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
#include "nic/utils/ftlite/ftlite_ipv4_structs.hpp"
#include "nic/utils/ftlite/ftlite_ipv6_structs.hpp"
#include "nic/sdk/platform/capri/capri_tm_utils.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
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
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/meter.hpp"
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
#include "gen/proto/types.pb.h"

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
static inline void
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
    }
    return;
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
    ipaddr_api_spec_to_proto_spec(out_ippfx->mutable_addr(), &in_ippfx->addr);
    return SDK_RET_OK;
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

static inline pds::IfType
pds_if_type_to_proto_if_type (pds_if_type_t type)
{
    switch (type) {
    case PDS_IF_TYPE_UPLINK:
        return pds::IF_TYPE_UPLINK;
    case PDS_IF_TYPE_UPLINK_PC:
        return pds::IF_TYPE_UPLINK_PC;
    case PDS_IF_TYPE_L3:
        return pds::IF_TYPE_L3;
    default:
        return pds::IF_TYPE_NONE;
    }
}

static inline pds::IfStatus
pds_admin_state_to_proto_admin_state (pds_if_state_t state)
{
    switch (state) {
    case PDS_IF_STATE_DOWN:
        return pds::IF_STATUS_DOWN;
    case PDS_IF_STATE_UP:
        return pds::IF_STATUS_UP;
    default:
        return pds::IF_STATUS_NONE;
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

static inline void
pds_if_api_spec_to_proto (pds::InterfaceSpec *proto_spec,
                          const pds_if_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_type(pds_if_type_to_proto_if_type(api_spec->type));
    proto_spec->set_adminstatus(
                pds_admin_state_to_proto_admin_state(api_spec->admin_state));

    switch (api_spec->type) {
    case PDS_IF_TYPE_UPLINK:
        {
            auto proto_uplink = proto_spec->mutable_uplinkspec();
            proto_uplink->set_portid(api_spec->uplink_info.port_num);
        }
        break;
    case PDS_IF_TYPE_L3:
        {
            auto proto_l3 = proto_spec->mutable_l3ifspec();
            proto_l3->set_vpcid(api_spec->l3_if_info.vpc.id);
            proto_l3->set_ethifindex(api_spec->l3_if_info.eth_ifindex);
            proto_l3->set_macaddress(
                      MAC_TO_UINT64(api_spec->l3_if_info.mac_addr));
            pds_encap_to_proto_encap(proto_l3->mutable_encap(),
                                     &api_spec->l3_if_info.encap);
            ippfx_api_spec_to_proto_spec(proto_l3->mutable_prefix(),
                                         &api_spec->l3_if_info.ip_prefix);
        }
        break;
    default:
        break;
    }
}

static inline void
pds_if_api_status_to_proto (pds::InterfaceStatus *proto_status,
                            const pds_if_status_t *api_status,
                            pds_if_type_t type)
{
    switch (type) {
    case PDS_IF_TYPE_UPLINK:
        {
            auto uplink_status = proto_status->mutable_uplinkstatus();
            uplink_status->set_lifid(api_status->uplink_status.lif_id);
        }
        break;
    default:
        break;
    }
}

static inline void
pds_if_api_stats_to_proto (pds::InterfaceStats *proto_stats,
                           const pds_if_stats_t *api_stats)
{
}

static inline void
pds_if_api_info_to_proto (void *entry, void *ctxt)
{
    pds::InterfaceGetResponse *proto_rsp = (pds::InterfaceGetResponse *)ctxt;
    auto intf = proto_rsp->add_response();
    pds_if_info_t *info = (pds_if_info_t *)entry;

    pds_if_api_spec_to_proto(intf->mutable_spec(), &info->spec);
    pds_if_api_status_to_proto(intf->mutable_status(), &info->status, info->spec.type);
    pds_if_api_stats_to_proto(intf->mutable_stats(), &info->stats);
}

static inline sdk_ret_t
pds_if_proto_to_api_spec (pds_if_spec_t *api_spec,
                          const pds::InterfaceSpec &proto_spec)
{
    if (proto_spec.type() != pds::IF_TYPE_L3) {
        return SDK_RET_INVALID_ARG;
    }
    if (IFINDEX_TO_IFTYPE(proto_spec.l3ifspec().ethifindex()) != IF_TYPE_ETH) {
        return SDK_RET_INVALID_ARG;
    }
    api_spec->key.id = proto_spec.id();
    api_spec->type = PDS_IF_TYPE_L3;
    api_spec->admin_state =
        proto_admin_state_to_pds_admin_state(proto_spec.adminstatus());

    api_spec->l3_if_info.vpc.id = proto_spec.l3ifspec().vpcid();
    api_spec->l3_if_info.eth_ifindex = proto_spec.l3ifspec().ethifindex();
    api_spec->l3_if_info.encap =
        proto_encap_to_pds_encap(proto_spec.l3ifspec().encap());
    MAC_UINT64_TO_ADDR(api_spec->l3_if_info.mac_addr,
                       proto_spec.l3ifspec().macaddress());
    ippfx_proto_spec_to_api_spec(&api_spec->l3_if_info.ip_prefix,
                                 proto_spec.l3ifspec().prefix());
    return SDK_RET_OK;
}

static inline bool
pds_slab_to_proto (void *entry, void *ctxt)
{
    pds::SlabGetResponse *rsp = (pds::SlabGetResponse *)ctxt;
    slab *s = (slab *)entry;

    auto rsp_entry = rsp->add_slab();
    auto spec = rsp_entry->mutable_spec();
    auto stats = rsp_entry->mutable_stats();

    spec->set_name(s->name());
    spec->set_id(s->slab_id());
    spec->set_elementsize(s->elem_sz());
    spec->set_elementsperblock(s->elems_per_block());
    spec->set_threadsafe(s->thread_safe());
    spec->set_growondemand(s->grow_on_demand());
    //spec->set_delaydelete(s->delay_delete());
    spec->set_zeroonallocation(s->zero_on_alloc());
    spec->set_rawblocksize(s->raw_block_sz());
    stats->set_numelementsinuse(s->num_in_use());
    stats->set_numallocs(s->num_allocs());
    stats->set_numfrees(s->num_frees());
    stats->set_numallocerrors(s->num_alloc_fails());
    stats->set_numblocks(s->num_blocks());

    return false;
}

static inline cli_cmd_t
pds_proto_cmd_to_api_cmd (pds::Command proto_cmd)
{
    switch (proto_cmd) {
    case pds::CMD_MAPPING_DUMP:
        return CLI_CMD_MAPPING_DUMP;
    case pds::CMD_INTR_DUMP:
        return CLI_CMD_INTR_DUMP;
    case pds::CMD_INTR_CLEAR:
        return CLI_CMD_INTR_CLEAR;
    case pds::CMD_API_ENGINE_STATS_DUMP:
        return CLI_CMD_API_ENGINE_STATS_DUMP;
    default:
        return CLI_CMD_MAX;
    }
}

static inline mapping_dump_type_t
proto_mapping_dump_type_to_pds (pds::MappingDumpType type)
{
    switch (type) {
    case pds::MAPPING_DUMP_LOCAL:
        return MAPPING_DUMP_TYPE_LOCAL;
    case pds::MAPPING_DUMP_REMOTE:
        return MAPPING_DUMP_TYPE_REMOTE;
    case pds::MAPPING_DUMP_ALL:
    default:
        return MAPPING_DUMP_TYPE_ALL;
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
        if (proto_ctxt->mappingdumpfilter().has_key()) {
            auto key = proto_ctxt->mappingdumpfilter().key();
            cmd_ctxt->args.mapping_dump.key_valid = true;
            cmd_ctxt->args.mapping_dump.key.vpc.id = key.ipkey().vpcid();
            ipaddr_proto_spec_to_api_spec(&cmd_ctxt->args.mapping_dump.key.ip_addr,
                                          key.ipkey().ipaddr());
        }
        cmd_ctxt->args.mapping_dump.type =
                proto_mapping_dump_type_to_pds(
                proto_ctxt->mappingdumpfilter().type());
    }
}

static inline void
pds_queue_credits_to_proto (uint32_t port_num,
                            port_queue_credit_t *credit,
                            void *ctxt)
{
    QueueCreditsGetResponse *proto = (QueueCreditsGetResponse *)ctxt;
    auto port_credit = proto->add_portqueuecredit();
    port_credit->set_port(port_num);

    for (uint32_t j = 0; j < credit->num_queues; j++) {
        auto queue_credit = port_credit->add_queuecredit();
        queue_credit->set_queue(credit->queues[j].oq);
        queue_credit->set_credit(credit->queues[j].credit);
    }
}

// populate proto buf from lif api spec
static inline void
pds_lif_api_info_to_proto (void *entry, void *ctxt)
{
    auto rsp = ((pds::LifGetResponse *)ctxt)->add_response();
    auto proto_spec = rsp->mutable_spec();
    auto proto_status = rsp->mutable_status();
    pds_lif_info_t *api_info = (pds_lif_info_t *)entry;

    proto_spec->set_lifid(api_info->spec.key);
    proto_spec->set_pinnedinterfaceid(api_info->spec.pinned_ifidx);
    proto_spec->set_type(types::LifType(api_info->spec.type));
    proto_spec->set_macaddress(MAC_TO_UINT64(api_info->spec.mac));
    proto_status->set_name(api_info->status.name);
    proto_status->set_ifindex(api_info->status.ifindex);
    proto_status->set_operstatus(pds_admin_state_to_proto_admin_state(api_info->status.state));
}

static inline void
pds_meter_debug_stats_to_proto (pds_meter_debug_stats_t *stats, void *ctxt)
{
    pds::MeterStatsGetResponse *rsp = (pds::MeterStatsGetResponse *)ctxt;
    auto proto_stats = rsp->add_stats();

    proto_stats->set_statsindex(stats->idx);
    proto_stats->set_rxbytes(stats->rx_bytes);
    proto_stats->set_txbytes(stats->tx_bytes);
}

static inline pds::PolicerDir
pds_policer_dir_api_spec_to_proto (pds_policer_dir_t dir)
{
    switch (dir) {
    case PDS_POLICER_DIR_INGRESS:
        return pds::POLICER_DIR_INGRESS;
    case PDS_POLICER_DIR_EGRESS:
        return pds::POLICER_DIR_EGRESS;
    default:
        return pds::POLICER_DIR_NONE;
    }
}

static inline pds_policer_dir_t
pds_policer_dir_proto_to_api_spec (pds::PolicerDir dir)
{
    switch (dir) {
    case pds::POLICER_DIR_INGRESS:
        return PDS_POLICER_DIR_INGRESS;
    case pds::POLICER_DIR_EGRESS:
        return PDS_POLICER_DIR_EGRESS;
    default:
        return PDS_POLICER_DIR_NONE;
    }
}

// build policer api spec from proto buf spec
static inline sdk_ret_t
pds_policer_proto_to_api_spec (pds_policer_spec_t *api_spec,
                               const pds::PolicerSpec &proto_spec)
{
    api_spec->key.id = proto_spec.id();
    api_spec->dir = pds_policer_dir_proto_to_api_spec(proto_spec.direction());
    if (proto_spec.has_ppspolicer()) {
        api_spec->type = sdk::POLICER_TYPE_PPS;
        api_spec->pps = proto_spec.ppspolicer().packetspersecond();
        api_spec->pps_burst = proto_spec.ppspolicer().burst();
    } else if (proto_spec.has_bpspolicer()) {
        api_spec->type = sdk::POLICER_TYPE_BPS;
        api_spec->bps = proto_spec.bpspolicer().bytespersecond();
        api_spec->bps_burst = proto_spec.bpspolicer().burst();
    }
    return SDK_RET_OK;
}

// populate proto buf spec from policer API spec
static inline void
pds_policer_api_spec_to_proto (pds::PolicerSpec *proto_spec,
                               const pds_policer_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_direction(pds_policer_dir_api_spec_to_proto(api_spec->dir));
    switch (api_spec->type) {
    case sdk::POLICER_TYPE_PPS:
        {
            auto pps = proto_spec->mutable_ppspolicer();
            pps->set_packetspersecond(api_spec->pps);
            pps->set_burst(api_spec->pps_burst);
        }
        break;
    case sdk::POLICER_TYPE_BPS:
        {
            auto bps = proto_spec->mutable_bpspolicer();
            bps->set_bytespersecond(api_spec->bps);
            bps->set_burst(api_spec->bps_burst);
        }
        break;
    default:
        break;
    }
}

// populate proto buf status from policer API status
static inline void
pds_policer_api_status_to_proto (pds::PolicerStatus *proto_status,
                                 const pds_policer_status_t *api_status)
{
}

// populate proto buf stats from policer API stats
static inline void
pds_policer_api_stats_to_proto (pds::PolicerStats *proto_stats,
                                const pds_policer_stats_t *api_stats)
{
}

// populate proto buf from policer API info
static inline void
pds_policer_api_info_to_proto (pds_policer_info_t *api_info, void *ctxt)
{
    pds::PolicerGetResponse *proto_rsp = (pds::PolicerGetResponse *)ctxt;
    auto policer = proto_rsp->add_response();
    pds::PolicerSpec *proto_spec = policer->mutable_spec();
    pds::PolicerStatus *proto_status = policer->mutable_status();
    pds::PolicerStats *proto_stats = policer->mutable_stats();

    pds_policer_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_policer_api_status_to_proto(proto_status, &api_info->status);
    pds_policer_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build Meter api spec from proto buf spec
static inline sdk_ret_t
pds_meter_proto_to_api_spec (pds_meter_spec_t *api_spec,
                             const pds::MeterSpec &proto_spec)
{
    sdk_ret_t ret;

    api_spec->key.id = proto_spec.id();
    ret = pds_af_proto_spec_to_api_spec(&api_spec->af, proto_spec.af());
    if (ret != SDK_RET_OK) {
        return SDK_RET_INVALID_ARG;
    }

    api_spec->num_rules = proto_spec.rules_size();
    api_spec->rules =
        (pds_meter_rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                        api_spec->num_rules * sizeof(pds_meter_rule_t));
    for (uint32_t rule = 0; rule < api_spec->num_rules; rule++) {
        const pds::MeterRuleSpec &proto_rule_spec = proto_spec.rules(rule);
        pds_meter_rule_t *api_rule_spec = &api_spec->rules[rule];
        if (proto_rule_spec.has_ppspolicer()) {
            api_rule_spec->type = PDS_METER_TYPE_PPS_POLICER;
            api_rule_spec->pps =
                            proto_rule_spec.ppspolicer().packetspersecond();
            api_rule_spec->pkt_burst = proto_rule_spec.ppspolicer().burst();
        } else if (proto_rule_spec.has_bpspolicer()) {
            api_rule_spec->type = PDS_METER_TYPE_BPS_POLICER;
            api_rule_spec->bps =
                            proto_rule_spec.bpspolicer().bytespersecond();
            api_rule_spec->byte_burst = proto_rule_spec.bpspolicer().burst();
        } else {
            api_rule_spec->type = PDS_METER_TYPE_ACCOUNTING;
        }

        api_rule_spec->priority = proto_rule_spec.priority();
        api_rule_spec->num_prefixes = proto_rule_spec.prefix_size();
        api_rule_spec->prefixes =
                 (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                            api_rule_spec->num_prefixes * sizeof(ip_prefix_t));
        for (int pfx = 0; pfx < proto_rule_spec.prefix_size(); pfx++) {
            ippfx_proto_spec_to_api_spec(
                    &api_rule_spec->prefixes[pfx], proto_rule_spec.prefix(pfx));
        }
    }
    return SDK_RET_OK;
}

// populate proto buf spec from meter API spec
static inline void
pds_meter_api_spec_to_proto (pds::MeterSpec *proto_spec,
                             const pds_meter_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id);
    if (api_spec->af == IP_AF_IPV4) {
        proto_spec->set_af(types::IP_AF_INET);
    } else if (api_spec->af == IP_AF_IPV6) {
        proto_spec->set_af(types::IP_AF_INET6);
    } else {
        SDK_ASSERT(FALSE);
    }
    for (uint32_t rule = 0; rule < api_spec->num_rules; rule++) {
        pds::MeterRuleSpec *proto_rule_spec = proto_spec->add_rules();
        pds_meter_rule_t *api_rule_spec = &api_spec->rules[rule];
        switch (api_rule_spec->type) {
        case pds_meter_type_t::PDS_METER_TYPE_PPS_POLICER:
            proto_rule_spec->mutable_ppspolicer()->set_packetspersecond(
                                            api_rule_spec->pps);
            proto_rule_spec->mutable_ppspolicer()->set_burst(
                                            api_rule_spec->pkt_burst);
            break;

        case pds_meter_type_t::PDS_METER_TYPE_BPS_POLICER:
            proto_rule_spec->mutable_bpspolicer()->set_bytespersecond(
                                            api_rule_spec->bps);
            proto_rule_spec->mutable_bpspolicer()->set_burst(
                                            api_rule_spec->byte_burst);
            break;

        case pds_meter_type_t::PDS_METER_TYPE_ACCOUNTING:
            break;

        default:
            break;
        }
        proto_rule_spec->set_priority(api_rule_spec->priority);
        for (uint32_t pfx = 0;
                  pfx < api_rule_spec->num_prefixes; pfx++) {
            ippfx_api_spec_to_proto_spec(proto_rule_spec->add_prefix(),
                           &api_rule_spec->prefixes[pfx]);
        }
    }
    return;
}

// populate proto buf status from meter API status
static inline void
pds_meter_api_status_to_proto (pds::MeterStatus *proto_status,
                               const pds_meter_status_t *api_status)
{
}

// populate proto buf stats from meter API stats
static inline void
pds_meter_api_stats_to_proto (pds::MeterStats *proto_stats,
                              const pds_meter_stats_t *api_stats)
{
    proto_stats->set_meterid(api_stats->idx);
    proto_stats->set_rxbytes(api_stats->rx_bytes);
    proto_stats->set_txbytes(api_stats->tx_bytes);
}

// populate proto buf from meter API info
static inline void
pds_meter_api_info_to_proto (const pds_meter_info_t *api_info, void *ctxt)
{
    pds::MeterGetResponse *proto_rsp = (pds::MeterGetResponse *)ctxt;
    auto meter = proto_rsp->add_response();
    //pds::MeterSpec *proto_spec = meter->mutable_spec();
    pds::MeterStatus *proto_status = meter->mutable_status();
    pds::MeterStats *proto_stats = meter->mutable_stats();

    //pds_meter_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_meter_api_status_to_proto(proto_status, &api_info->status);
    pds_meter_api_stats_to_proto(proto_stats, &api_info->stats);
}

// populate proto buf spec from tag API spec
static inline void
pds_tag_api_spec_to_proto (pds::TagSpec *proto_spec,
                           const pds_tag_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_af(pds_af_api_spec_to_proto_spec(api_spec->af));
#if 0 // We don't store rules in db for now
    for (uint32_t i = 0; i < api_spec->num_rules; i ++) {
        auto rule = proto_spec->add_rules();
        rule->set_tag(api_spec->rules[i].tag);
        rule->set_priority(api_spec->rules[i].priority);
        for (uint32_t j = 0; j < api_spec->rules[i].num_prefixes; j ++) {
            ippfx_api_spec_to_proto_spec(
                    rule->add_prefix(), &api_spec->rules[i].prefixes[j]);
        }
    }
#endif
}

// populate proto buf status from tag API status
static inline void
pds_tag_api_status_to_proto (const pds_tag_status_t *api_status,
                             pds::TagStatus *proto_status)
{
}

// populate proto buf stats from tag API stats
static inline void
pds_tag_api_stats_to_proto (const pds_tag_stats_t *api_stats,
                            pds::TagStats *proto_stats)
{
}

// populate proto buf from tag API info
static inline void
pds_tag_api_info_to_proto (const pds_tag_info_t *api_info, void *ctxt)
{
    pds::TagGetResponse *proto_rsp = (pds::TagGetResponse *)ctxt;
    auto tag = proto_rsp->add_response();
    pds::TagSpec *proto_spec = tag->mutable_spec();
    pds::TagStatus *proto_status = tag->mutable_status();
    pds::TagStats *proto_stats = tag->mutable_stats();

    pds_tag_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_tag_api_status_to_proto(&api_info->status, proto_status);
    pds_tag_api_stats_to_proto(&api_info->stats, proto_stats);
}

// build Tag api spec from proto buf spec
static inline sdk_ret_t
pds_tag_proto_to_api_spec (pds_tag_spec_t *api_spec,
                           const pds::TagSpec &proto_spec)
{
    sdk_ret_t ret;

    api_spec->key.id = proto_spec.id();
    ret = pds_af_proto_spec_to_api_spec(&api_spec->af, proto_spec.af());
    if (ret != SDK_RET_OK) {
        return SDK_RET_INVALID_ARG;
    }

    api_spec->num_rules = proto_spec.rules_size();
    api_spec->rules = (pds_tag_rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_TAG,
                                                   api_spec->num_rules *
                                                   sizeof(pds_tag_rule_t));
    for (int i = 0; i < proto_spec.rules_size(); i ++) {
        auto proto_rule = proto_spec.rules(i);

        api_spec->rules[i].tag = proto_rule.tag();
        api_spec->rules[i].priority = proto_rule.priority();
        api_spec->rules[i].num_prefixes = proto_rule.prefix_size();
        api_spec->rules[i].prefixes =
            (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_TAG,
                                      api_spec->rules[i].num_prefixes *
                                          sizeof(ip_prefix_t));
        for (int j = 0; j < proto_rule.prefix_size(); j ++) {
            ippfx_proto_spec_to_api_spec(
                        &api_spec->rules[i].prefixes[j], proto_rule.prefix(j));
        }
    }

    return SDK_RET_OK;
}

// build VNIC api spec from proto buf spec
static inline sdk_ret_t
pds_vnic_proto_to_api_spec (pds_vnic_spec_t *api_spec,
                            const pds::VnicSpec &proto_spec)
{
    uint32_t msid;

    api_spec->key.id = proto_spec.vnicid();
    if (proto_spec.hostname().empty()) {
        api_spec->hostname[0] = '\0';
    } else {
        strncpy(api_spec->hostname, proto_spec.hostname().c_str(),
                PDS_MAX_HOST_NAME_LEN);
         api_spec->hostname[PDS_MAX_HOST_NAME_LEN] = '\0';
    }
    api_spec->subnet.id = proto_spec.subnetid();
    api_spec->vnic_encap = proto_encap_to_pds_encap(proto_spec.vnicencap());
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.fabricencap());
    MAC_UINT64_TO_ADDR(api_spec->mac_addr, proto_spec.macaddress());
    api_spec->src_dst_check = proto_spec.sourceguardenable();
    for (int i = 0; i < proto_spec.txmirrorsessionid_size(); i++) {
        msid = proto_spec.txmirrorsessionid(i);
        if ((msid < 1) || (msid > 8)) {
            PDS_TRACE_ERR("Invalid tx mirror session id {} in vnic {} spec, "
                          "mirror session ids must be in the range [1-8]",
                          msid, api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        api_spec->tx_mirror_session_bmap |= (1 << (msid - 1));
    }
    for (int i = 0; i < proto_spec.rxmirrorsessionid_size(); i++) {
        msid = proto_spec.rxmirrorsessionid(i);
        if ((msid < 1) || (msid > 8)) {
            PDS_TRACE_ERR("Invalid rx mirror session id {} in vnic {} spec",
                          "mirror session ids must be in the range [1-8]",
                          msid, api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        api_spec->rx_mirror_session_bmap |= (1 << (msid - 1));
    }
    api_spec->v4_meter.id = proto_spec.v4meterid();
    api_spec->v6_meter.id = proto_spec.v6meterid();
    api_spec->switch_vnic = proto_spec.switchvnic();
    if (proto_spec.ingv4securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv4 ingress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_ing_v4_policy = proto_spec.ingv4securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_ing_v4_policy; i++) {
        api_spec->ing_v4_policy[i].id = proto_spec.ingv4securitypolicyid(i);
    }
    if (proto_spec.ingv6securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv6 ingress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_ing_v6_policy = proto_spec.ingv6securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_ing_v6_policy; i++) {
        api_spec->ing_v6_policy[i].id = proto_spec.ingv6securitypolicyid(i);
    }
    if (proto_spec.egv4securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv4 egress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_egr_v4_policy = proto_spec.egv4securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_egr_v4_policy; i++) {
        api_spec->egr_v4_policy[i].id = proto_spec.egv4securitypolicyid(i);
    }
    if (proto_spec.egv6securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv6 egress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_egr_v6_policy = proto_spec.egv6securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_egr_v6_policy; i++) {
        api_spec->egr_v6_policy[i].id = proto_spec.egv6securitypolicyid(i);
    }
    api_spec->host_ifindex = proto_spec.hostifindex();
    api_spec->primary = proto_spec.primary();
    api_spec->max_sessions = proto_spec.maxsessions();
    api_spec->flow_learn_en = proto_spec.flowlearnen();
    api_spec->tx_policer.id = proto_spec.txpolicerid();
    api_spec->rx_policer.id = proto_spec.rxpolicerid();
    return SDK_RET_OK;
}

// populate proto buf spec from vnic API spec
static inline void
pds_vnic_api_spec_to_proto (pds::VnicSpec *proto_spec,
                            const pds_vnic_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_vnicid(api_spec->key.id);
    proto_spec->set_hostname(api_spec->hostname);
    proto_spec->set_subnetid(api_spec->subnet.id);
    pds_encap_to_proto_encap(proto_spec->mutable_vnicencap(),
                             &api_spec->vnic_encap);
    proto_spec->set_macaddress(MAC_TO_UINT64(api_spec->mac_addr));
    //proto_spec->set_providermacaddress(
                        //MAC_TO_UINT64(api_spec->provider_mac_addr));
    pds_encap_to_proto_encap(proto_spec->mutable_fabricencap(),
                             &api_spec->fabric_encap);
    proto_spec->set_sourceguardenable(api_spec->src_dst_check);
    proto_spec->set_v4meterid(api_spec->v4_meter.id);
    proto_spec->set_v6meterid(api_spec->v6_meter.id);
    if (api_spec->tx_mirror_session_bmap) {
        for (uint8_t i = 0; i < 8; i++) {
            if (api_spec->tx_mirror_session_bmap & (1 << i)) {
                proto_spec->add_txmirrorsessionid(i + 1);
            }
        }
    }
    if (api_spec->rx_mirror_session_bmap) {
        for (uint8_t i = 0; i < 8; i++) {
            if (api_spec->rx_mirror_session_bmap & (1 << i)) {
                proto_spec->add_rxmirrorsessionid(i + 1);
            }
        }
    }
    proto_spec->set_switchvnic(api_spec->switch_vnic);
    for (uint8_t i = 0; i < api_spec->num_ing_v4_policy; i++) {
        proto_spec->add_ingv4securitypolicyid(api_spec->ing_v4_policy[i].id);
    }
    for (uint8_t i = 0; i < api_spec->num_ing_v6_policy; i++) {
        proto_spec->add_ingv6securitypolicyid(api_spec->ing_v6_policy[i].id);
    }
    for (uint8_t i = 0; i < api_spec->num_egr_v4_policy; i++) {
        proto_spec->add_egv4securitypolicyid(api_spec->egr_v4_policy[i].id);
    }
    for (uint8_t i = 0; i < api_spec->num_egr_v6_policy; i++) {
        proto_spec->add_egv6securitypolicyid(api_spec->egr_v6_policy[i].id);
    }
    proto_spec->set_hostifindex(api_spec->host_ifindex);
    proto_spec->set_primary(api_spec->primary);
    proto_spec->set_maxsessions(api_spec->max_sessions);
    proto_spec->set_flowlearnen(api_spec->flow_learn_en);
    proto_spec->set_txpolicerid(api_spec->tx_policer.id);
    proto_spec->set_rxpolicerid(api_spec->rx_policer.id);
}

// populate proto buf status from vnic API status
static inline void
pds_vnic_api_status_to_proto (pds::VnicStatus *proto_status,
                              const pds_vnic_status_t *api_status)
{
    proto_status->set_hwid(api_status->hw_id);
}

// populate proto buf stats from vnic API stats
static inline void
pds_vnic_api_stats_to_proto (pds::VnicStats *proto_stats,
                             const pds_vnic_stats_t *api_stats)
{
}

// populate proto buf from vnic API info
static inline void
pds_vnic_api_info_to_proto (const pds_vnic_info_t *api_info, void *ctxt)
{
    pds::VnicGetResponse *proto_rsp = (pds::VnicGetResponse *)ctxt;
    auto vnic = proto_rsp->add_response();
    pds::VnicSpec *proto_spec = vnic->mutable_spec();
    pds::VnicStatus *proto_status = vnic->mutable_status();
    pds::VnicStats *proto_stats = vnic->mutable_stats();

    pds_vnic_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_vnic_api_status_to_proto(proto_status, &api_info->status);
    pds_vnic_api_stats_to_proto(proto_stats, &api_info->stats);
}

// populate VPCPeer protobuf spec from VPCPeer API spec
static inline void
pds_vpc_peer_api_spec_to_proto (pds::VPCPeerSpec *proto_spec,
                                const pds_vpc_peer_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_vpc1(api_spec->vpc1.id);
    proto_spec->set_vpc2(api_spec->vpc2.id);
}

// populate proto buf status from vpc API status
static inline void
pds_vpc_peer_api_status_to_proto (pds::VPCPeerStatus *proto_status,
                                  const pds_vpc_peer_status_t *api_status)
{
}

// populate proto buf stats from vpc API stats
static inline void
pds_vpc_peer_api_stats_to_proto (pds::VPCPeerStats *proto_stats,
                                 const pds_vpc_peer_stats_t *api_stats)
{
}

// populate proto buf from vpc API info
static inline void
pds_vpc_peer_api_info_to_proto (const pds_vpc_peer_info_t *api_info, void *ctxt)
{
    pds::VPCPeerGetResponse *proto_rsp = (pds::VPCPeerGetResponse *)ctxt;
    auto vpc = proto_rsp->add_response();
    pds::VPCPeerSpec *proto_spec = vpc->mutable_spec();
    pds::VPCPeerStatus *proto_status = vpc->mutable_status();
    pds::VPCPeerStats *proto_stats = vpc->mutable_stats();

    pds_vpc_peer_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_vpc_peer_api_status_to_proto(proto_status, &api_info->status);
    pds_vpc_peer_api_stats_to_proto(proto_stats, &api_info->stats);
}

// populate VPCPeer api spec from VPCPeer proto spec
static inline void
pds_vpc_peer_proto_to_api_spec (pds_vpc_peer_spec_t *api_spec,
                                const pds::VPCPeerSpec &proto_spec)
{
    if (!api_spec) {
        return;
    }
    api_spec->key.id = proto_spec.id();
    api_spec->vpc1.id = proto_spec.vpc1();
    api_spec->vpc2.id = proto_spec.vpc2();
}

// populate proto buf spec from tep API spec
static inline sdk_ret_t
pds_tep_api_spec_to_proto (pds::TunnelSpec *proto_spec,
                           const pds_tep_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return SDK_RET_INVALID_ARG;
    }
    proto_spec->set_id(api_spec->key.id);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_remoteip(),
                                  &api_spec->remote_ip);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_localip(),
                                  &api_spec->ip_addr);
    proto_spec->set_macaddress(MAC_TO_UINT64(api_spec->mac));
    pds_encap_to_proto_encap(proto_spec->mutable_encap(),
                             &api_spec->encap);
    proto_spec->set_nat(api_spec->nat);
    proto_spec->set_vpcid(api_spec->vpc.id);
    switch (api_spec->type) {
    case PDS_TEP_TYPE_WORKLOAD:
        proto_spec->set_type(pds::TUNNEL_TYPE_WORKLOAD);
        break;
    case PDS_TEP_TYPE_IGW:
        proto_spec->set_type(pds::TUNNEL_TYPE_IGW);
        break;
    case PDS_TEP_TYPE_INTER_DC:
        proto_spec->set_type(pds::TUNNEL_TYPE_INTER_DC);
        break;
    case PDS_TEP_TYPE_SERVICE:
        proto_spec->set_type(pds::TUNNEL_TYPE_SERVICE);
        break;
    case PDS_TEP_TYPE_NONE:
    default:
        proto_spec->set_type(pds::TUNNEL_TYPE_NONE);
        break;
    }

    proto_spec->set_remoteservice(api_spec->remote_svc);
    if (api_spec->remote_svc) {
        pds_encap_to_proto_encap(proto_spec->mutable_remoteserviceencap(),
                                 &api_spec->remote_svc_encap);
        ipaddr_api_spec_to_proto_spec(
            proto_spec->mutable_remoteservicepublicip(),
            &api_spec->remote_svc_public_ip);
    }
    switch (api_spec->nh_type) {
    case PDS_NH_TYPE_UNDERLAY:
        proto_spec->set_nexthopid(api_spec->nh.id);
        break;
    case PDS_NH_TYPE_UNDERLAY_ECMP:
        proto_spec->set_nexthopid(api_spec->nh_group.id);
        break;
    case PDS_NH_TYPE_OVERLAY:
        proto_spec->set_tunnelid(api_spec->tep.id);
        break;
    default:
        //PDS_TRACE_ERR("Unsupported nexthop type {} in TEP {} spec",
                      //api_spec->nh_type, api_spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

// populate proto buf status from tep API status
static inline void
pds_tep_api_status_to_proto (pds::TunnelStatus *proto_status,
                             const pds_tep_status_t *api_status)
{
}

// populate proto buf stats from tep API stats
static inline void
pds_tep_api_stats_to_proto (pds::TunnelStats *proto_stats,
                            const pds_tep_stats_t *api_stats)
{
}

// populate proto buf from tep API info
static inline void
pds_tep_api_info_to_proto (const pds_tep_info_t *api_info, void *ctxt)
{
    pds::TunnelGetResponse *proto_rsp = (pds::TunnelGetResponse *)ctxt;
    auto tep = proto_rsp->add_response();
    pds::TunnelSpec *proto_spec = tep->mutable_spec();
    pds::TunnelStatus *proto_status = tep->mutable_status();
    pds::TunnelStats *proto_stats = tep->mutable_stats();

    pds_tep_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_tep_api_status_to_proto(proto_status, &api_info->status);
    pds_tep_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build TEP API spec from protobuf spec
static inline sdk_ret_t
pds_tep_proto_to_api_spec (pds_tep_spec_t *api_spec,
                           const pds::TunnelSpec &proto_spec)
{
    memset(api_spec, 0, sizeof(pds_tep_spec_t));
    api_spec->key.id = proto_spec.id();
    ipaddr_proto_spec_to_api_spec(&api_spec->remote_ip,
                                  proto_spec.remoteip());
    ipaddr_proto_spec_to_api_spec(&api_spec->ip_addr,
                                  proto_spec.localip());
    MAC_UINT64_TO_ADDR(api_spec->mac, proto_spec.macaddress());
    api_spec->vpc.id = proto_spec.vpcid();

    switch (proto_spec.type()) {
    case pds::TUNNEL_TYPE_IGW:
        api_spec->type = PDS_TEP_TYPE_IGW;
        break;
    case pds::TUNNEL_TYPE_WORKLOAD:
        api_spec->type = PDS_TEP_TYPE_WORKLOAD;
        break;
    case pds::TUNNEL_TYPE_INTER_DC:
        api_spec->type = PDS_TEP_TYPE_INTER_DC;
        break;
    case pds::TUNNEL_TYPE_SERVICE:
        api_spec->type = PDS_TEP_TYPE_SERVICE;
        break;
    default:
        api_spec->type = PDS_TEP_TYPE_NONE;
        break;
    }
    api_spec->encap = proto_encap_to_pds_encap(proto_spec.encap());
    api_spec->nat = proto_spec.nat();
    api_spec->remote_svc = proto_spec.remoteservice();
    if (api_spec->remote_svc) {
        api_spec->remote_svc_encap =
            proto_encap_to_pds_encap(proto_spec.remoteserviceencap());
        ipaddr_proto_spec_to_api_spec(&api_spec->remote_svc_public_ip,
                                      proto_spec.remoteservicepublicip());
    }
    switch (proto_spec.nh_case()) {
    case pds::TunnelSpec::kNexthopId:
        api_spec->nh_type = PDS_NH_TYPE_UNDERLAY;
        api_spec->nh.id = proto_spec.nexthopid();
        break;
    case pds::TunnelSpec::kNexthopGroupId:
        api_spec->nh_type = PDS_NH_TYPE_UNDERLAY_ECMP;
        api_spec->nh_group.id = proto_spec.nexthopgroupid();
        break;
    case pds::TunnelSpec::kTunnelId:
        api_spec->nh_type = PDS_NH_TYPE_OVERLAY;
        api_spec->tep.id = proto_spec.tunnelid();
        break;
    default:
        PDS_TRACE_ERR("Unsupported nexthop type {} in TEP {} spec",
                      proto_spec.nh_case(), api_spec->key.id);
        return SDK_RET_INVALID_ARG;
        break;
    }
    return SDK_RET_OK;
}

// populate proto buf spec from service API spec
static inline void
pds_service_api_spec_to_proto (pds::SvcMappingSpec *proto_spec,
                               const pds_svc_mapping_spec_t *api_spec)
{
    if (!proto_spec || !api_spec) {
        return;
    }

    auto proto_key = proto_spec->mutable_key();
    proto_key->set_vpcid(api_spec->key.vpc.id);
    proto_key->set_backendport(api_spec->key.backend_port);
    ipaddr_api_spec_to_proto_spec(
                proto_key->mutable_backendip(), &api_spec->key.backend_ip);
    ipaddr_api_spec_to_proto_spec(
                proto_spec->mutable_ipaddr(), &api_spec->vip);
    ipaddr_api_spec_to_proto_spec(
                proto_spec->mutable_providerip(),
                &api_spec->backend_provider_ip);
    proto_spec->set_svcport(api_spec->svc_port);
}

// populate proto buf status from service API status
static inline void
pds_service_api_status_to_proto (pds::SvcMappingStatus *proto_status,
                                 const pds_svc_mapping_status_t *api_status)
{
}

// populate proto buf stats from service API stats
static inline void
pds_service_api_stats_to_proto (pds::SvcMappingStats *proto_stats,
                                const pds_svc_mapping_stats_t *api_stats)
{
}

// populate proto buf from service API info
static inline void
pds_service_api_info_to_proto (const pds_svc_mapping_info_t *api_info,
                               void *ctxt)
{
    pds::SvcMappingGetResponse *proto_rsp = (pds::SvcMappingGetResponse *)ctxt;
    auto service = proto_rsp->add_response();
    pds::SvcMappingSpec *proto_spec = service->mutable_spec();
    pds::SvcMappingStatus *proto_status = service->mutable_status();
    pds::SvcMappingStats *proto_stats = service->mutable_stats();

    pds_service_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_service_api_status_to_proto(proto_status, &api_info->status);
    pds_service_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build service API spec from proto buf spec
static inline void
pds_service_proto_to_api_spec (pds_svc_mapping_spec_t *api_spec,
                               const pds::SvcMappingSpec &proto_spec)
{
    api_spec->key.vpc.id = proto_spec.key().vpcid();
    api_spec->key.backend_port = proto_spec.key().backendport();
    ipaddr_proto_spec_to_api_spec(&api_spec->key.backend_ip,
                                  proto_spec.key().backendip());
    ipaddr_proto_spec_to_api_spec(&api_spec->vip,
                                  proto_spec.ipaddr());
    ipaddr_proto_spec_to_api_spec(&api_spec->backend_provider_ip,
                                  proto_spec.providerip());
    api_spec->svc_port = proto_spec.svcport();
}

static inline port_fec_type_t
pds_port_proto_fec_type_to_sdk_fec_type (pds::PortFecType proto_fec_type)
{
    switch (proto_fec_type) {
    case pds::PORT_FEC_TYPE_NONE:
        return port_fec_type_t::PORT_FEC_TYPE_NONE;
    case pds::PORT_FEC_TYPE_FC:
        return port_fec_type_t::PORT_FEC_TYPE_FC;
    case pds::PORT_FEC_TYPE_RS:
        return port_fec_type_t::PORT_FEC_TYPE_RS;
    default:
        return port_fec_type_t::PORT_FEC_TYPE_NONE;
    }
}

static inline pds::PortFecType
pds_port_sdk_fec_type_to_proto_fec_type (port_fec_type_t sdk_fec_type)
{
    switch (sdk_fec_type) {
    case port_fec_type_t::PORT_FEC_TYPE_FC:
        return pds::PORT_FEC_TYPE_FC;
    case port_fec_type_t::PORT_FEC_TYPE_RS:
        return pds::PORT_FEC_TYPE_RS;
    default:
        return pds::PORT_FEC_TYPE_NONE;
    }
}

static inline port_speed_t
pds_port_proto_speed_to_sdk_speed (pds::PortSpeed proto_port_speed)
{
    switch (proto_port_speed) {
    case pds::PORT_SPEED_NONE:
        return port_speed_t::PORT_SPEED_NONE;
    case pds::PORT_SPEED_1G:
        return port_speed_t::PORT_SPEED_1G;
    case pds::PORT_SPEED_10G:
        return port_speed_t::PORT_SPEED_10G;
    case pds::PORT_SPEED_25G:
        return port_speed_t::PORT_SPEED_25G;
    case pds::PORT_SPEED_40G:
        return port_speed_t::PORT_SPEED_40G;
    case pds::PORT_SPEED_50G:
        return port_speed_t::PORT_SPEED_50G;
    case pds::PORT_SPEED_100G:
        return port_speed_t::PORT_SPEED_100G;
    default:
        return port_speed_t::PORT_SPEED_NONE;
    }
}

static inline pds::PortSpeed
pds_port_sdk_speed_to_proto_speed (port_speed_t sdk_port_speed)
{
    switch (sdk_port_speed) {
    case port_speed_t::PORT_SPEED_1G:
        return pds::PORT_SPEED_1G;
    case port_speed_t::PORT_SPEED_10G:
        return pds::PORT_SPEED_10G;
    case port_speed_t::PORT_SPEED_25G:
        return pds::PORT_SPEED_25G;
    case port_speed_t::PORT_SPEED_40G:
        return pds::PORT_SPEED_40G;
    case port_speed_t::PORT_SPEED_50G:
        return pds::PORT_SPEED_50G;
    case port_speed_t::PORT_SPEED_100G:
        return pds::PORT_SPEED_100G;
    default:
        return pds::PORT_SPEED_NONE;
    }
}

static inline port_admin_state_t
pds_port_proto_admin_state_to_sdk_admin_state (
                            pds::PortAdminState proto_admin_state)
{
    switch (proto_admin_state) {
    case pds::PORT_ADMIN_STATE_NONE:
        return port_admin_state_t::PORT_ADMIN_STATE_NONE;
    case pds::PORT_ADMIN_STATE_DOWN:
        return port_admin_state_t::PORT_ADMIN_STATE_DOWN;
    case pds::PORT_ADMIN_STATE_UP:
        return port_admin_state_t::PORT_ADMIN_STATE_UP;
    default:
        return port_admin_state_t::PORT_ADMIN_STATE_NONE;
    }
}

static inline pds::PortAdminState
pds_port_sdk_admin_state_to_proto_admin_state (port_admin_state_t sdk_admin_state)
{
    switch (sdk_admin_state) {
    case port_admin_state_t::PORT_ADMIN_STATE_DOWN:
        return pds::PORT_ADMIN_STATE_DOWN;
    case port_admin_state_t::PORT_ADMIN_STATE_UP:
        return pds::PORT_ADMIN_STATE_UP;
    default:
        return pds::PORT_ADMIN_STATE_NONE;
    }
}

static inline port_pause_type_t
pds_port_proto_pause_type_to_sdk_pause_type (pds::PortPauseType proto_pause_type)
{
    switch(proto_pause_type) {
    case pds::PORT_PAUSE_TYPE_LINK:
        return port_pause_type_t::PORT_PAUSE_TYPE_LINK;
    case pds::PORT_PAUSE_TYPE_PFC:
        return port_pause_type_t::PORT_PAUSE_TYPE_PFC;
    default:
        return port_pause_type_t::PORT_PAUSE_TYPE_NONE;
    }
}

static inline pds::PortPauseType
pds_port_sdk_pause_type_to_proto_pause_type (port_pause_type_t sdk_pause_type)
{
    switch (sdk_pause_type) {
    case port_pause_type_t::PORT_PAUSE_TYPE_LINK:
        return pds::PORT_PAUSE_TYPE_LINK;
    case port_pause_type_t::PORT_PAUSE_TYPE_PFC:
        return pds::PORT_PAUSE_TYPE_PFC;
    default:
        return pds::PORT_PAUSE_TYPE_NONE;
    }
}

static inline port_loopback_mode_t
pds_port_proto_loopback_mode_to_sdk_loopback_mode (
                        pds::PortLoopBackMode proto_loopback_mode)
{
    switch(proto_loopback_mode) {
    case pds::PORT_LOOPBACK_MODE_MAC:
        return port_loopback_mode_t::PORT_LOOPBACK_MODE_MAC;
    case pds::PORT_LOOPBACK_MODE_PHY:
        return port_loopback_mode_t::PORT_LOOPBACK_MODE_PHY;
    default:
        return port_loopback_mode_t::PORT_LOOPBACK_MODE_NONE;
    }
}

static inline pds::PortLoopBackMode
pds_port_sdk_loopback_mode_to_proto_loopback_mode (
                        port_loopback_mode_t sdk_loopback_mode)
{
    switch (sdk_loopback_mode) {
    case port_loopback_mode_t::PORT_LOOPBACK_MODE_MAC:
        return pds::PORT_LOOPBACK_MODE_MAC;
    case port_loopback_mode_t::PORT_LOOPBACK_MODE_PHY:
        return pds::PORT_LOOPBACK_MODE_PHY;
    default:
        return pds::PORT_LOOPBACK_MODE_NONE;
    }
}

static inline void
pds_port_proto_to_port_args (port_args_t *port_args,
                              const pds::PortSpec &spec)
{
    port_args->user_admin_state = port_args->admin_state =
                pds_port_proto_admin_state_to_sdk_admin_state(spec.adminstate());
    port_args->port_speed = pds_port_proto_speed_to_sdk_speed(spec.speed());
    port_args->fec_type = pds_port_proto_fec_type_to_sdk_fec_type(spec.fectype());
    port_args->auto_neg_cfg = port_args->auto_neg_enable = spec.autonegen();
    port_args->debounce_time = spec.debouncetimeout();
    port_args->mtu = spec.mtu();
    port_args->pause =
                pds_port_proto_pause_type_to_sdk_pause_type(spec.pausetype());
    port_args->loopback_mode =
       pds_port_proto_loopback_mode_to_sdk_loopback_mode(spec.loopbackmode());
    port_args->num_lanes_cfg = port_args->num_lanes = spec.numlanes();
}

static inline pds::SecurityRuleAction
pds_rule_action_to_proto_action (rule_action_data_t *action_data)
{
    switch (action_data->fw_action.action) {
    case SECURITY_RULE_ACTION_ALLOW:
        return pds::SECURITY_RULE_ACTION_ALLOW;
    case SECURITY_RULE_ACTION_DENY:
        return pds::SECURITY_RULE_ACTION_DENY;
    default:
        return pds::SECURITY_RULE_ACTION_NONE;
    }
}

static inline fw_action_t
pds_proto_action_to_rule_action (pds::SecurityRuleAction action)
{
    switch (action) {
    case pds::SECURITY_RULE_ACTION_ALLOW:
        return SECURITY_RULE_ACTION_ALLOW;
    case pds::SECURITY_RULE_ACTION_DENY:
        return SECURITY_RULE_ACTION_DENY;
    default:
        return SECURITY_RULE_ACTION_DENY;
    }
}

static inline sdk_ret_t
pds_policy_dir_proto_to_api_spec (rule_dir_t *dir,
                                  const pds::SecurityPolicySpec &proto_spec)
{
    if (proto_spec.direction() == types::RULE_DIR_INGRESS) {
        *dir = RULE_DIR_INGRESS;
    } else if (proto_spec.direction() == types::RULE_DIR_EGRESS) {
        *dir = RULE_DIR_EGRESS;
    } else {
        PDS_TRACE_ERR("Invalid direction {} in policy spec {}",
                      proto_spec.direction(), proto_spec.id());
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_policy_rule_match_proto_to_api_spec (pds_policy_id_t policy_id,
                                         uint32_t rule_id, uint8_t af,
                                         rule_match_t *match,
                                         const pds::SecurityRule &proto_rule)
{
    if (unlikely(proto_rule.has_match() == false)) {
        PDS_TRACE_ERR("Security policy {}, rule {} has no match condition, "
                      "IP protocol is a mandatory match condition",
                      policy_id, rule_id);
        return SDK_RET_INVALID_ARG;
    }

    if (unlikely(proto_rule.match().has_l3match() == false)) {
        PDS_TRACE_ERR("Security policy {}, rule {} has no L3 match condition, "
                      "IP protocol is a mandatory match condition",
                      policy_id, rule_id);
        return SDK_RET_INVALID_ARG;
    }

    const types::RuleMatch& proto_match = proto_rule.match();
    const types::RuleL3Match& proto_l3_match = proto_match.l3match();
    match->l3_match.ip_proto = proto_l3_match.protocol();
    if ((match->l3_match.ip_proto != IP_PROTO_UDP) &&
        (match->l3_match.ip_proto != IP_PROTO_TCP) &&
        (match->l3_match.ip_proto != IP_PROTO_ICMP) &&
        (match->l3_match.ip_proto != IP_PROTO_ICMPV6)) {
        PDS_TRACE_ERR("Security policy {}, rule {} with unsupported IP "
                      "protocol {}", policy_id, rule_id,
                      match->l3_match.ip_proto);
        return SDK_RET_INVALID_ARG;
    }
    if (proto_l3_match.has_srcprefix()) {
        match->l3_match.src_match_type = IP_MATCH_PREFIX;
        ippfx_proto_spec_to_api_spec(&match->l3_match.src_ip_pfx,
                                     proto_l3_match.srcprefix());
    } else if (proto_l3_match.has_srcrange()) {
        match->l3_match.src_match_type = IP_MATCH_RANGE;
        iprange_proto_spec_to_api_spec(&match->l3_match.src_ip_range,
                                       proto_l3_match.srcrange());
    } else if (proto_l3_match.srctag()) {
        match->l3_match.src_match_type = IP_MATCH_TAG;
        match->l3_match.src_tag = proto_l3_match.srctag();
    } else {
        // TODO: introduce IP_MATCH_NONE and clean this up
        match->l3_match.src_match_type = IP_MATCH_PREFIX;
        match->l3_match.src_ip_pfx.addr.af = af;
    }
    if (proto_l3_match.has_dstprefix()) {
        match->l3_match.dst_match_type = IP_MATCH_PREFIX;
        ippfx_proto_spec_to_api_spec(&match->l3_match.dst_ip_pfx,
                                     proto_l3_match.dstprefix());
    } else if (proto_l3_match.has_dstrange()) {
        match->l3_match.dst_match_type = IP_MATCH_RANGE;
        iprange_proto_spec_to_api_spec(&match->l3_match.dst_ip_range,
                                       proto_l3_match.dstrange());
    } else if (proto_l3_match.dsttag()) {
        match->l3_match.dst_match_type = IP_MATCH_TAG;
        match->l3_match.dst_tag = proto_l3_match.dsttag();
    } else {
        // TODO: introduce IP_MATCH_NONE and clean this up
        match->l3_match.dst_match_type = IP_MATCH_PREFIX;
        match->l3_match.dst_ip_pfx.addr.af = af;
    }

    if (proto_rule.match().has_l4match() &&
        (proto_rule.match().l4match().has_ports() ||
         proto_rule.match().l4match().has_typecode())) {
        const types::RuleL4Match& proto_l4_match = proto_match.l4match();
        if (proto_l4_match.has_ports()) {
            if ((match->l3_match.ip_proto != IP_PROTO_UDP) &&
                (match->l3_match.ip_proto != IP_PROTO_TCP)) {
                PDS_TRACE_ERR("Invalid port config in security policy {}, "
                              "rule {}", policy_id, rule_id);
                return SDK_RET_INVALID_ARG;
            }
            if (proto_l4_match.ports().has_srcportrange()) {
                const types::PortRange& sport_range =
                    proto_l4_match.ports().srcportrange();
                match->l4_match.sport_range.port_lo = sport_range.portlow();
                match->l4_match.sport_range.port_hi = sport_range.porthigh();
                if (unlikely(match->l4_match.sport_range.port_lo >
                             match->l4_match.sport_range.port_hi)) {
                    PDS_TRACE_ERR("Invalid src port range in security "
                                  "policy {}, rule {}", policy_id, rule_id);
                    return SDK_RET_INVALID_ARG;
                }
            } else {
                match->l4_match.sport_range.port_lo = 0;
                match->l4_match.sport_range.port_hi = 65535;
            }
            if (proto_l4_match.ports().has_dstportrange()) {
                const types::PortRange& dport_range =
                    proto_l4_match.ports().dstportrange();
                match->l4_match.dport_range.port_lo = dport_range.portlow();
                match->l4_match.dport_range.port_hi = dport_range.porthigh();
                if (unlikely(match->l4_match.dport_range.port_lo >
                             match->l4_match.dport_range.port_hi)) {
                    PDS_TRACE_ERR("Invalid dst port range in security "
                                  "policy {}, rule {}", policy_id, rule_id);
                    return SDK_RET_INVALID_ARG;
                }
            } else {
                match->l4_match.dport_range.port_lo = 0;
                match->l4_match.dport_range.port_hi = 65535;
            }
        } else if (proto_l4_match.has_typecode()) {
            if ((match->l3_match.ip_proto != IP_PROTO_ICMP) &&
                (match->l3_match.ip_proto != IP_PROTO_ICMPV6)) {
                PDS_TRACE_ERR("Invalid ICMP config in security policy {}, "
                              "rule {}", policy_id, rule_id);
                return SDK_RET_INVALID_ARG;
            }
            const types::ICMPMatch& typecode = proto_l4_match.typecode();
            match->l4_match.icmp_type = typecode.type();
            match->l4_match.icmp_code = typecode.code();
        }
    } else {
        // wildcard L4 match
        if ((match->l3_match.ip_proto == IP_PROTO_UDP) ||
            (match->l3_match.ip_proto == IP_PROTO_TCP)) {
            match->l4_match.sport_range.port_lo = 0;
            match->l4_match.sport_range.port_hi = 65535;
            match->l4_match.dport_range.port_lo = 0;
            match->l4_match.dport_range.port_hi = 65535;
        } else if ((match->l3_match.ip_proto == IP_PROTO_ICMP) ||
                   (match->l3_match.ip_proto == IP_PROTO_ICMPV6)) {
            // TODO : wildcard ICMP support will come later
        }
    }
    return SDK_RET_OK;
}

// build policy API spec from protobuf spec
static inline sdk_ret_t
pds_policy_proto_to_api_spec (pds_policy_spec_t *api_spec,
                              const pds::SecurityPolicySpec &proto_spec)
{
    uint32_t num_rules = 0;
    sdk_ret_t ret;

    api_spec->key.id = proto_spec.id();
    api_spec->policy_type = POLICY_TYPE_FIREWALL;
    ret = pds_af_proto_spec_to_api_spec(&api_spec->af, proto_spec.addrfamily());
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    ret = pds_policy_dir_proto_to_api_spec(&api_spec->direction, proto_spec);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    num_rules = proto_spec.rules_size();
    api_spec->num_rules = num_rules;
    api_spec->rules = (rule_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                           sizeof(rule_t) * num_rules);
    if (unlikely(api_spec->rules == NULL)) {
        PDS_TRACE_ERR("Failed to allocate memory for security policy {}",
                      api_spec->key.id);
        return SDK_RET_OOM;
    }
    for (uint32_t i = 0; i < num_rules; i++) {
        const pds::SecurityRule &proto_rule = proto_spec.rules(i);
        api_spec->rules[i].priority = proto_rule.priority();
        api_spec->rules[i].stateful = proto_rule.stateful();
        api_spec->rules[i].action_data.fw_action.action =
                                      pds_proto_action_to_rule_action(proto_rule.action());
        ret = pds_policy_rule_match_proto_to_api_spec(api_spec->key.id, i + 1,
                                                      api_spec->af,
                                                      &api_spec->rules[i].match,
                                                      proto_rule);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed converting policy {} spec, err {}",
                          api_spec->key.id, ret);
            goto cleanup;
        }
    }

    return SDK_RET_OK;

cleanup :

    if (api_spec->rules) {
        SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec->rules);
        api_spec->rules = NULL;
    }
    return ret;
}

// populate proto buf spec from policy API spec
inline void
pds_policy_api_spec_to_proto (pds::SecurityPolicySpec *proto_spec,
                              const pds_policy_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }

    proto_spec->set_id(api_spec->key.id);
    if (api_spec->af == IP_AF_IPV4) {
        proto_spec->set_addrfamily(types::IP_AF_INET);
    } else if (api_spec->af == IP_AF_IPV6) {
        proto_spec->set_addrfamily(types::IP_AF_INET6);
    } else {
        SDK_ASSERT(FALSE);
    }
    if (api_spec->direction == RULE_DIR_INGRESS) {
        proto_spec->set_direction(types::RULE_DIR_INGRESS);
    } else if (api_spec->direction == RULE_DIR_EGRESS) {
        proto_spec->set_direction(types::RULE_DIR_EGRESS);
    }
    for (uint32_t i = 0; i < api_spec->num_rules; i++) {
        pds::SecurityRule *proto_rule = proto_spec->add_rules();
        rule_t *api_rule = &api_spec->rules[i];
        proto_rule->set_priority(api_rule->priority);
        proto_rule->set_action(pds_rule_action_to_proto_action(&api_rule->action_data));
        proto_rule->set_stateful(api_rule->stateful);
        if (api_rule->match.l3_match.ip_proto) {
            proto_rule->mutable_match()->mutable_l3match()->set_protocol(
                                            api_rule->match.l3_match.ip_proto);
        }

        switch (api_rule->match.l3_match.src_match_type) {
        case IP_MATCH_PREFIX:
            if ((api_rule->match.l3_match.src_ip_pfx.len) &&
                ((api_rule->match.l3_match.src_ip_pfx.addr.af == IP_AF_IPV4) ||
                 (api_rule->match.l3_match.src_ip_pfx.addr.af == IP_AF_IPV6))) {
                ippfx_api_spec_to_proto_spec(
                    proto_rule->mutable_match()->mutable_l3match()->mutable_srcprefix(),
                    &api_rule->match.l3_match.src_ip_pfx);
            }
            break;
        case IP_MATCH_RANGE:
            iprange_api_spec_to_proto_spec(
                proto_rule->mutable_match()->mutable_l3match()->mutable_srcrange(),
                &api_rule->match.l3_match.src_ip_range);
            break;
        case IP_MATCH_TAG:
            proto_rule->mutable_match()->mutable_l3match()->set_srctag(
                api_rule->match.l3_match.src_tag);
            break;
        default:
            break;
        }

        switch (api_rule->match.l3_match.dst_match_type) {
        case IP_MATCH_PREFIX:
            if ((api_rule->match.l3_match.dst_ip_pfx.len) &&
                ((api_rule->match.l3_match.dst_ip_pfx.addr.af == IP_AF_IPV4) ||
                 (api_rule->match.l3_match.dst_ip_pfx.addr.af == IP_AF_IPV6))) {
                ippfx_api_spec_to_proto_spec(
                    proto_rule->mutable_match()->mutable_l3match()->mutable_dstprefix(),
                    &api_rule->match.l3_match.dst_ip_pfx);
            }
            break;
        case IP_MATCH_RANGE:
            iprange_api_spec_to_proto_spec(
                proto_rule->mutable_match()->mutable_l3match()->mutable_dstrange(),
                &api_rule->match.l3_match.dst_ip_range);
            break;
        case IP_MATCH_TAG:
            proto_rule->mutable_match()->mutable_l3match()->set_dsttag(
                api_rule->match.l3_match.dst_tag);
            break;
        default:
            break;
        }
        proto_rule->mutable_match()->mutable_l4match()->mutable_ports()->mutable_srcportrange()->set_portlow(api_rule->match.l4_match.sport_range.port_lo);
        proto_rule->mutable_match()->mutable_l4match()->mutable_ports()->mutable_srcportrange()->set_porthigh(api_rule->match.l4_match.sport_range.port_hi);
        proto_rule->mutable_match()->mutable_l4match()->mutable_ports()->mutable_dstportrange()->set_portlow(api_rule->match.l4_match.dport_range.port_lo);
        proto_rule->mutable_match()->mutable_l4match()->mutable_ports()->mutable_dstportrange()->set_porthigh(api_rule->match.l4_match.dport_range.port_hi);
    }

    return;
}

// populate proto buf status from policy API status
static inline void
pds_policy_api_status_to_proto (pds::SecurityPolicyStatus *proto_status,
                                const pds_policy_status_t *api_status)
{
}

// populate proto buf stats from policy API stats
static inline void
pds_policy_api_stats_to_proto (pds::SecurityPolicyStats *proto_stats,
                                 const pds_policy_stats_t *api_stats)
{
}

// populate proto buf from policy API info
static inline void
pds_policy_api_info_to_proto (const pds_policy_info_t *api_info, void *ctxt)
{
    pds::SecurityPolicyGetResponse *proto_rsp = (pds::SecurityPolicyGetResponse *)ctxt;
    auto policy = proto_rsp->add_response();
    pds::SecurityPolicySpec *proto_spec = policy->mutable_spec();
    pds::SecurityPolicyStatus *proto_status = policy->mutable_status();
    pds::SecurityPolicyStats *proto_stats = policy->mutable_stats();

    pds_policy_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_policy_api_status_to_proto(proto_status, &api_info->status);
    pds_policy_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build policy API spec from protobuf spec
static inline sdk_ret_t
pds_security_profile_proto_to_api_spec (pds_security_profile_spec_t *api_spec,
                                        const pds::SecurityProfileSpec &proto_spec)
{
    api_spec->key.id = proto_spec.id();
    api_spec->conn_track_en = proto_spec.conntracken();
    api_spec->default_action.fw_action.action =
            pds_proto_action_to_rule_action(proto_spec.defaultfwaction());
    api_spec->tcp_idle_timeout = proto_spec.tcpidletimeout();
    api_spec->udp_idle_timeout = proto_spec.udpidletimeout();
    api_spec->icmp_idle_timeout = proto_spec.icmpidletimeout();
    api_spec->other_idle_timeout = proto_spec.otheridletimeout();
    api_spec->tcp_syn_timeout = proto_spec.tcpcnxnsetuptimeout();
    api_spec->tcp_halfclose_timeout = proto_spec.tcphalfclosetimeout();
    api_spec->tcp_close_timeout = proto_spec.tcpclosetimeout();
    api_spec->tcp_drop_timeout = proto_spec.tcpdroptimeout();
    api_spec->udp_drop_timeout = proto_spec.udpdroptimeout();
    api_spec->icmp_drop_timeout = proto_spec.icmpdroptimeout();
    api_spec->other_drop_timeout = proto_spec.otherdroptimeout();
    return SDK_RET_OK;
}

// populate proto buf spec from security profile API spec
static inline void
pds_security_profile_api_spec_to_proto (pds::SecurityProfileSpec *proto_spec,
                                        const pds_security_profile_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }

    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_conntracken(api_spec->conn_track_en);
    proto_spec->set_defaultfwaction(
            pds_rule_action_to_proto_action((rule_action_data_t *)&api_spec->default_action));
    proto_spec->set_tcpidletimeout(api_spec->tcp_idle_timeout);
    proto_spec->set_udpidletimeout(api_spec->udp_idle_timeout);
    proto_spec->set_icmpidletimeout(api_spec->icmp_idle_timeout);
    proto_spec->set_otheridletimeout(api_spec->other_idle_timeout);
    proto_spec->set_tcpcnxnsetuptimeout(api_spec->tcp_syn_timeout);
    proto_spec->set_tcphalfclosetimeout(api_spec->tcp_halfclose_timeout);
    proto_spec->set_tcpclosetimeout(api_spec->tcp_close_timeout);
    proto_spec->set_tcpdroptimeout(api_spec->tcp_drop_timeout);
    proto_spec->set_udpdroptimeout(api_spec->udp_drop_timeout);
    proto_spec->set_icmpdroptimeout(api_spec->icmp_drop_timeout);
    proto_spec->set_otherdroptimeout(api_spec->other_drop_timeout);
}

// populate proto buf status from security profile API status
static inline void
pds_security_profile_api_status_to_proto (pds::SecurityProfileStatus *proto_status,
                                          const pds_security_profile_status_t *api_status)
{
}

// populate proto buf stats from security profile API stats
static inline void
pds_security_profile_api_stats_to_proto (pds::SecurityProfileStats *proto_stats,
                                         const pds_security_profile_stats_t *api_stats)
{
}

// populate proto buf from security profile API info
static inline void
pds_security_profile_api_info_to_proto (const pds_security_profile_info_t *api_info, void *ctxt)
{
    pds::SecurityProfileGetResponse *proto_rsp = (pds::SecurityProfileGetResponse *)ctxt;
    auto profile = proto_rsp->add_response();
    pds::SecurityProfileSpec *proto_spec = profile->mutable_spec();
    pds::SecurityProfileStatus *proto_status = profile->mutable_status();
    pds::SecurityProfileStats *proto_stats = profile->mutable_stats();

    pds_security_profile_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_security_profile_api_status_to_proto(proto_status, &api_info->status);
    pds_security_profile_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline pds_nh_type_t
proto_nh_type_to_pds_nh_type (pds::NexthopType type)
{
    switch (type) {
    case pds::NEXTHOP_TYPE_IP:
        return PDS_NH_TYPE_IP;
    case pds::NEXTHOP_TYPE_UNDERLAY:
        return PDS_NH_TYPE_UNDERLAY;
    case pds::NEXTHOP_TYPE_OVERLAY:
        return PDS_NH_TYPE_OVERLAY;
    default:
        return PDS_NH_TYPE_NONE;
    }
}

// build nh API spec from protobuf spec
static inline void
pds_nh_proto_to_api_spec (pds_nexthop_spec_t *api_spec,
                          const pds::NexthopSpec &proto_spec)
{
    api_spec->key.id = proto_spec.id();
    switch (proto_spec.nhinfo_case()) {
    case pds::NexthopSpec::kIPNhInfo:
        api_spec->type = PDS_NH_TYPE_IP;
        api_spec->vpc.id = proto_spec.ipnhinfo().vpcid();
        ipaddr_proto_spec_to_api_spec(&api_spec->ip,
                                      proto_spec.ipnhinfo().ip());
        api_spec->vlan = proto_spec.ipnhinfo().vlan();
        if (proto_spec.ipnhinfo().mac() != 0) {
            MAC_UINT64_TO_ADDR(api_spec->mac, proto_spec.ipnhinfo().mac());
        }
        break;

    case pds::NexthopSpec::kTunnelId:
        api_spec->type = PDS_NH_TYPE_OVERLAY;
        api_spec->tep.id = proto_spec.tunnelid();
        break;

    case pds::NexthopSpec::kUnderlayNhInfo:
        api_spec->type = PDS_NH_TYPE_UNDERLAY;
        api_spec->l3_if.id = proto_spec.underlaynhinfo().l3interfaceid();
        MAC_UINT64_TO_ADDR(api_spec->underlay_mac,
                           proto_spec.underlaynhinfo().underlaymac());
        break;

    default:
         api_spec->type = PDS_NH_TYPE_NONE;
         break;
    }
}

// populate proto buf spec from nh API spec
static inline void
pds_nh_api_spec_to_proto (pds::NexthopSpec *proto_spec,
                          const pds_nexthop_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    if (api_spec->type == PDS_NH_TYPE_IP) {
        auto ipnhinfo = proto_spec->mutable_ipnhinfo();
        ipnhinfo->set_vpcid(api_spec->vpc.id);
        ipaddr_api_spec_to_proto_spec(ipnhinfo->mutable_ip(), &api_spec->ip);
        ipnhinfo->set_vlan(api_spec->vlan);
        ipnhinfo->set_mac(MAC_TO_UINT64(api_spec->mac));
    } else if (api_spec->type == PDS_NH_TYPE_OVERLAY) {
        proto_spec->set_tunnelid(api_spec->tep.id);
    } else if (api_spec->type == PDS_NH_TYPE_UNDERLAY) {
        auto underlayinfo = proto_spec->mutable_underlaynhinfo();
        underlayinfo->set_l3interfaceid(api_spec->l3_if.id);
        underlayinfo->set_underlaymac(MAC_TO_UINT64(api_spec->underlay_mac));
    }
}

// populate proto buf status from nh API status
static inline void
pds_nh_api_status_to_proto (pds::NexthopStatus *proto_status,
                            const pds_nexthop_status_t *api_status)
{
}

// populate proto buf stats from nh API stats
static inline void
pds_nh_api_stats_to_proto (pds::NexthopStats *proto_stats,
                           const pds_nexthop_stats_t *api_stats)
{
}

// populate proto buf from nh API info
static inline void
pds_nh_api_info_to_proto (const pds_nexthop_info_t *api_info, void *ctxt)
{
    pds::NexthopGetResponse *proto_rsp = (pds::NexthopGetResponse *)ctxt;
    auto nh = proto_rsp->add_response();
    pds::NexthopSpec *proto_spec = nh->mutable_spec();
    pds::NexthopStatus *proto_status = nh->mutable_status();
    pds::NexthopStats *proto_stats = nh->mutable_stats();

    pds_nh_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_nh_api_status_to_proto(proto_status, &api_info->status);
    pds_nh_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build nh group API spec from protobuf spec
static inline sdk_ret_t
pds_nh_group_proto_to_api_spec (pds_nexthop_group_spec_t *api_spec,
                                const pds::NhGroupSpec &proto_spec)
{
    api_spec->key.id = proto_spec.id();
    api_spec->num_nexthops = proto_spec.members_size();
    switch (proto_spec.type()) {
    case pds::NEXTHOP_GROUP_TYPE_OVERLAY_ECMP:
        if (api_spec->num_nexthops > PDS_MAX_OVERLAY_ECMP_TEP) {
            return SDK_RET_INVALID_ARG;
        }
        api_spec->type = PDS_NHGROUP_TYPE_OVERLAY_ECMP;
        break;

    case pds::NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP:
        if (api_spec->num_nexthops > PDS_MAX_ECMP_NEXTHOP) {
            return SDK_RET_INVALID_ARG;
        }
        api_spec->type = PDS_NHGROUP_TYPE_UNDERLAY_ECMP;
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }
    for (uint32_t i = 0; i < api_spec->num_nexthops; i++) {
        pds_nh_proto_to_api_spec(&api_spec->nexthops[i],
                                 proto_spec.members(i));
    }
    return SDK_RET_OK;
}

// populate proto buf spec from nh API spec
static inline sdk_ret_t
pds_nh_group_api_spec_to_proto (pds::NhGroupSpec *proto_spec,
                                const pds_nexthop_group_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    switch (api_spec->type) {
    case PDS_NHGROUP_TYPE_OVERLAY_ECMP:
        proto_spec->set_type(pds::NEXTHOP_GROUP_TYPE_OVERLAY_ECMP);
        break;

    case PDS_NHGROUP_TYPE_UNDERLAY_ECMP:
        proto_spec->set_type(pds::NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP);
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }

    for (uint32_t i = 0; i < api_spec->num_nexthops; i++) {
        pds_nh_api_spec_to_proto(proto_spec->add_members(),
                                 &api_spec->nexthops[i]);
    }
    return SDK_RET_OK;
}

// populate proto buf status from nh group API status
static inline void
pds_nh_group_api_status_to_proto (pds::NhGroupStatus *proto_status,
                                  const pds_nexthop_group_status_t *api_status)
{
}

// populate proto buf stats from nh group API stats
static inline void
pds_nh_group_api_stats_to_proto (pds::NhGroupStats *proto_stats,
                                 const pds_nexthop_group_stats_t *api_stats)
{
}

// populate proto buf from nh API info
static inline void
pds_nh_group_api_info_to_proto (const pds_nexthop_group_info_t *api_info,
                                void *ctxt)
{
    pds::NhGroupGetResponse *proto_rsp = (pds::NhGroupGetResponse *)ctxt;
    auto nh = proto_rsp->add_response();
    pds::NhGroupSpec *proto_spec = nh->mutable_spec();
    pds::NhGroupStatus *proto_status = nh->mutable_status();
    pds::NhGroupStats *proto_stats = nh->mutable_stats();

    pds_nh_group_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_nh_group_api_status_to_proto(proto_status, &api_info->status);
    pds_nh_group_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_dhcp_relay_proto_to_api_spec (pds_dhcp_relay_spec_t *api_spec,
                                  const pds::DHCPRelaySpec &proto_spec)
{
    api_spec->key.id = proto_spec.id();
    api_spec->vpc.id = proto_spec.vpcid();
    ipaddr_proto_spec_to_api_spec(&api_spec->server_ip, proto_spec.serverip());
    ipaddr_proto_spec_to_api_spec(&api_spec->agent_ip, proto_spec.agentip());
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_relay_api_spec_to_proto (pds::DHCPRelaySpec *proto_spec,
                                  const pds_dhcp_relay_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_vpcid(api_spec->vpc.id);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_serverip(),
                                  &api_spec->server_ip);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_agentip(),
                                  &api_spec->agent_ip);
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_relay_api_status_to_proto (pds::DHCPRelayStatus *proto_status,
                                    const pds_dhcp_relay_status_t *api_status)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_relay_api_stats_to_proto (pds::DHCPRelayStats *proto_stats,
                                   const pds_dhcp_relay_stats_t *api_stats)
{
    return SDK_RET_OK;
}

// populate proto buf from route table API info
static inline void
pds_dhcp_relay_api_info_to_proto (const pds_dhcp_relay_info_t *api_info,
                                  void *ctxt)
{
    pds::DHCPRelayGetResponse *proto_rsp = (pds::DHCPRelayGetResponse *)ctxt;
    auto dhcp = proto_rsp->add_response();
    pds::DHCPRelaySpec *proto_spec = dhcp->mutable_spec();
    pds::DHCPRelayStatus *proto_status = dhcp->mutable_status();
    pds::DHCPRelayStats *proto_stats = dhcp->mutable_stats();

    pds_dhcp_relay_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_dhcp_relay_api_status_to_proto(proto_status, &api_info->status);
    pds_dhcp_relay_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_dhcp_policy_proto_to_api_spec (pds_dhcp_policy_spec_t *api_spec,
                                   const pds::DHCPPolicySpec &proto_spec)
{
    api_spec->key.id = proto_spec.id();
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
pds_dhcp_policy_api_spec_to_proto (pds::DHCPPolicySpec *proto_spec,
                                  const pds_dhcp_policy_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
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
pds_dhcp_policy_api_status_to_proto (pds::DHCPPolicyStatus *proto_status,
                                    const pds_dhcp_policy_status_t *api_status)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_dhcp_policy_api_stats_to_proto (pds::DHCPPolicyStats *proto_stats,
                                   const pds_dhcp_policy_stats_t *api_stats)
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
    pds_dhcp_policy_api_status_to_proto(proto_status, &api_info->status);
    pds_dhcp_policy_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_nat_port_block_proto_to_api_spec (pds_nat_port_block_spec_t *api_spec,
                                      const pds::NatPortBlockSpec &proto_spec)
{
    api_spec->key.id = proto_spec.id();
    api_spec->vpc.id = proto_spec.vpcid();
    api_spec->ip_proto = proto_spec.protocol();
    if (proto_spec.nataddress().has_prefix()) {
        ipsubnet_proto_spec_to_ipvx_range(&api_spec->nat_ip_range,
                                          proto_spec.nataddress().prefix());
    } else if (proto_spec.nataddress().has_range()) {
        iprange_proto_spec_to_api_spec(&api_spec->nat_ip_range,
                                       proto_spec.nataddress().range());
    }
    api_spec->nat_port_range.port_lo = proto_spec.ports().portlow();
    api_spec->nat_port_range.port_hi = proto_spec.ports().porthigh();
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nat_port_block_api_spec_to_proto (pds::NatPortBlockSpec *proto_spec,
                                      const pds_nat_port_block_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_vpcid(api_spec->vpc.id);
    proto_spec->set_protocol(api_spec->ip_proto);
    auto range_spec = proto_spec->mutable_nataddress()->mutable_range();
    iprange_api_spec_to_proto_spec(range_spec, &api_spec->nat_ip_range);
    proto_spec->mutable_ports()->set_portlow(api_spec->nat_port_range.port_lo);
    proto_spec->mutable_ports()->set_porthigh(api_spec->nat_port_range.port_hi);
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nat_port_block_api_status_to_proto (pds::NatPortBlockStatus *proto_status,
                                        const pds_nat_port_block_status_t *api_status)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nat_port_block_api_stats_to_proto (pds::NatPortBlockStats *proto_stats,
                                       const pds_nat_port_block_stats_t *api_stats)
{
    proto_stats->set_inusecount(api_stats->in_use_count);
    proto_stats->set_sessioncount(api_stats->session_count);
    return SDK_RET_OK;
}

// populate proto buf from route table API info
static inline void
pds_nat_port_block_api_info_to_proto (const pds_nat_port_block_info_t *api_info,
                                      void *ctxt)
{
    pds::NatPortBlockGetResponse *proto_rsp =
        (pds::NatPortBlockGetResponse *)ctxt;
    auto dhcp = proto_rsp->add_response();
    pds::NatPortBlockSpec *proto_spec = dhcp->mutable_spec();
    pds::NatPortBlockStatus *proto_status = dhcp->mutable_status();
    pds::NatPortBlockStats *proto_stats = dhcp->mutable_stats();

    pds_nat_port_block_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_nat_port_block_api_status_to_proto(proto_status, &api_info->status);
    pds_nat_port_block_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_route_table_proto_to_api_spec (pds_route_table_spec_t *api_spec,
                                   const pds::RouteTableSpec &proto_spec)
{
    uint32_t num_routes = 0;

    api_spec->key.id = proto_spec.id();
    switch (proto_spec.af()) {
    case types::IP_AF_INET:
        api_spec->af = IP_AF_IPV4;
        break;

    case types::IP_AF_INET6:
        api_spec->af = IP_AF_IPV6;
        break;

    default:
        SDK_ASSERT(FALSE);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->enable_pbr = proto_spec.enablepbr();
    num_routes = proto_spec.routes_size();
    api_spec->num_routes = num_routes;
    api_spec->routes = (pds_route_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                                 sizeof(pds_route_t) *
                                                 num_routes);
    if (unlikely(api_spec->routes == NULL)) {
        PDS_TRACE_ERR("Failed to allocate memory for route table {}",
                      api_spec->key.id);
        return sdk::SDK_RET_OOM;
    }
    for (uint32_t i = 0; i < num_routes; i++) {
        const pds::Route &proto_route = proto_spec.routes(i);
        ippfx_proto_spec_to_api_spec(&api_spec->routes[i].prefix,
                                     proto_route.prefix());
        if (api_spec->enable_pbr) {
            api_spec->routes[i].prio = proto_route.priority();
        }
        switch (proto_route.nh_case()) {
        case pds::Route::kNextHop:
        case pds::Route::kTunnelId:
            api_spec->routes[i].nh_type = PDS_NH_TYPE_OVERLAY;
            api_spec->routes[i].tep.id = proto_route.tunnelid();
            break;
        case pds::Route::kNexthopGroupId:
            // NOTE: UNDERLAY_ECMP is not done in the datapath
            api_spec->routes[i].nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
            api_spec->routes[i].nh_group.id = proto_route.nexthopgroupid();
            break;
        case pds::Route::kVPCId:
            api_spec->routes[i].vpc.id = proto_route.vpcid();
            api_spec->routes[i].nh_type = PDS_NH_TYPE_PEER_VPC;
            break;
        case pds::Route::kVnicId:
            api_spec->routes[i].vnic.id = proto_route.vnicid();
            api_spec->routes[i].nh_type = PDS_NH_TYPE_VNIC;
            break;
        case pds::Route::kNexthopId:
            api_spec->routes[i].nh_type = PDS_NH_TYPE_IP;
            api_spec->routes[i].nh.id = proto_route.nexthopid();
            break;
        default:
            api_spec->routes[i].nh_type = PDS_NH_TYPE_BLACKHOLE;
            break;
        }
        if (proto_route.has_nataction()) {
            auto nat_action = proto_route.nataction();
            switch (nat_action.srcnataction()) {
            case types::NAT_ACTION_STATIC:
                api_spec->routes[i].nat.src_nat_type = PDS_NAT_TYPE_STATIC;
                break;
            case types::NAT_ACTION_NAPT_PUBLIC:
                api_spec->routes[i].nat.src_nat_type = PDS_NAT_TYPE_NAPT_PUBLIC;
                break;
            case types::NAT_ACTION_NAPT_SVC:
                api_spec->routes[i].nat.src_nat_type = PDS_NAT_TYPE_NAPT_SVC;
                break;
            case types::NAT_ACTION_NONE:
            default:
                api_spec->routes[i].nat.src_nat_type = PDS_NAT_TYPE_NONE;
                break;
            }
            if (nat_action.has_dstnatip()) {
                ipaddr_proto_spec_to_api_spec(
                    &api_spec->routes[i].nat.dst_nat_ip, nat_action.dstnatip());
            } else {
                memset(&api_spec->routes[i].nat.dst_nat_ip, 0,
                       sizeof(api_spec->routes[i].nat.dst_nat_ip));
            }
        }
    }
    return SDK_RET_OK;
}

// populate proto buf spec from route table API spec
static inline void
pds_route_table_api_spec_to_proto (pds::RouteTableSpec *proto_spec,
                                   const pds_route_table_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }

    proto_spec->set_id(api_spec->key.id);
    if (api_spec->af == IP_AF_IPV4) {
        proto_spec->set_af(types::IP_AF_INET);
    } else if (api_spec->af == IP_AF_IPV6) {
        proto_spec->set_af(types::IP_AF_INET6);
    } else {
        SDK_ASSERT(FALSE);
    }
    proto_spec->set_enablepbr(api_spec->enable_pbr);

    for (uint32_t i = 0; i < api_spec->num_routes; i++) {
        pds::Route *route = proto_spec->add_routes();
        ippfx_api_spec_to_proto_spec(route->mutable_prefix(),
                                     &api_spec->routes[i].prefix);
        if (api_spec->enable_pbr) {
            route->set_priority(api_spec->routes[i].prio);
        }
        switch (api_spec->routes[i].nh_type) {
        case PDS_NH_TYPE_OVERLAY:
            route->set_tunnelid(api_spec->routes[i].tep.id);
            break;
        case PDS_NH_TYPE_OVERLAY_ECMP:
            route->set_nexthopgroupid(api_spec->routes[i].nh_group.id);
            break;
        case PDS_NH_TYPE_PEER_VPC:
            route->set_vpcid(api_spec->routes[i].vpc.id);
            break;
        case PDS_NH_TYPE_VNIC:
            route->set_vnicid(api_spec->routes[i].vnic.id);
            break;
        case PDS_NH_TYPE_IP:
            route->set_nexthopid(api_spec->routes[i].nh.id);
            break;
        case PDS_NH_TYPE_BLACKHOLE:
        default:
            // blackhole nexthop
            break;
        }
        switch (api_spec->routes[i].nat.src_nat_type) {
        case PDS_NAT_TYPE_STATIC:
            route->mutable_nataction()->set_srcnataction(types::NAT_ACTION_STATIC);
            break;
        case PDS_NAT_TYPE_NAPT_PUBLIC:
            route->mutable_nataction()->set_srcnataction(types::NAT_ACTION_NAPT_PUBLIC);
            break;
        case PDS_NAT_TYPE_NAPT_SVC:
            route->mutable_nataction()->set_srcnataction(types::NAT_ACTION_NAPT_SVC);
            break;
        case PDS_NAT_TYPE_NONE:
        default:
            route->mutable_nataction()->set_srcnataction(types::NAT_ACTION_NONE);
            break;
        }
        if (!ip_addr_is_zero(&api_spec->routes[i].nat.dst_nat_ip)) {
            ipaddr_api_spec_to_proto_spec(
                route->mutable_nataction()->mutable_dstnatip(),
                &api_spec->routes[i].nat.dst_nat_ip);
        }
    }
    return;
}

// populate proto buf status from route table API status
static inline void
pds_route_table_api_status_to_proto (pds::RouteTableStatus *proto_status,
                                     const pds_route_table_status_t *api_status)
{
}

// populate proto buf stats from route table API stats
static inline void
pds_route_table_api_stats_to_proto (pds::RouteTableStats *proto_stats,
                                    const pds_route_table_stats_t *api_stats)
{
}

// populate proto buf from route table API info
static inline void
pds_route_table_api_info_to_proto (const pds_route_table_info_t *api_info,
                                   void *ctxt)
{
    pds::RouteTableGetResponse *proto_rsp = (pds::RouteTableGetResponse *)ctxt;
    auto route_table = proto_rsp->add_response();
    pds::RouteTableSpec *proto_spec = route_table->mutable_spec();
    pds::RouteTableStatus *proto_status = route_table->mutable_status();
    pds::RouteTableStats *proto_stats = route_table->mutable_stats();

    pds_route_table_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_route_table_api_status_to_proto(proto_status, &api_info->status);
    pds_route_table_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline void
pds_table_api_stats_to_proto (pds::TableApiStats *proto_stats,
                              sdk::table::sdk_table_api_stats_t *stats)
{
    auto entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_INSERT);
    entry->set_count(stats->insert);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_INSERT_DUPLICATE);
    entry->set_count(stats->insert_duplicate);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_INSERT_FAIL);
    entry->set_count(stats->insert_fail);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_REMOVE);
    entry->set_count(stats->remove);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_REMOVE_NOT_FOUND);
    entry->set_count(stats->remove_not_found);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_REMOVE_FAIL);
    entry->set_count(stats->remove_fail);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_UPDATE);
    entry->set_count(stats->update);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_UPDATE_FAIL);
    entry->set_count(stats->update_fail);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_GET);
    entry->set_count(stats->get);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_GET_FAIL);
    entry->set_count(stats->get_fail);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_RESERVE);
    entry->set_count(stats->reserve);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_RESERVE_FAIL);
    entry->set_count(stats->reserve_fail);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_RELEASE);
    entry->set_count(stats->release);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_API_STATS_RELEASE_FAIL);
    entry->set_count(stats->release_fail);
}

static inline void
pds_table_stats_to_proto (pds::TableStats *proto_stats,
                          sdk::table::sdk_table_stats_t *stats)
{
    auto entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_STATS_ENTRIES);
    entry->set_count(stats->entries);

    entry = proto_stats->add_entry();
    entry->set_type(pds::TABLE_STATS_COLLISIONS);
    entry->set_count(stats->collisions);
}

static inline void
pds_table_stats_entry_to_proto (pds_table_stats_t *stats, void *ctxt)
{
    pds::TableStatsGetResponse *rsp = (pds::TableStatsGetResponse *)ctxt;
    auto response = rsp->add_response();
    auto api_stats = response->mutable_apistats();
    auto table_stats = response->mutable_tablestats();

    response->set_tablename(stats->table_name);
    pds_table_api_stats_to_proto(api_stats, &stats->api_stats);
    pds_table_stats_to_proto(table_stats, &stats->table_stats);
    rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
}

static inline void
pds_pb_stats_port_to_proto (pds::PacketBufferPort *buf_port, uint32_t port)
{
    if ((port >= TM_UPLINK_PORT_BEGIN) && (port <= TM_UPLINK_PORT_END)) {
        buf_port->set_porttype(pds::PACKET_BUFFER_PORT_TYPE_UPLINK);
        buf_port->set_portnum(port-TM_UPLINK_PORT_BEGIN);
    } else if ((port >= TM_DMA_PORT_BEGIN) && (port <= TM_DMA_PORT_END)) {
        buf_port->set_porttype(pds::PACKET_BUFFER_PORT_TYPE_DMA);
        buf_port->set_portnum(TM_PORT_DMA);
    } else if (port == TM_PORT_INGRESS) {
        buf_port->set_porttype(pds::PACKET_BUFFER_PORT_TYPE_P4IG);
        buf_port->set_portnum(TM_PORT_INGRESS);
    } else if (port == TM_PORT_EGRESS) {
        buf_port->set_porttype(pds::PACKET_BUFFER_PORT_TYPE_P4EG);
        buf_port->set_portnum(TM_PORT_EGRESS);
    }
}

static inline void
pds_session_debug_stats_to_proto (uint32_t idx,
                                  pds_session_debug_stats_t *stats, void *ctxt)
{
    pds::SessionStatsGetResponse *rsp = (pds::SessionStatsGetResponse *)ctxt;
    auto proto_stats = rsp->add_stats();

    proto_stats->set_statsindex(idx);
    proto_stats->set_initiatorflowpkts(stats->iflow_packet_count);
    proto_stats->set_initiatorflowbytes(stats->iflow_bytes_count);
    proto_stats->set_responderflowpkts(stats->rflow_packet_count);
    proto_stats->set_responderflowbytes(stats->rflow_bytes_count);
}

static inline void
pds_session_to_proto (void *ctxt)
{
}

static inline void
pds_ipv4_flow_to_proto (ftlite::internal::ipv4_entry_t *ipv4_entry,
                        void *ctxt)
{
    flow_get_t *fget = (flow_get_t *)ctxt;
    auto flow = fget->msg.add_flow();
    auto key = flow->mutable_key();
    auto srcaddr = key->mutable_srcaddr();
    auto dstaddr = key->mutable_dstaddr();

    srcaddr->set_af(types::IP_AF_INET);
    srcaddr->set_v4addr(ipv4_entry->src);
    dstaddr->set_af(types::IP_AF_INET);
    dstaddr->set_v4addr(ipv4_entry->dst);
    key->set_srcport(ipv4_entry->sport);
    key->set_dstport(ipv4_entry->dport);
    key->set_ipproto(ipv4_entry->proto);

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
    auto key = flow->mutable_key();
    auto srcaddr = key->mutable_srcaddr();
    auto dstaddr = key->mutable_dstaddr();

    srcaddr->set_af(types::IP_AF_INET6);
    srcaddr->set_v6addr(std::string((const char *)(ipv6_entry->src),
                        IP6_ADDR8_LEN));
    dstaddr->set_af(types::IP_AF_INET6);
    srcaddr->set_v6addr(std::string((const char *)(ipv6_entry->dst),
                        IP6_ADDR8_LEN));
    key->set_srcport(ipv6_entry->sport);
    key->set_dstport(ipv6_entry->dport);
    key->set_ipproto(ipv6_entry->proto);

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

static inline void
pds_flow_to_proto (ftlite::internal::ipv4_entry_t *ipv4_entry,
                   ftlite::internal::ipv6_entry_t *ipv6_entry,
                   void *ctxt)
{
    if (ipv4_entry) {
        pds_ipv4_flow_to_proto(ipv4_entry, ctxt);
    } else if (ipv6_entry) {
        pds_ipv6_flow_to_proto(ipv6_entry, ctxt);
    } else {
        flow_get_t *fget = (flow_get_t *)ctxt;
        if (fget->count) {
            fget->msg.set_apistatus(types::ApiStatus::API_STATUS_OK);
            fget->writer->Write(fget->msg);
            fget->msg.Clear();
            fget->count = 0;
        }
    }
}

static inline void
pds_pb_stats_entry_to_proto (pds_pb_debug_stats_t *pds_stats, void *ctxt)
{
    sdk::platform::capri::tm_pb_debug_stats_t *stats = &pds_stats->stats;
    sdk::platform::capri::capri_queue_stats_t *qos_queue_stats =
        &pds_stats->qos_queue_stats;
    pds::PbStatsGetResponse *rsp = (pds::PbStatsGetResponse *)ctxt;
    auto pb_stats = rsp->mutable_pbstats()->add_portstats();
    auto port = pb_stats->mutable_packetbufferport();
    auto buffer_stats = pb_stats->mutable_bufferstats();
    auto oflow_fifo_stats = pb_stats->mutable_oflowfifostats();
    auto q_stats = pb_stats->mutable_qosqueuestats();

    pds_pb_stats_port_to_proto(port, pds_stats->port);

    buffer_stats->set_sopcountin(stats->buffer_stats.sop_count_in);
    buffer_stats->set_eopcountin(stats->buffer_stats.eop_count_in);
    buffer_stats->set_sopcountout(stats->buffer_stats.sop_count_out);
    buffer_stats->set_eopcountout(stats->buffer_stats.eop_count_out);

    auto drop_stats = buffer_stats->mutable_dropcounts();
    for (int i = sdk::platform::capri::BUFFER_INTRINSIC_DROP;
         i < sdk::platform::capri::BUFFER_DROP_MAX; i ++) {
        auto drop_stats_entry = drop_stats->add_statsentries();
        drop_stats_entry->set_reasons(pds::BufferDropReasons(i));
        drop_stats_entry->set_dropcount(stats->buffer_stats.drop_counts[i]);
    }

    oflow_fifo_stats->set_sopcountin(stats->oflow_fifo_stats.sop_count_in);
    oflow_fifo_stats->set_eopcountin(stats->oflow_fifo_stats.eop_count_in);
    oflow_fifo_stats->set_sopcountout(stats->oflow_fifo_stats.sop_count_out);
    oflow_fifo_stats->set_eopcountout(stats->oflow_fifo_stats.eop_count_out);

    auto drop_counts = oflow_fifo_stats->mutable_dropcounts();
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::OCCUPANCY_DROP);
    drop_counts->mutable_entry(0)->set_count(stats->oflow_fifo_stats.drop_counts.occupancy_drop_count);
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::EMERGENCY_STOP_DROP);
    drop_counts->mutable_entry(1)->set_count(stats->oflow_fifo_stats.drop_counts.emergency_stop_drop_count);
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::WRITE_BUFFER_ACK_FILL_UP_DROP);
    drop_counts->mutable_entry(2)->set_count(stats->oflow_fifo_stats.drop_counts.write_buffer_ack_fill_up_drop_count);
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::WRITE_BUFFER_ACK_FULL_DROP);
    drop_counts->mutable_entry(3)->set_count(stats->oflow_fifo_stats.drop_counts.write_buffer_ack_full_drop_count);
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::WRITE_BUFFER_FULL_DROP);
    drop_counts->mutable_entry(4)->set_count(stats->oflow_fifo_stats.drop_counts.write_buffer_full_drop_count);
    drop_counts->add_entry()->set_type(pds::OflowFifoDropType::CONTROL_FIFO_FULL_DROP);
    drop_counts->mutable_entry(5)->set_count(stats->oflow_fifo_stats.drop_counts.control_fifo_full_drop_count);

    for (int i = 0; i < CAPRI_TM_MAX_QUEUES; i++) {
        if (qos_queue_stats->iq_stats[i].iq.valid) {
            auto input_stats = q_stats->add_inputqueuestats();
            auto iq_stats = &qos_queue_stats->iq_stats[i].stats;
            input_stats->set_inputqueueidx(qos_queue_stats->iq_stats[i].iq.queue);
            input_stats->mutable_oflowfifostats()->set_goodpktsin(iq_stats->oflow.good_pkts_in);
            input_stats->mutable_oflowfifostats()->set_goodpktsout(iq_stats->oflow.good_pkts_out);
            input_stats->mutable_oflowfifostats()->set_erroredpktsin(iq_stats->oflow.errored_pkts_in);
            input_stats->mutable_oflowfifostats()->set_fifodepth(iq_stats->oflow.fifo_depth);
            input_stats->mutable_oflowfifostats()->set_maxfifodepth(iq_stats->oflow.max_fifo_depth);
            input_stats->set_bufferoccupancy(iq_stats->buffer_occupancy);
            input_stats->set_peakoccupancy(iq_stats->peak_occupancy);
            input_stats->set_portmonitor(iq_stats->port_monitor);
        }
        if (qos_queue_stats->oq_stats[i].oq.valid) {
            auto output_stats = q_stats->add_outputqueuestats();
            auto oq_stats = &qos_queue_stats->oq_stats[i].stats;
            output_stats->set_outputqueueidx(qos_queue_stats->oq_stats[i].oq.queue);
            output_stats->set_queuedepth(oq_stats->queue_depth);
            output_stats->set_portmonitor(oq_stats->port_monitor);
        }
    }

    rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
}

// populate proto buf spec from mirror session API spec
static inline sdk_ret_t
pds_mirror_session_api_spec_to_proto (pds::MirrorSessionSpec *proto_spec,
                                      const pds_mirror_session_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_snaplen(api_spec->snap_len);
    switch (api_spec->type) {
    case PDS_MIRROR_SESSION_TYPE_RSPAN:
        {
            pds::RSpanSpec *proto_rspan = proto_spec->mutable_rspanspec();
            pds_encap_to_proto_encap(proto_rspan->mutable_encap(),
                                     &api_spec->rspan_spec.encap);
            proto_rspan->set_interfaceid(api_spec->rspan_spec.interface);
        }
        break;

    case PDS_MIRROR_SESSION_TYPE_ERSPAN:
        {
            pds::ERSpanSpec *proto_erspan = proto_spec->mutable_erspanspec();
            proto_erspan->set_tunnelid(api_spec->erspan_spec.tep.id);
            ipaddr_api_spec_to_proto_spec(proto_erspan->mutable_srcip(),
                                          &api_spec->erspan_spec.src_ip);
            proto_erspan->set_dscp(api_spec->erspan_spec.dscp);
            proto_erspan->set_spanid(api_spec->erspan_spec.span_id);
            proto_erspan->set_vpcid(api_spec->erspan_spec.vpc.id);
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
    api_spec->key.id = proto_spec.id();
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
        api_spec->rspan_spec.interface =
            proto_spec.rspanspec().interfaceid();
    } else if (proto_spec.has_erspanspec()) {
        if ((proto_spec.erspanspec().tunnelid() == 0) ||
            !proto_spec.erspanspec().has_srcip()) {
            PDS_TRACE_ERR("src IP or dst ID missing in mirror session {} spec",
                          api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        types::IPAddress srcip = proto_spec.erspanspec().srcip();
        api_spec->type = PDS_MIRROR_SESSION_TYPE_ERSPAN;
        api_spec->erspan_spec.tep.id = proto_spec.erspanspec().tunnelid();
        ipaddr_proto_spec_to_api_spec(&api_spec->erspan_spec.src_ip, srcip);
        api_spec->erspan_spec.dscp = proto_spec.erspanspec().dscp();
        api_spec->erspan_spec.span_id = proto_spec.erspanspec().spanid();
        api_spec->erspan_spec.vpc.id = proto_spec.erspanspec().vpcid();
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
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_ipaddr(),
                                  &api_spec->device_ip_addr);
    proto_spec->set_macaddr(MAC_TO_UINT64(api_spec->device_mac_addr));
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_gatewayip(),
                                  &api_spec->gateway_ip_addr);
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
}

// populate proto buf status from device API status
static inline void
pds_device_api_status_to_proto (pds::DeviceStatus *proto_status,
                                const pds_device_status_t *api_status)
{
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
    } else {
        return SDK_RET_INVALID_ARG;
    }
    MAC_UINT64_TO_ADDR(api_spec->device_mac_addr, macaddr);
    if ((gwipaddr.af() == types::IP_AF_INET) ||
        (gwipaddr.af() == types::IP_AF_INET6)) {
        ipaddr_proto_spec_to_api_spec(&api_spec->gateway_ip_addr, gwipaddr);
    } else {
        return SDK_RET_INVALID_ARG;
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
    return SDK_RET_OK;
}

// populate proto buf spec from subnet API spec
static inline void
pds_subnet_api_spec_to_proto (pds::SubnetSpec *proto_spec,
                              const pds_subnet_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_vpcid(api_spec->vpc.id);
    ipv4pfx_api_spec_to_proto_spec(
                    proto_spec->mutable_v4prefix(), &api_spec->v4_prefix);
    ippfx_api_spec_to_ipv6pfx_proto_spec(
                    proto_spec->mutable_v6prefix(), &api_spec->v6_prefix);
    proto_spec->set_ipv4virtualrouterip(api_spec->v4_vr_ip);
    proto_spec->set_ipv6virtualrouterip(&api_spec->v6_vr_ip.addr.v6_addr.addr8,
                                        IP6_ADDR8_LEN);
    proto_spec->set_virtualroutermac(MAC_TO_UINT64(api_spec->vr_mac));
    proto_spec->set_v4routetableid(api_spec->v4_route_table.id);
    proto_spec->set_v6routetableid(api_spec->v6_route_table.id);

    for (uint8_t i = 0; i < api_spec->num_ing_v4_policy; i++) {
        proto_spec->add_ingv4securitypolicyid(api_spec->ing_v4_policy[i].id);
    }
    for (uint8_t i = 0; i < api_spec->num_ing_v6_policy; i++) {
        proto_spec->add_ingv6securitypolicyid(api_spec->ing_v6_policy[i].id);
    }
    for (uint8_t i = 0; i < api_spec->num_egr_v4_policy; i++) {
        proto_spec->add_egv4securitypolicyid(api_spec->egr_v4_policy[i].id);
    }
    for (uint8_t i = 0; i < api_spec->num_egr_v6_policy; i++) {
        proto_spec->add_egv6securitypolicyid(api_spec->egr_v6_policy[i].id);
    }
    pds_encap_to_proto_encap(proto_spec->mutable_fabricencap(),
                             &api_spec->fabric_encap);
    proto_spec->set_hostifindex(api_spec->host_ifindex);
    proto_spec->set_dhcppolicyid(api_spec->dhcp_policy.id);
    proto_spec->set_tos(api_spec->tos);
}

// populate proto buf status from subnet API status
static inline void
pds_subnet_api_status_to_proto (pds::SubnetStatus *proto_status,
                                const pds_subnet_status_t *api_status)
{
    proto_status->set_hwid(api_status->hw_id);
}

// populate proto buf stats from subnet API stats
static inline void
pds_subnet_api_stats_to_proto (pds::SubnetStats *proto_stats,
                               const pds_subnet_stats_t *api_stats)
{
}

// populate proto buf from subnet API info
static inline void
pds_subnet_api_info_to_proto (const pds_subnet_info_t *api_info, void *ctxt)
{
    pds::SubnetGetResponse *proto_rsp = (pds::SubnetGetResponse *)ctxt;
    auto subnet = proto_rsp->add_response();
    pds::SubnetSpec *proto_spec = subnet->mutable_spec();
    pds::SubnetStatus *proto_status = subnet->mutable_status();
    pds::SubnetStats *proto_stats = subnet->mutable_stats();

    pds_subnet_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_subnet_api_status_to_proto(proto_status, &api_info->status);
    pds_subnet_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build subnet API spec from proto buf spec
static inline sdk_ret_t
pds_subnet_proto_to_api_spec (pds_subnet_spec_t *api_spec,
                              const pds::SubnetSpec &proto_spec)
{
    api_spec->key.id = proto_spec.id();
    api_spec->vpc.id = proto_spec.vpcid();
    ipv4pfx_proto_spec_to_api_spec(&api_spec->v4_prefix, proto_spec.v4prefix());
    ipv6pfx_proto_spec_to_ippfx_api_spec(&api_spec->v6_prefix,
                                         proto_spec.v6prefix());
    api_spec->v4_vr_ip = proto_spec.ipv4virtualrouterip();
    api_spec->v6_vr_ip.af = IP_AF_IPV6;
    memcpy(api_spec->v6_vr_ip.addr.v6_addr.addr8,
           proto_spec.ipv6virtualrouterip().c_str(), IP6_ADDR8_LEN);
    MAC_UINT64_TO_ADDR(api_spec->vr_mac, proto_spec.virtualroutermac());
    api_spec->v4_route_table.id = proto_spec.v4routetableid();
    api_spec->v6_route_table.id = proto_spec.v6routetableid();

    if (proto_spec.ingv4securitypolicyid_size() > PDS_MAX_SUBNET_POLICY) {
        PDS_TRACE_ERR("No. of IPv4 ingress security policies on subnet can't "
                      "exceed {}", PDS_MAX_SUBNET_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_ing_v4_policy = proto_spec.ingv4securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_ing_v4_policy; i++) {
        api_spec->ing_v4_policy[i].id = proto_spec.ingv4securitypolicyid(i);
    }
    if (proto_spec.ingv6securitypolicyid_size() > PDS_MAX_SUBNET_POLICY) {
        PDS_TRACE_ERR("No. of IPv6 ingress security policies on subnet can't "
                      "exceed {}", PDS_MAX_SUBNET_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_ing_v6_policy = proto_spec.ingv6securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_ing_v6_policy; i++) {
        api_spec->ing_v6_policy[i].id = proto_spec.ingv6securitypolicyid(i);
    }
    if (proto_spec.egv4securitypolicyid_size() > PDS_MAX_SUBNET_POLICY) {
        PDS_TRACE_ERR("No. of IPv4 egress security policies on subnet can't "
                      "exceed {}", PDS_MAX_SUBNET_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_egr_v4_policy = proto_spec.egv4securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_egr_v4_policy; i++) {
        api_spec->egr_v4_policy[i].id = proto_spec.egv4securitypolicyid(i);
    }
    if (proto_spec.egv6securitypolicyid_size() > PDS_MAX_SUBNET_POLICY) {
        PDS_TRACE_ERR("No. of IPv6 egress security policies on subnet can't "
                      "exceed {}", PDS_MAX_SUBNET_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_egr_v6_policy = proto_spec.egv6securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_egr_v6_policy; i++) {
        api_spec->egr_v6_policy[i].id = proto_spec.egv6securitypolicyid(i);
    }
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.fabricencap());
    api_spec->host_ifindex = proto_spec.hostifindex();
    api_spec->dhcp_policy.id = proto_spec.dhcppolicyid();
    api_spec->tos = proto_spec.tos();
    return SDK_RET_OK;
}

// build VPC API spec from protobuf spec
static inline void
pds_vpc_proto_to_api_spec (pds_vpc_spec_t *api_spec,
                           const pds::VPCSpec &proto_spec)
{
    pds::VPCType type;

    api_spec->key.id = proto_spec.id();
    type = proto_spec.type();
    if (type == pds::VPC_TYPE_TENANT) {
        api_spec->type = PDS_VPC_TYPE_TENANT;
    } else if (type == pds::VPC_TYPE_UNDERLAY) {
        api_spec->type = PDS_VPC_TYPE_UNDERLAY;
    }
    ipv6pfx_proto_spec_to_ippfx_api_spec(&api_spec->nat46_prefix,
                                         proto_spec.nat46prefix());
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.fabricencap());
    MAC_UINT64_TO_ADDR(api_spec->vr_mac, proto_spec.virtualroutermac());
    api_spec->v4_route_table.id = proto_spec.v4routetableid();
    api_spec->v6_route_table.id = proto_spec.v6routetableid();
    api_spec->tos = proto_spec.tos();
}

// populate proto buf spec from vpc API spec
static inline void
pds_vpc_api_spec_to_proto (pds::VPCSpec *proto_spec,
                           const pds_vpc_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id);
    if (api_spec->type == PDS_VPC_TYPE_TENANT) {
        proto_spec->set_type(pds::VPC_TYPE_TENANT);
    } else if (api_spec->type == PDS_VPC_TYPE_UNDERLAY) {
        proto_spec->set_type(pds::VPC_TYPE_UNDERLAY);
    }
    ippfx_api_spec_to_ipv6pfx_proto_spec(proto_spec->mutable_nat46prefix(),
                                         &api_spec->nat46_prefix);
    pds_encap_to_proto_encap(proto_spec->mutable_fabricencap(),
                             &api_spec->fabric_encap);
    proto_spec->set_virtualroutermac(MAC_TO_UINT64(api_spec->vr_mac));
    proto_spec->set_v4routetableid(api_spec->v4_route_table.id);
    proto_spec->set_v6routetableid(api_spec->v6_route_table.id);
    proto_spec->set_tos(api_spec->tos);
}

// populate proto buf status from vpc API status
static inline void
pds_vpc_api_status_to_proto (pds::VPCStatus *proto_status,
                             const pds_vpc_status_t *api_status)
{
    proto_status->set_hwid(api_status->hw_id);
}

// populate proto buf stats from vpc API stats
static inline void
pds_vpc_api_stats_to_proto (pds::VPCStats *proto_stats,
                            const pds_vpc_stats_t *api_stats)
{
}

// populate proto buf from vpc API info
static inline void
pds_vpc_api_info_to_proto (const pds_vpc_info_t *api_info, void *ctxt)
{
    pds::VPCGetResponse *proto_rsp = (pds::VPCGetResponse *)ctxt;
    auto vpc = proto_rsp->add_response();
    pds::VPCSpec *proto_spec = vpc->mutable_spec();
    pds::VPCStatus *proto_status = vpc->mutable_status();
    pds::VPCStats *proto_stats = vpc->mutable_stats();

    pds_vpc_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_vpc_api_status_to_proto(proto_status, &api_info->status);
    pds_vpc_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline void
pds_local_mapping_api_spec_to_proto (pds::MappingSpec *proto_spec,
                                     const pds_local_mapping_spec_t *local_spec)
{
    if (!proto_spec || !local_spec) {
        return;
    }

    switch (local_spec->key.type) {
    case PDS_MAPPING_TYPE_L2:
        {
            auto key = proto_spec->mutable_id()->mutable_mackey();
            key->set_macaddr(MAC_TO_UINT64(local_spec->key.mac_addr));
            key->set_subnetid(local_spec->key.subnet.id);
        }
        break;
    case PDS_MAPPING_TYPE_L3:
        {
            auto key = proto_spec->mutable_id()->mutable_ipkey();
            ipaddr_api_spec_to_proto_spec(key->mutable_ipaddr(),
                                          &local_spec->key.ip_addr);
            key->set_vpcid(local_spec->key.vpc.id);
        }
        break;
    default:
        return;
    }

    proto_spec->set_subnetid(local_spec->subnet.id);
    proto_spec->set_macaddr(MAC_TO_UINT64(local_spec->vnic_mac));
    pds_encap_to_proto_encap(proto_spec->mutable_encap(),
                             &local_spec->fabric_encap);
    proto_spec->set_vnicid(local_spec->vnic.id);
    if (local_spec->public_ip_valid) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_publicip(),
                                      &local_spec->public_ip);
    }
    if (local_spec->provider_ip_valid) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_providerip(),
                                      &local_spec->provider_ip);
    }
    for (uint32_t i = 0; i < local_spec->num_tags; i++) {
        proto_spec->add_tags(local_spec->tags[i]);
    }
}

// build API spec from protobuf spec
static inline sdk_ret_t
pds_local_mapping_proto_to_api_spec (pds_local_mapping_spec_t *local_spec,
                                     const pds::MappingSpec &proto_spec)
{
    pds::MappingKey key;

    key = proto_spec.id();
    switch (key.keyinfo_case()) {
    case pds::MappingKey::kIPKey:
        local_spec->key.type = PDS_MAPPING_TYPE_L3;
        local_spec->key.vpc.id = key.ipkey().vpcid();
        ipaddr_proto_spec_to_api_spec(&local_spec->key.ip_addr,
                                      key.ipkey().ipaddr());
        local_spec->subnet.id = proto_spec.subnetid();
        break;

    case pds::MappingKey::kMACKey:
    default:
        PDS_TRACE_ERR("Unsupported local mapping key type %u, local mappings "
                      "can only be L3 mappings", key.keyinfo_case());
        return SDK_RET_INVALID_ARG;
    }
    if (proto_spec.dstinfo_case() != pds::MappingSpec::kVnicId) {
        PDS_TRACE_ERR("Mandatory vnic attribute not set for local mapping");
        return SDK_RET_INVALID_ARG;
    }
    local_spec->vnic.id = proto_spec.vnicid();
    if (proto_spec.has_publicip()) {
        if (proto_spec.publicip().af() == types::IP_AF_INET ||
            proto_spec.publicip().af() == types::IP_AF_INET6) {
            local_spec->public_ip_valid = true;
            ipaddr_proto_spec_to_api_spec(&local_spec->public_ip,
                                          proto_spec.publicip());
        }
    }
    if (proto_spec.has_providerip()) {
        if (proto_spec.providerip().af() == types::IP_AF_INET ||
            proto_spec.providerip().af() == types::IP_AF_INET6) {
            local_spec->provider_ip_valid = true;
            ipaddr_proto_spec_to_api_spec(&local_spec->provider_ip,
                                          proto_spec.providerip());
        }
    }
    MAC_UINT64_TO_ADDR(local_spec->vnic_mac, proto_spec.macaddr());
    local_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.encap());
    local_spec->num_tags = proto_spec.tags_size();
    if (local_spec->num_tags > PDS_MAX_TAGS_PER_MAPPING) {
        PDS_TRACE_ERR("No. of tags {} on local IP mapping exceeded max. "
                      "supported {}", local_spec->num_tags,
                      PDS_MAX_TAGS_PER_MAPPING);
        return SDK_RET_INVALID_ARG;
    }
    for (uint32_t i = 0; i < local_spec->num_tags; i++) {
        local_spec->tags[i] = proto_spec.tags(i);
    }
    return SDK_RET_OK;
}

static inline void
pds_remote_mapping_api_spec_to_proto (pds::MappingSpec *proto_spec,
                                      const pds_remote_mapping_spec_t *remote_spec)
{
    if (!proto_spec || !remote_spec) {
        return;
    }

    switch (remote_spec->key.type) {
    case PDS_MAPPING_TYPE_L2:
        {
            auto key = proto_spec->mutable_id()->mutable_mackey();
            key->set_macaddr(MAC_TO_UINT64(remote_spec->key.mac_addr));
            key->set_subnetid(remote_spec->key.subnet.id);
        }
        break;
    case PDS_MAPPING_TYPE_L3:
        {
            auto key = proto_spec->mutable_id()->mutable_ipkey();
            ipaddr_api_spec_to_proto_spec(key->mutable_ipaddr(),
                                          &remote_spec->key.ip_addr);
            key->set_vpcid(remote_spec->key.vpc.id);
        }
        break;
    default:
        return;
    }
    switch (remote_spec->nh_type) {
    case PDS_NH_TYPE_OVERLAY:
        proto_spec->set_tunnelid(remote_spec->tep.id);
        break;
    case PDS_NH_TYPE_OVERLAY_ECMP:
        proto_spec->set_nexthopgroupid(remote_spec->nh_group.id);
        break;
    default:
        return;
    }
    proto_spec->set_subnetid(remote_spec->subnet.id);
    proto_spec->set_macaddr(MAC_TO_UINT64(remote_spec->vnic_mac));
    pds_encap_to_proto_encap(proto_spec->mutable_encap(), &remote_spec->fabric_encap);
    if (remote_spec->provider_ip_valid) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_providerip(),
                                      &remote_spec->provider_ip);
    }
    for (uint32_t i = 0; i < remote_spec->num_tags; i++) {
        proto_spec->add_tags(remote_spec->tags[i]);
    }
}

// populate proto buf status from mapping API status
static inline void
pds_mapping_api_status_to_proto (pds::MappingStatus *proto_status,
                                 const pds_mapping_status_t *api_status)
{
}

// populate proto buf stats from mapping API stats
static inline void
pds_mapping_api_stats_to_proto (pds::MappingStats *proto_stats,
                                const pds_mapping_stats_t *api_stats)
{
}

// populate proto buf from local mapping API info
static inline void
pds_local_mapping_api_info_to_proto (const pds_local_mapping_info_t *api_info , void *ctxt)
{
    pds::MappingGetResponse *proto_rsp = (pds::MappingGetResponse *)ctxt;
    auto mapping = proto_rsp->add_response();
    pds::MappingSpec *proto_spec = mapping->mutable_spec();
    pds::MappingStatus *proto_status = mapping->mutable_status();
    pds::MappingStats *proto_stats = mapping->mutable_stats();

    pds_local_mapping_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_mapping_api_status_to_proto(proto_status, &api_info->status);
    pds_mapping_api_stats_to_proto(proto_stats, &api_info->stats);
}

// populate proto buf from remote mapping API info
static inline void
pds_remote_mapping_api_info_to_proto (const pds_remote_mapping_info_t *api_info , void *ctxt)
{
    pds::MappingGetResponse *proto_rsp = (pds::MappingGetResponse *)ctxt;
    auto mapping = proto_rsp->add_response();
    pds::MappingSpec *proto_spec = mapping->mutable_spec();
    pds::MappingStatus *proto_status = mapping->mutable_status();
    pds::MappingStats *proto_stats = mapping->mutable_stats();

    pds_remote_mapping_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_mapping_api_status_to_proto(proto_status, &api_info->status);
    pds_mapping_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_remote_mapping_proto_to_api_spec (pds_remote_mapping_spec_t *remote_spec,
                                      const pds::MappingSpec &proto_spec)
{
    pds::MappingKey key;

    key = proto_spec.id();
    switch (key.keyinfo_case()) {
    case pds::MappingKey::kMACKey:
        remote_spec->key.type = PDS_MAPPING_TYPE_L2;
        remote_spec->key.subnet.id = key.mackey().subnetid();
        MAC_UINT64_TO_ADDR(remote_spec->key.mac_addr,
                           key.mackey().macaddr());
        remote_spec->subnet.id = remote_spec->key.subnet.id;
        break;

    case pds::MappingKey::kIPKey:
        remote_spec->key.type = PDS_MAPPING_TYPE_L3;
        remote_spec->key.vpc.id = key.ipkey().vpcid();
        ipaddr_proto_spec_to_api_spec(&remote_spec->key.ip_addr,
                                      key.ipkey().ipaddr());
        remote_spec->subnet.id = proto_spec.subnetid();
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }

    switch (proto_spec.dstinfo_case()) {
    case pds::MappingSpec::kTunnelId:
        remote_spec->nh_type = PDS_NH_TYPE_OVERLAY;
        remote_spec->tep.id = proto_spec.tunnelid();
        break;

    case pds::MappingSpec::kNexthopGroupId:
        remote_spec->nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
        remote_spec->nh_group.id = proto_spec.nexthopgroupid();
        break;

    case pds::MappingSpec::kVnicId:
    default:
        PDS_TRACE_ERR("Usage of vnic attribute is invalid for remote mappings");
        return SDK_RET_INVALID_ARG;
    }
    MAC_UINT64_TO_ADDR(remote_spec->vnic_mac, proto_spec.macaddr());
    remote_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.encap());
    if (proto_spec.has_providerip()) {
        if (proto_spec.providerip().af() == types::IP_AF_INET ||
            proto_spec.providerip().af() == types::IP_AF_INET6) {
            remote_spec->provider_ip_valid = true;
            ipaddr_proto_spec_to_api_spec(&remote_spec->provider_ip,
                                          proto_spec.providerip());
        }
    }
    remote_spec->num_tags = proto_spec.tags_size();
    if (remote_spec->num_tags > PDS_MAX_TAGS_PER_MAPPING) {
        PDS_TRACE_ERR("No. of tags {} on remote IP mapping exceeded max. "
                      "supported {}", remote_spec->num_tags,
                      PDS_MAX_TAGS_PER_MAPPING);
        return SDK_RET_INVALID_ARG;
    }
    for (uint32_t i = 0; i < remote_spec->num_tags; i++) {
        remote_spec->tags[i] = proto_spec.tags(i);
    }
    return SDK_RET_OK;
}

static inline void
pds_port_stats_to_proto (pds::PortStats *stats,
                         sdk::linkmgr::port_args_t *port_info)
{
    if (port_info->port_type == port_type_t::PORT_TYPE_ETH) {
        for (uint32_t i = 0; i < MAX_MAC_STATS; i++) {
            auto macstats = stats->add_macstats();
            macstats->set_type(pds::MacStatsType(i));
            macstats->set_count(port_info->stats_data[i]);
        }
    } else if (port_info->port_type == port_type_t::PORT_TYPE_MGMT) {
        for (uint32_t i = 0; i < MAX_MGMT_MAC_STATS; i++) {
            auto macstats = stats->add_mgmtmacstats();
            macstats->set_type(pds::MgmtMacStatsType(i));
            macstats->set_count(port_info->stats_data[i]);
        }
    }
}

static inline void
pds_port_spec_to_proto (pds::PortSpec *spec,
                        sdk::linkmgr::port_args_t *port_info)
{
    spec->set_id(port_info->port_num);
    switch(port_info->admin_state) {
    case port_admin_state_t::PORT_ADMIN_STATE_DOWN:
        spec->set_adminstate(pds::PORT_ADMIN_STATE_DOWN);
        break;
    case port_admin_state_t::PORT_ADMIN_STATE_UP:
        spec->set_adminstate(pds::PORT_ADMIN_STATE_UP);
        break;
    default:
        spec->set_adminstate(pds::PORT_ADMIN_STATE_NONE);
        break;
    }
    spec->set_speed(pds_port_sdk_speed_to_proto_speed(
                                      port_info->port_speed));
    spec->set_fectype(pds_port_sdk_fec_type_to_proto_fec_type
                                      (port_info->fec_type));
    spec->set_autonegen(port_info->auto_neg_cfg);
    spec->set_debouncetimeout(port_info->debounce_time);
    spec->set_mtu(port_info->mtu);
    spec->set_pausetype(pds_port_sdk_pause_type_to_proto_pause_type
                                      (port_info->pause));
    spec->set_loopbackmode(pds_port_sdk_loopback_mode_to_proto_loopback_mode(
                                     port_info->loopback_mode));
    spec->set_numlanes(port_info->num_lanes_cfg);
}

static inline void
pds_port_status_to_proto (pds::PortStatus *status,
                          sdk::linkmgr::port_args_t *port_info)
{
    auto link_status = status->mutable_linkstatus();
    auto xcvr_status = status->mutable_xcvrstatus();

    switch (port_info->oper_status) {
    case port_oper_status_t::PORT_OPER_STATUS_UP:
        link_status->set_operstate(pds::PORT_OPER_STATUS_UP);
        break;
    case port_oper_status_t::PORT_OPER_STATUS_DOWN:
        link_status->set_operstate(pds::PORT_OPER_STATUS_DOWN);
        break;
    default:
        link_status->set_operstate(pds::PORT_OPER_STATUS_NONE);
        break;
    }

    switch (port_info->port_speed) {
    case port_speed_t::PORT_SPEED_1G:
        link_status->set_portspeed(pds::PORT_SPEED_1G);
        break;
    case port_speed_t::PORT_SPEED_10G:
        link_status->set_portspeed(pds::PORT_SPEED_10G);
        break;
    case port_speed_t::PORT_SPEED_25G:
        link_status->set_portspeed(pds::PORT_SPEED_25G);
        break;
    case port_speed_t::PORT_SPEED_40G:
        link_status->set_portspeed(pds::PORT_SPEED_40G);
        break;
    case port_speed_t::PORT_SPEED_50G:
        link_status->set_portspeed(pds::PORT_SPEED_50G);
        break;
    case port_speed_t::PORT_SPEED_100G:
        link_status->set_portspeed(pds::PORT_SPEED_100G);
        break;
    default:
        link_status->set_portspeed(pds::PORT_SPEED_NONE);
        break;
    }
    link_status->set_autonegen(port_info->auto_neg_enable);
    link_status->set_numlanes(port_info->num_lanes);

    xcvr_status->set_port(port_info->xcvr_event_info.phy_port);
    xcvr_status->set_state(pds::PortXcvrState(port_info->xcvr_event_info.state));
    xcvr_status->set_pid(pds::PortXcvrPid(port_info->xcvr_event_info.pid));
    xcvr_status->set_mediatype(pds::MediaType(port_info->xcvr_event_info.cable_type));
    xcvr_status->set_xcvrsprom(std::string((char*)&port_info->xcvr_event_info.xcvr_sprom));
}

static inline void
pds_port_to_proto (sdk::linkmgr::port_args_t *port_info, void *ctxt)
{
    pds::PortGetResponse *rsp = (pds::PortGetResponse *)ctxt;
    pds::Port *port = rsp->add_response();
    pds::PortSpec *spec = port->mutable_spec();
    pds::PortStats *stats = port->mutable_stats();
    pds::PortStatus *status = port->mutable_status();

    pds_port_spec_to_proto(spec, port_info);
    pds_port_stats_to_proto(stats, port_info);
    pds_port_status_to_proto(status, port_info);
}

#endif    // __AGENT_SVC_SPECS_HPP__
