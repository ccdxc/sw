//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/include/api/pds_route.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/route.hpp"

static inline void
pds_agent_route_table_api_spec_fill (const tpc::RouteTableSpec *proto_spec,
                                     pds_route_table_spec_t *api_spec)
{
    pds_route_t route = {0};

    api_spec->key.id = proto_spec->id();
    api_spec->af = proto_spec->af();
    api_spec->num_routes = proto_spec->routes_size();
    api_spec->routes = (pds_route_t *)SDK_CALLOC(PDS_MEM_ALLOC_ROUTE_TABLE,
                                                 sizeof(pds_route_t) *
                                                     proto_spec->routes_size());
    for (int i = 0; i < proto_spec->routes_size(); i++) {
        const tpc::Route &proto_route = proto_spec->routes(i);
        pds_agent_util_ip_pfx_fill(proto_route.prefix(), &route.prefix);
        pds_agent_util_ipaddr_fill(proto_route.nexthop(), &route.nh_ip);
        route.vcn_id = proto_route.pcnid();
        api_spec->routes[i] = route;
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
                               const tpc::RouteTableSpec *proto_spec,
                               tpc::RouteTableStatus *proto_status) {
    pds_route_table_spec_t api_spec;
    sdk_ret_t ret;

    if (proto_spec) {
        pds_agent_route_table_api_spec_fill(proto_spec, &api_spec);
        ret = pds_route_table_create(&api_spec);
        pds_agent_route_table_api_spec_free(&api_spec);
        if (ret == sdk::SDK_RET_OK)
            return Status::OK;
    }
    return Status::CANCELLED;
}
