//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/include/api/pds_route.hpp"
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
    api_spec->routes = (pds_route_t *)SDK_CALLOC(PDS_MEM_ALLOC_ROUTE_TABLE,
                                                 sizeof(pds_route_t) *
                                                 api_spec->num_routes);
    for (int i = 0; i < proto_spec.routes_size(); i++) {
        const pds::Route &proto_route = proto_spec.routes(i);
        pds_agent_util_ip_pfx_fill(proto_route.prefix(), &api_spec->routes[i].prefix);
        pds_agent_util_ipaddr_fill(proto_route.nexthop(), &api_spec->routes[i].nh_ip);
        api_spec->routes[i].vcn_id = proto_route.pcnid();
        // TODO: hardcoded for now
        api_spec->routes[i].nh_type = PDS_NH_TYPE_REMOTE_TEP;
    }
}

static inline void
pds_agent_route_table_api_spec_free (pds_route_table_spec_t *api_spec)
{
    if (api_spec->routes) {
        SDK_FREE(PDS_MEM_ALLOC_ROUTE_TABLE, api_spec->routes);
        api_spec->routes = NULL;
    }
}

Status
RouteSvcImpl::RouteTableCreate(ServerContext *context,
                               const pds::RouteTableRequest *proto_req,
                               pds::RouteTableResponse *proto_rsp) {
    sdk_ret_t ret;

    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            pds_route_table_spec_t api_spec;

            pds_agent_route_table_api_spec_fill(proto_req->request(i), &api_spec);
            ret = pds_route_table_create(&api_spec);
            pds_agent_route_table_api_spec_free(&api_spec);
            if (ret == sdk::SDK_RET_OK)
                return Status::OK;
        }
    }
    return Status::CANCELLED;
}
