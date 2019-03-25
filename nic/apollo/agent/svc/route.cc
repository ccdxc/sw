//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/route.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/route.hpp"

static inline void
pds_agent_route_table_api_spec_fill (const pds::RouteTableSpec &proto_spec,
                                     pds_route_table_spec_t *api_spec)
{
    api_spec->key.id = proto_spec.id();
    switch (proto_spec.af()) {
    case types::IP_AF_INET:
        api_spec->af = IP_AF_IPV4;
        break;

    case types::IP_AF_INET6:
        api_spec->af = IP_AF_IPV6;
        break;

    default:
        break;
    }
    api_spec->num_routes = proto_spec.routes_size();
    api_spec->routes = (pds_route_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                                 sizeof(pds_route_t) *
                                                 api_spec->num_routes);
    for (int i = 0; i < proto_spec.routes_size(); i++) {
        const pds::Route &proto_route = proto_spec.routes(i);
        ippfx_proto_spec_to_api_spec_fill(proto_route.prefix(), &api_spec->routes[i].prefix);
        ipaddr_proto_spec_to_api_spec_fill(proto_route.nexthop(), &api_spec->routes[i].nh_ip);
        api_spec->routes[i].vcn_id = proto_route.vpcid();
        // TODO: hardcoded for now
        api_spec->routes[i].nh_type = PDS_NH_TYPE_REMOTE_TEP;
    }
}

Status
RouteSvcImpl::RouteTableCreate(ServerContext *context,
                               const pds::RouteTableRequest *proto_req,
                               pds::RouteTableResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_route_table_key_t key;
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
        memset(&key, 0, sizeof(pds_route_table_key_t));
        key.id = request.id();
        pds_agent_route_table_api_spec_fill(request, api_spec);
        ret = core::route_table_create(&key, api_spec);
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
    pds_route_table_key_t key;

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        memset(&key, 0, sizeof(pds_route_table_key_t));
        key.id = proto_req->id(i);
        ret = core::route_table_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}
