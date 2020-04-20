//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for route object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_ROUTE_SVC_HPP__
#define __AGENT_SVC_ROUTE_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/route.hpp"

static inline sdk_ret_t
pds_route_attrs_proto_to_api_spec (pds_route_attrs_t *route_attrs,
                                   const pds::RouteAttrs &proto_route_attrs)
{
    ippfx_proto_spec_to_api_spec(&route_attrs->prefix,
                                 proto_route_attrs.prefix());
    route_attrs->prio = proto_route_attrs.priority();
    switch (proto_route_attrs.nh_case()) {
    case pds::RouteAttrs::kNextHop:
    case pds::RouteAttrs::kTunnelId:
        route_attrs->nh_type = PDS_NH_TYPE_OVERLAY;
        pds_obj_key_proto_to_api_spec(&route_attrs->tep,
                                      proto_route_attrs.tunnelid());
        break;
    case pds::RouteAttrs::kNexthopGroupId:
        // NOTE: UNDERLAY_ECMP is not done in the datapath
        route_attrs->nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
        pds_obj_key_proto_to_api_spec(&route_attrs->nh_group,
                                      proto_route_attrs.nexthopgroupid());
        break;
    case pds::RouteAttrs::kVPCId:
        pds_obj_key_proto_to_api_spec(&route_attrs->vpc,
                                      proto_route_attrs.vpcid());
        route_attrs->nh_type = PDS_NH_TYPE_PEER_VPC;
        break;
    case pds::RouteAttrs::kVnicId:
        pds_obj_key_proto_to_api_spec(&route_attrs->vnic,
                                      proto_route_attrs.vnicid());
        route_attrs->nh_type = PDS_NH_TYPE_VNIC;
        break;
    case pds::RouteAttrs::kNexthopId:
        route_attrs->nh_type = PDS_NH_TYPE_IP;
        pds_obj_key_proto_to_api_spec(&route_attrs->nh,
                                      proto_route_attrs.nexthopid());
        break;
    default:
        route_attrs->nh_type = PDS_NH_TYPE_BLACKHOLE;
        break;
    }
    if (proto_route_attrs.has_nataction()) {
        auto nat_action = proto_route_attrs.nataction();
        switch (nat_action.srcnataction()) {
        case types::NAT_ACTION_STATIC:
            route_attrs->nat.src_nat_type =
                PDS_NAT_TYPE_STATIC;
            break;
        case types::NAT_ACTION_NAPT_PUBLIC:
            route_attrs->nat.src_nat_type =
                PDS_NAT_TYPE_NAPT_PUBLIC;
            break;
        case types::NAT_ACTION_NAPT_SVC:
            route_attrs->nat.src_nat_type =
                PDS_NAT_TYPE_NAPT_SVC;
            break;
        case types::NAT_ACTION_NONE:
        default:
            route_attrs->nat.src_nat_type =
                PDS_NAT_TYPE_NONE;
            break;
        }
        if (nat_action.has_dstnatip()) {
            ipaddr_proto_spec_to_api_spec(
                &route_attrs->nat.dst_nat_ip,
                nat_action.dstnatip());
        } else {
            memset(&route_attrs->nat.dst_nat_ip, 0,
                   sizeof(route_attrs->nat.dst_nat_ip));
        }
    }
    route_attrs->meter = proto_route_attrs.meteren();
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_route_table_proto_to_api_spec (pds_route_table_spec_t *api_spec,
                                   const pds::RouteTableSpec &proto_spec)
{
    uint32_t num_routes = 0;

    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    num_routes = proto_spec.routes_size();
    api_spec->route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(num_routes));
    if (unlikely(api_spec->route_info == NULL)) {
        PDS_TRACE_ERR("Failed to allocate memory for route table {}",
                      api_spec->key.id);
        return sdk::SDK_RET_OOM;
    }
    switch (proto_spec.af()) {
    case types::IP_AF_INET:
        api_spec->route_info->af = IP_AF_IPV4;
        break;

    case types::IP_AF_INET6:
        api_spec->route_info->af = IP_AF_IPV6;
        break;

    default:
        SDK_ASSERT(FALSE);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->route_info->priority_en = proto_spec.priorityen();
    api_spec->route_info->num_routes = num_routes;
    for (uint32_t i = 0; i < num_routes; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->route_info->routes[i].key,
                                      proto_spec.routes(i).id());
        pds_route_attrs_proto_to_api_spec(
            &api_spec->route_info->routes[i].attrs,
            proto_spec.routes(i).attrs());
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_route_attrs_api_spec_to_proto (pds::RouteAttrs *proto_route_attrs,
                                   const pds_route_attrs_t *route_attrs)
{
    ippfx_api_spec_to_proto_spec(proto_route_attrs->mutable_prefix(),
                                 &route_attrs->prefix);
    proto_route_attrs->set_priority(route_attrs->prio);
    switch (route_attrs->nh_type) {
    case PDS_NH_TYPE_OVERLAY:
        proto_route_attrs->set_tunnelid(route_attrs->tep.id,
                            PDS_MAX_KEY_LEN);
        break;
    case PDS_NH_TYPE_OVERLAY_ECMP:
        proto_route_attrs->set_nexthopgroupid(route_attrs->nh_group.id,
                                  PDS_MAX_KEY_LEN);
        break;
    case PDS_NH_TYPE_PEER_VPC:
        proto_route_attrs->set_vpcid(route_attrs->vpc.id,
                         PDS_MAX_KEY_LEN);
        break;
    case PDS_NH_TYPE_VNIC:
        proto_route_attrs->set_vnicid(route_attrs->vnic.id,
                          PDS_MAX_KEY_LEN);
        break;
    case PDS_NH_TYPE_IP:
        proto_route_attrs->set_nexthopid(route_attrs->nh.id,
                             PDS_MAX_KEY_LEN);
        break;
    case PDS_NH_TYPE_BLACKHOLE:
    default:
        // blackhole nexthop
        break;
    }
    switch (route_attrs->nat.src_nat_type) {
    case PDS_NAT_TYPE_STATIC:
        proto_route_attrs->mutable_nataction()->set_srcnataction(types::NAT_ACTION_STATIC);
        break;
    case PDS_NAT_TYPE_NAPT_PUBLIC:
        proto_route_attrs->mutable_nataction()->set_srcnataction(types::NAT_ACTION_NAPT_PUBLIC);
        break;
    case PDS_NAT_TYPE_NAPT_SVC:
        proto_route_attrs->mutable_nataction()->set_srcnataction(types::NAT_ACTION_NAPT_SVC);
        break;
    case PDS_NAT_TYPE_NONE:
    default:
        proto_route_attrs->mutable_nataction()->set_srcnataction(types::NAT_ACTION_NONE);
        break;
    }
    if (!ip_addr_is_zero(&route_attrs->nat.dst_nat_ip)) {
        ipaddr_api_spec_to_proto_spec(
            proto_route_attrs->mutable_nataction()->mutable_dstnatip(),
            &route_attrs->nat.dst_nat_ip);
    }
    proto_route_attrs->set_meteren(route_attrs->meter);
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
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    if (api_spec->route_info == NULL) {
        return;
    }
    if (api_spec->route_info->af == IP_AF_IPV4) {
        proto_spec->set_af(types::IP_AF_INET);
    } else if (api_spec->route_info->af == IP_AF_IPV6) {
        proto_spec->set_af(types::IP_AF_INET6);
    } else {
        SDK_ASSERT(FALSE);
    }
    proto_spec->set_priorityen(api_spec->route_info->priority_en);

    for (uint32_t i = 0; i < api_spec->route_info->num_routes; i++) {
        pds::RouteInfo *route = proto_spec->add_routes();
        route->set_id(api_spec->route_info->routes[i].key.id, PDS_MAX_KEY_LEN);
        pds_route_attrs_api_spec_to_proto(route->mutable_attrs(),
            &api_spec->route_info->routes[i].attrs);
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
pds_route_table_api_info_to_proto (pds_route_table_info_t *api_info,
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

static inline sdk_ret_t
pds_route_proto_to_api_spec (pds_route_spec_t *api_spec,
                             const pds::RouteSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key.route_id, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->key.route_table_id,
                                  proto_spec.routetableid());
    return pds_route_attrs_proto_to_api_spec(&api_spec->attrs,
                                             proto_spec.attrs());
}

static inline sdk_ret_t
pds_route_api_spec_to_proto (pds::RouteSpec *proto_spec,
                             const pds_route_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return SDK_RET_INVALID_ARG;
    }
    proto_spec->set_id(api_spec->key.route_id.id, PDS_MAX_KEY_LEN);
    proto_spec->set_routetableid(api_spec->key.route_table_id.id,
                                 PDS_MAX_KEY_LEN);
    pds_route_attrs_api_spec_to_proto(proto_spec->mutable_attrs(),
                                      &api_spec->attrs);
    return SDK_RET_OK;
}

static inline void
pds_route_api_status_to_proto (pds::RouteStatus *proto_status,
                               const pds_route_status_t *api_status)
{
}

static inline void
pds_route_api_stats_to_proto (pds::RouteStats *proto_stats,
                              const pds_route_stats_t *api_stats)
{
}

static inline void
pds_route_api_info_to_proto (pds_route_info_t *api_info, void *ctxt)
{
    pds::RouteGetResponse *proto_rsp = (pds::RouteGetResponse *)ctxt;
    auto route = proto_rsp->add_response();
    pds::RouteSpec *proto_spec = route->mutable_spec();
    pds::RouteStatus *proto_status = route->mutable_status();
    pds::RouteStats *proto_stats = route->mutable_stats();

    pds_route_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_route_api_status_to_proto(proto_status, &api_info->status);
    pds_route_api_stats_to_proto(proto_stats, &api_info->stats);
}

#endif    //__AGENT_SVC_ROUTE_SVC_HPP__
