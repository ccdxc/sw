//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for vpc object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_VPC_SVC_HPP__
#define __AGENT_SVC_VPC_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"

// build VPC API spec from protobuf spec
static inline void
pds_vpc_proto_to_api_spec (pds_vpc_spec_t *api_spec,
                           const pds::VPCSpec &proto_spec)
{
    pds::VPCType type;

    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    type = proto_spec.type();
    if (type == pds::VPC_TYPE_TENANT) {
        api_spec->type = PDS_VPC_TYPE_TENANT;
    } else if (type == pds::VPC_TYPE_UNDERLAY) {
        api_spec->type = PDS_VPC_TYPE_UNDERLAY;
    } else if (type == pds::VPC_TYPE_CONTROL) {
        api_spec->type = PDS_VPC_TYPE_CONTROL;
    }
    ipv6pfx_proto_spec_to_ippfx_api_spec(&api_spec->nat46_prefix,
                                         proto_spec.nat46prefix());
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.fabricencap());
    MAC_UINT64_TO_ADDR(api_spec->vr_mac, proto_spec.virtualroutermac());
    pds_obj_key_proto_to_api_spec(&api_spec->v4_route_table,
                                  proto_spec.v4routetableid());
    pds_obj_key_proto_to_api_spec(&api_spec->v6_route_table,
                                  proto_spec.v6routetableid());
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
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    if (api_spec->type == PDS_VPC_TYPE_TENANT) {
        proto_spec->set_type(pds::VPC_TYPE_TENANT);
    } else if (api_spec->type == PDS_VPC_TYPE_UNDERLAY) {
        proto_spec->set_type(pds::VPC_TYPE_UNDERLAY);
    } else if (api_spec->type == PDS_VPC_TYPE_CONTROL) {
        proto_spec->set_type(pds::VPC_TYPE_CONTROL);
    }
    ippfx_api_spec_to_ipv6pfx_proto_spec(proto_spec->mutable_nat46prefix(),
                                         &api_spec->nat46_prefix);
    pds_encap_to_proto_encap(proto_spec->mutable_fabricencap(),
                             &api_spec->fabric_encap);
    proto_spec->set_virtualroutermac(MAC_TO_UINT64(api_spec->vr_mac));
    proto_spec->set_v4routetableid(api_spec->v4_route_table.id,
                                   PDS_MAX_KEY_LEN);
    proto_spec->set_v6routetableid(api_spec->v6_route_table.id,
                                   PDS_MAX_KEY_LEN);
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

// populate VPCPeer protobuf spec from VPCPeer API spec
static inline void
pds_vpc_peer_api_spec_to_proto (pds::VPCPeerSpec *proto_spec,
                                const pds_vpc_peer_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpc1(api_spec->vpc1.id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpc2(api_spec->vpc2.id, PDS_MAX_KEY_LEN);
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
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->vpc1, proto_spec.vpc1());
    pds_obj_key_proto_to_api_spec(&api_spec->vpc2, proto_spec.vpc2());
}

#endif //__AGENT_SVC_VPC_SVC_HPP__
