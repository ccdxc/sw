//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __AGENT_SVC_SPECS_HPP__
#define __AGENT_SVC_SPECS_HPP__

#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/meter.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/meter.hpp"
#include "nic/apollo/agent/svc/tag.hpp"
#include "nic/apollo/agent/svc/vnic.hpp"
#include "nic/apollo/agent/svc/vpc.hpp"
#include "nic/apollo/agent/svc/tunnel.hpp"
#include "nic/apollo/agent/svc/service.hpp"
#include "nic/apollo/agent/svc/port.hpp"
#include "gen/proto/types.pb.h"

//----------------------------------------------------------------------------
// convert HAL IP address to spec
//----------------------------------------------------------------------------
static inline void
ip_addr_to_spec (types::IPAddress *ip_addr_spec,
                 ip_addr_t *ip_addr)
{
    if (ip_addr->af == IP_AF_IPV4) {
        ip_addr_spec->set_af(types::IP_AF_INET);
        ip_addr_spec->set_v4addr(ip_addr->addr.v4_addr);
    } else if (ip_addr->af == IP_AF_IPV6) {
        ip_addr_spec->set_af(types::IP_AF_INET6);
        ip_addr_spec->set_v6addr(ip_addr->addr.v6_addr.addr8, IP6_ADDR8_LEN);
    }
}

//----------------------------------------------------------------------------
// convert HAL IPv4 address to spec
//----------------------------------------------------------------------------
static inline void
ipv4_addr_to_spec (types::IPAddress *ip_addr_spec,
                   ipv4_addr_t *ipv4_addr)
{
    ip_addr_spec->set_af(types::IP_AF_INET);
    ip_addr_spec->set_v4addr(*ipv4_addr);
}

//----------------------------------------------------------------------------
// convert IPv4 prefix to IPPrefix proto spec
//----------------------------------------------------------------------------
static inline void
ipv4_pfx_to_spec (types::IPPrefix *ip_pfx_spec,
                ipv4_prefix_t *ip_pfx)
{
    ip_pfx_spec->set_len(ip_pfx->len);
    ipv4_addr_to_spec(ip_pfx_spec->mutable_addr(), &ip_pfx->v4_addr);
}

//----------------------------------------------------------------------------
// convert IP prefix to IPPrefix proto spec
//----------------------------------------------------------------------------
static inline void
ip_pfx_to_spec (types::IPPrefix *ip_pfx_spec,
                ip_prefix_t *ip_pfx)
{
    ip_pfx_spec->set_len(ip_pfx->len);
    ip_addr_to_spec(ip_pfx_spec->mutable_addr(), &ip_pfx->addr);
}

// Populate proto buf spec from meter API spec
static inline void
meter_api_spec_to_proto_spec (const pds_meter_spec_t *api_spec,
                              pds::MeterSpec *proto_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id);
    if (api_spec->af == IP_AF_IPV4) {
        proto_spec->set_af(types::IP_AF_INET);
    } else if (api_spec->af == IP_AF_IPV6) {
        proto_spec->set_af(types::IP_AF_INET6);
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
            ip_pfx_to_spec(proto_rule_spec->add_prefix(),
                           &api_rule_spec->prefixes[pfx]);
        }
    }
    return;
}

// Populate proto buf spec from tag API spec
static inline void
tag_api_spec_to_proto_spec (const pds_tag_spec_t *api_spec,
                            pds::TagSpec *proto_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_af(pds_af_api_spec_to_proto_spec(api_spec->af));
    for (uint32_t i = 0; i < api_spec->num_rules; i ++) {
        auto rule = proto_spec->add_rules();
        rule->set_tag(api_spec->rules[i].tag);
        for (uint32_t j = 0; j < api_spec->rules[i].num_prefixes; j ++) {
            ippfx_api_spec_to_proto_spec(
                    rule->add_prefix(), &api_spec->rules[i].prefixes[j]);
        }
    }
}

// Populate proto buf spec from vnic API spec
static inline void
vnic_api_spec_to_proto_spec (const pds_vnic_spec_t *api_spec,
                             pds::VnicSpec *proto_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_vnicid(api_spec->key.id);
    proto_spec->set_vpcid(api_spec->vpc.id);
    proto_spec->set_subnetid(api_spec->subnet.id);
    pds_encap_to_proto_encap(proto_spec->mutable_vnicencap(),
                             &api_spec->vnic_encap);
    proto_spec->set_macaddress(MAC_TO_UINT64(api_spec->mac_addr));
    //proto_spec->set_providermacaddress(
                        //MAC_TO_UINT64(api_spec->provider_mac_addr));
    pds_encap_to_proto_encap(proto_spec->mutable_fabricencap(),
                             &api_spec->fabric_encap);
    proto_spec->set_resourcepoolid(api_spec->rsc_pool_id);
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
}

// Populate VPCPeer protobuf spec from VPCPeer API spec
static inline void
vpc_peer_api_spec_to_proto_spec (pds::VPCPeerSpec *proto_spec,
                                 const pds_vpc_peer_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_vpc1(api_spec->vpc1.id);
    proto_spec->set_vpc2(api_spec->vpc2.id);
}

