//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/route.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/route.hpp"

static inline sdk_ret_t
pds_agent_route_table_api_spec_fill (pds_route_table_spec_t *api_spec,
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
        return SDK_RET_INVALID_ARG;
    }
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
        ippfx_proto_spec_to_api_spec(&api_spec->routes[i].prefix, proto_route.prefix());
        switch (proto_route.Nh_case()) {
        case pds::Route::kNextHop:
            ipaddr_proto_spec_to_api_spec(&api_spec->routes[i].nh_ip, proto_route.nexthop());
            api_spec->routes[i].nh_type = PDS_NH_TYPE_TEP;
            break;
        case pds::Route::kVPCId:
            api_spec->routes[i].vpc.id = proto_route.vpcid();
            api_spec->routes[i].nh_type = PDS_NH_TYPE_PEER_VPC;
            break;
        default:
            api_spec->routes[i].nh_type = PDS_NH_TYPE_BLACKHOLE;
            break;
        }
    }

    return SDK_RET_OK;
}

Status
RouteSvcImpl::RouteTableCreate(ServerContext *context,
                               const pds::RouteTableRequest *proto_req,
                               pds::RouteTableResponse *proto_rsp) {
    sdk_ret_t ret = sdk::SDK_RET_OK;
    pds_route_table_key_t key = {0};
    pds_route_table_spec_t *api_spec;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_route_table_spec_t *)
                    core::agent_state::state()->route_table_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_agent_route_table_api_spec_fill(api_spec, request);
        if (unlikely(ret != SDK_RET_OK)) {
            return Status::CANCELLED;
        }
        ret = core::route_table_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
RouteSvcImpl::RouteTableUpdate(ServerContext *context,
                               const pds::RouteTableRequest *proto_req,
                               pds::RouteTableResponse *proto_rsp) {
    sdk_ret_t ret = sdk::SDK_RET_OK;
    pds_route_table_key_t key = {0};
    pds_route_table_spec_t *api_spec;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_route_table_spec_t *)
                    core::agent_state::state()->route_table_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_agent_route_table_api_spec_fill(api_spec, request);
        if (unlikely(ret != SDK_RET_OK)) {
            return Status::CANCELLED;
        }
        ret = core::route_table_update(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
RouteSvcImpl::RouteTableDelete(ServerContext *context,
                               const pds::RouteTableDeleteRequest *proto_req,
                               pds::RouteTableDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_route_table_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::route_table_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}
