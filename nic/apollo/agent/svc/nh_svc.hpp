//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for nh object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_NH_SVC_HPP__
#define __AGENT_SVC_NH_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"

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
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    switch (proto_spec.nhinfo_case()) {
    case pds::NexthopSpec::kIPNhInfo:
        api_spec->type = PDS_NH_TYPE_IP;
        pds_obj_key_proto_to_api_spec(&api_spec->vpc,
                                      proto_spec.ipnhinfo().vpcid());
        ipaddr_proto_spec_to_api_spec(&api_spec->ip,
                                      proto_spec.ipnhinfo().ip());
        api_spec->vlan = proto_spec.ipnhinfo().vlan();
        if (proto_spec.ipnhinfo().mac() != 0) {
            MAC_UINT64_TO_ADDR(api_spec->mac, proto_spec.ipnhinfo().mac());
        }
        break;

    case pds::NexthopSpec::kTunnelId:
        api_spec->type = PDS_NH_TYPE_OVERLAY;
        pds_obj_key_proto_to_api_spec(&api_spec->tep, proto_spec.tunnelid());
        break;

    case pds::NexthopSpec::kUnderlayNhInfo:
        api_spec->type = PDS_NH_TYPE_UNDERLAY;
        pds_obj_key_proto_to_api_spec(&api_spec->l3_if,
                                      proto_spec.underlaynhinfo().l3interface());
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
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    if (api_spec->type == PDS_NH_TYPE_IP) {
        auto ipnhinfo = proto_spec->mutable_ipnhinfo();
        ipnhinfo->set_vpcid(api_spec->vpc.id, PDS_MAX_KEY_LEN);
        ipaddr_api_spec_to_proto_spec(ipnhinfo->mutable_ip(), &api_spec->ip);
        ipnhinfo->set_vlan(api_spec->vlan);
        ipnhinfo->set_mac(MAC_TO_UINT64(api_spec->mac));
    } else if (api_spec->type == PDS_NH_TYPE_OVERLAY) {
        proto_spec->set_tunnelid(api_spec->tep.id, PDS_MAX_KEY_LEN);
    } else if (api_spec->type == PDS_NH_TYPE_UNDERLAY) {
        auto underlayinfo = proto_spec->mutable_underlaynhinfo();
        underlayinfo->set_l3interface(api_spec->l3_if.id, PDS_MAX_KEY_LEN);
        underlayinfo->set_underlaymac(MAC_TO_UINT64(api_spec->underlay_mac));
    }
}

// populate proto buf status from nh API status
static inline void
pds_nh_api_status_to_proto (pds::NexthopStatus *proto_status,
                            const pds_nexthop_status_t *api_status,
                            const pds_nexthop_spec_t *api_spec)
{
    proto_status->set_hwid(api_status->hw_id);
    switch (api_spec->type) {
    case PDS_NH_TYPE_UNDERLAY:
        {
            auto status = proto_status->mutable_underlaynhinfo();
            status->set_port(api_status->port);
            status->set_vlan(api_status->vlan);
        }
        break;
    default:
        break;
    }
}

// populate proto buf stats from nh API stats
static inline void
pds_nh_api_stats_to_proto (pds::NexthopStats *proto_stats,
                           const pds_nexthop_stats_t *api_stats)
{
}

// populate proto buf from nh API info
static inline void
pds_nh_api_info_to_proto (pds_nexthop_info_t *api_info, void *ctxt)
{
    nh_get_all_args_t *args = (nh_get_all_args_t *)ctxt;
    pds::NexthopGetResponse *proto_rsp = (pds::NexthopGetResponse *)(args->ctxt);

    if (api_info->spec.type != args->type) {
        return;
    }

    auto nh = proto_rsp->add_response();
    pds::NexthopSpec *proto_spec = nh->mutable_spec();
    pds::NexthopStatus *proto_status = nh->mutable_status();
    pds::NexthopStats *proto_stats = nh->mutable_stats();

    pds_nh_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_nh_api_status_to_proto(proto_status, &api_info->status, &api_info->spec);
    pds_nh_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build nh group API spec from protobuf spec
static inline sdk_ret_t
pds_nh_group_proto_to_api_spec (pds_nexthop_group_spec_t *api_spec,
                                const pds::NhGroupSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
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
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
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
                                  const pds_nexthop_group_status_t *api_status,
                                  const pds_nexthop_group_spec_t *api_spec)
{
    proto_status->set_hwid(api_status->hw_id);

    for (uint32_t i = 0; i < api_spec->num_nexthops; i++) {
        pds_nh_api_status_to_proto(proto_status->add_members(),
                                   &api_status->nexthops[i],
                                   &api_spec->nexthops[i]);
    }
}

// populate proto buf stats from nh group API stats
static inline void
pds_nh_group_api_stats_to_proto (pds::NhGroupStats *proto_stats,
                                 const pds_nexthop_group_stats_t *api_stats)
{
}

// populate proto buf from nh API info
static inline void
pds_nh_group_api_info_to_proto (pds_nexthop_group_info_t *api_info,
                                void *ctxt)
{
    pds::NhGroupGetResponse *proto_rsp = (pds::NhGroupGetResponse *)ctxt;
    auto nh = proto_rsp->add_response();
    pds::NhGroupSpec *proto_spec = nh->mutable_spec();
    pds::NhGroupStatus *proto_status = nh->mutable_status();
    pds::NhGroupStats *proto_stats = nh->mutable_stats();

    pds_nh_group_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_nh_group_api_status_to_proto(proto_status, &api_info->status, &api_info->spec);
    pds_nh_group_api_stats_to_proto(proto_stats, &api_info->stats);
}

#endif    //__AGENT_SVC_NH_SVC_HPP__
