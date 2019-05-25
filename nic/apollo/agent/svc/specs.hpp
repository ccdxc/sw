//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __AGENT_SVC_SPECS_HPP__
#define __AGENT_SVC_SPECS_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/meter.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/meter.hpp"
#include "nic/apollo/agent/svc/tag.hpp"
#include "nic/apollo/agent/svc/vnic.hpp"
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

#endif    // __AGENT_SVC_SPECS_HPP__
