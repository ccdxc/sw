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


static inline sdk_ret_t
pds_route_proto_to_api_spec_common (pds_route_t *route,
                                    const pds::RouteInfo &proto_route)
{
    pds_obj_key_proto_to_api_spec(&route->key, proto_route.id());
    ippfx_proto_spec_to_api_spec(&route->prefix,
                                 proto_route.prefix());
    route->prio = proto_route.priority();
    switch (proto_route.nh_case()) {
    case pds::RouteInfo::kNextHop:
    case pds::RouteInfo::kTunnelId:
        route->nh_type = PDS_NH_TYPE_OVERLAY;
        pds_obj_key_proto_to_api_spec(&route->tep,
                                      proto_route.tunnelid());
        break;
    case pds::RouteInfo::kNexthopGroupId:
        // NOTE: UNDERLAY_ECMP is not done in the datapath
        route->nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
        pds_obj_key_proto_to_api_spec(&route->nh_group,
                                      proto_route.nexthopgroupid());
        break;
    case pds::RouteInfo::kVPCId:
        pds_obj_key_proto_to_api_spec(&route->vpc,
                                      proto_route.vpcid());
        route->nh_type = PDS_NH_TYPE_PEER_VPC;
        break;
    case pds::RouteInfo::kVnicId:
        pds_obj_key_proto_to_api_spec(&route->vnic,
                                      proto_route.vnicid());
        route->nh_type = PDS_NH_TYPE_VNIC;
        break;
    case pds::RouteInfo::kNexthopId:
        route->nh_type = PDS_NH_TYPE_IP;
        pds_obj_key_proto_to_api_spec(&route->nh,
                                      proto_route.nexthopid());
        break;
    default:
        route->nh_type = PDS_NH_TYPE_BLACKHOLE;
        break;
    }
    if (proto_route.has_nataction()) {
        auto nat_action = proto_route.nataction();
        switch (nat_action.srcnataction()) {
        case types::NAT_ACTION_STATIC:
            route->nat.src_nat_type =
                PDS_NAT_TYPE_STATIC;
            break;
        case types::NAT_ACTION_NAPT_PUBLIC:
            route->nat.src_nat_type =
                PDS_NAT_TYPE_NAPT_PUBLIC;
            break;
        case types::NAT_ACTION_NAPT_SVC:
            route->nat.src_nat_type =
                PDS_NAT_TYPE_NAPT_SVC;
            break;
        case types::NAT_ACTION_NONE:
        default:
            route->nat.src_nat_type =
                PDS_NAT_TYPE_NONE;
            break;
        }
        if (nat_action.has_dstnatip()) {
            ipaddr_proto_spec_to_api_spec(
                &route->nat.dst_nat_ip,
                nat_action.dstnatip());
        } else {
            memset(&route->nat.dst_nat_ip, 0,
                   sizeof(route->nat.dst_nat_ip));
        }
    }
    route->meter = proto_route.meteren();

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
        pds_route_proto_to_api_spec_common(&api_spec->route_info->routes[i],
                                           proto_spec.routes(i));
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
        ippfx_api_spec_to_proto_spec(route->mutable_prefix(),
                                     &api_spec->route_info->routes[i].prefix);
        if (api_spec->route_info->priority_en) {
            route->set_priority(api_spec->route_info->routes[i].prio);
        }
        switch (api_spec->route_info->routes[i].nh_type) {
        case PDS_NH_TYPE_OVERLAY:
            route->set_tunnelid(api_spec->route_info->routes[i].tep.id,
                                PDS_MAX_KEY_LEN);
            break;
        case PDS_NH_TYPE_OVERLAY_ECMP:
            route->set_nexthopgroupid(api_spec->route_info->routes[i].nh_group.id,
                                      PDS_MAX_KEY_LEN);
            break;
        case PDS_NH_TYPE_PEER_VPC:
            route->set_vpcid(api_spec->route_info->routes[i].vpc.id,
                             PDS_MAX_KEY_LEN);
            break;
        case PDS_NH_TYPE_VNIC:
            route->set_vnicid(api_spec->route_info->routes[i].vnic.id,
                              PDS_MAX_KEY_LEN);
            break;
        case PDS_NH_TYPE_IP:
            route->set_nexthopid(api_spec->route_info->routes[i].nh.id,
                                 PDS_MAX_KEY_LEN);
            break;
        case PDS_NH_TYPE_BLACKHOLE:
        default:
            // blackhole nexthop
            break;
        }
        switch (api_spec->route_info->routes[i].nat.src_nat_type) {
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
        if (!ip_addr_is_zero(&api_spec->route_info->routes[i].nat.dst_nat_ip)) {
            ipaddr_api_spec_to_proto_spec(
                route->mutable_nataction()->mutable_dstnatip(),
                &api_spec->route_info->routes[i].nat.dst_nat_ip);
        }
        route->set_meteren(api_spec->route_info->routes[i].meter);
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

static inline sdk_ret_t
pds_route_proto_to_api_spec (pds_route_spec_t *api_spec,
                             const pds::RouteSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->route_table, proto_spec.routetableid());
    return pds_route_proto_to_api_spec_common(&api_spec->route, proto_spec.route());
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

#endif    //__AGENT_SVC_ROUTE_SVC_HPP__