// Populate VPCPeer api spec from VPCPeer proto spec
static inline void
pds_agent_vpc_peer_api_spec_fill (pds_vpc_peer_spec_t *api_spec,
                                  const pds::VPCPeerSpec &proto_spec)
{
    if (!api_spec) {
        return;
    }
    api_spec->key.id = proto_spec.id();
    api_spec->vpc1.id = proto_spec.vpc1();
    api_spec->vpc2.id = proto_spec.vpc2();
}

// Populate proto buf spec from tep API spec
static inline void
tep_api_spec_to_proto_spec (pds::TunnelSpec *proto_spec,
                            const pds_tep_spec_t *api_spec)
{
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_remoteip(),
                                  &api_spec->key.ip_addr);
    switch (api_spec->type) {
    case PDS_TEP_TYPE_WORKLOAD:
        proto_spec->set_type(pds::TUNNEL_TYPE_WORKLOAD);
        break;
    case PDS_TEP_TYPE_IGW:
        proto_spec->set_type(pds::TUNNEL_TYPE_IGW);
        break;
    case PDS_TEP_TYPE_SERVICE:
        proto_spec->set_type(pds::TUNNEL_TYPE_SERVICE);
        break;
    case PDS_TEP_TYPE_NONE:
    default:
        proto_spec->set_type(pds::TUNNEL_TYPE_NONE);
        break;
    }
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_localip(),
                                  &api_spec->ip_addr);
    proto_spec->set_macaddress(MAC_TO_UINT64(api_spec->mac));
    pds_encap_to_proto_encap(proto_spec->mutable_encap(),
                             &api_spec->encap);
    proto_spec->set_nat(api_spec->nat);
}

// Populate proto buf spec from service API spec
static inline void
service_api_spec_to_proto_spec (pds::SvcMappingSpec *proto_spec,
                                const pds_svc_mapping_spec_t *api_spec)
{
    auto proto_key = proto_spec->mutable_key();
    proto_key->set_vpcid(api_spec->key.vpc.id);
    proto_key->set_svcport(api_spec->key.svc_port);
    ipaddr_api_spec_to_proto_spec(
                proto_key->mutable_ipaddr(), &api_spec->key.vip);
    ipaddr_api_spec_to_proto_spec(
                proto_spec->mutable_privateip(), &api_spec->backend_ip);
    ipaddr_api_spec_to_proto_spec(
                proto_spec->mutable_providerip(), &api_spec->backend_provider_ip);
    proto_spec->set_vpcid(api_spec->vpc.id);
    proto_spec->set_port(api_spec->svc_port);
}

// Populate proto buf status from service API status
static inline void
service_api_status_to_proto_status (pds::SvcMappingStatus *proto_status,
                                    const pds_svc_mapping_status_t *api_status)
{
}

// Populate proto buf stats from service API stats
static inline void
service_api_stats_to_proto_stats (pds::SvcMappingStats *proto_stats,
                                  const pds_svc_mapping_stats_t *api_stats)
{
}

// Populate proto buf from service API info
static inline void
service_api_info_to_proto (const pds_svc_mapping_info_t *api_info, void *ctxt)
{
    pds::SvcMappingGetResponse *proto_rsp = (pds::SvcMappingGetResponse *)ctxt;
    auto service = proto_rsp->add_response();
    pds::SvcMappingSpec *proto_spec = service->mutable_spec();
    pds::SvcMappingStatus *proto_status = service->mutable_status();
    pds::SvcMappingStats *proto_stats = service->mutable_stats();

    service_api_spec_to_proto_spec(proto_spec, &api_info->spec);
    service_api_status_to_proto_status(proto_status, &api_info->status);
    service_api_stats_to_proto_stats(proto_stats, &api_info->stats);
}

// Build service API spec from proto buf spec
static inline void
service_proto_spec_to_api_spec (pds_svc_mapping_spec_t *api_spec,
                                const pds::SvcMappingSpec &proto_spec)
{
    api_spec->key.vpc.id = proto_spec.key().vpcid();
    api_spec->key.svc_port = proto_spec.key().svcport();
    ipaddr_proto_spec_to_api_spec(&api_spec->key.vip, proto_spec.key().ipaddr());
    ipaddr_proto_spec_to_api_spec(&api_spec->backend_ip, proto_spec.privateip());
    ipaddr_proto_spec_to_api_spec(&api_spec->backend_provider_ip, proto_spec.providerip());
    api_spec->svc_port = proto_spec.port();
    api_spec->vpc.id = proto_spec.vpcid();
}

static inline port_fec_type_t
proto_port_fec_type_to_sdk_fec_type (pds::PortFecType fec_type)
{
    switch (fec_type) {
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

static inline port_speed_t
proto_port_speed_to_sdk_port_speed (pds::PortSpeed port_speed)
{
    switch (port_speed) {
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

static inline port_admin_state_t
proto_port_admin_state_to_sdk_admin_state (pds::PortAdminState proto_state)
{
    switch (proto_state) {
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

static inline void
proto_port_spec_to_port_args (port_args_t *port_args,
                              const pds::PortSpec &spec)
{
    port_args->admin_state = proto_port_admin_state_to_sdk_admin_state(spec.adminstate());
    port_args->port_speed = proto_port_speed_to_sdk_port_speed(spec.speed());
    port_args->auto_neg_enable = spec.autonegen();
    port_args->fec_type = proto_port_fec_type_to_sdk_fec_type(spec.fectype());
}

#endif    // __AGENT_SVC_SPECS_HPP__
