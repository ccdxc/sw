//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/route_svc.hpp"

Status
RouteSvcImpl::RouteTableCreate(ServerContext *context,
                               const pds::RouteTableRequest *proto_req,
                               pds::RouteTableResponse *proto_rsp) {
    pds_svc_route_table_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
RouteSvcImpl::RouteTableUpdate(ServerContext *context,
                               const pds::RouteTableRequest *proto_req,
                               pds::RouteTableResponse *proto_rsp) {
    pds_svc_route_table_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
RouteSvcImpl::RouteTableDelete(ServerContext *context,
                               const pds::RouteTableDeleteRequest *proto_req,
                               pds::RouteTableDeleteResponse *proto_rsp) {
    pds_svc_route_table_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
RouteSvcImpl::RouteTableGet(ServerContext *context,
                            const pds::RouteTableGetRequest *proto_req,
                            pds::RouteTableGetResponse *proto_rsp) {
    pds_svc_route_table_get(proto_req, proto_rsp);
    return Status::OK;
}

Status
RouteSvcImpl::RouteCreate(ServerContext *context,
                          const pds::RouteRequest *proto_req,
                          pds::RouteResponse *proto_rsp) {
    pds_svc_route_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
RouteSvcImpl::RouteUpdate(ServerContext *context,
                          const pds::RouteRequest *proto_req,
                          pds::RouteResponse *proto_rsp) {
    pds_svc_route_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
RouteSvcImpl::RouteDelete(ServerContext *context,
                          const pds::RouteDeleteRequest *proto_req,
                          pds::RouteDeleteResponse *proto_rsp) {
    pds_svc_route_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
RouteSvcImpl::RouteGet(ServerContext *context,
                       const pds::RouteGetRequest *proto_req,
                       pds::RouteGetResponse *proto_rsp) {
    pds_svc_route_get(proto_req, proto_rsp);
    return Status::OK;
}
