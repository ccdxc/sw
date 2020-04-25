//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/tunnel_svc.hpp"

// create tunnel object
Status
TunnelSvcImpl::TunnelCreate(ServerContext *context,
                            const pds::TunnelRequest *proto_req,
                            pds::TunnelResponse *proto_rsp) {
    pds_svc_tunnel_create(proto_req, proto_rsp);
    return Status::OK;
}

// update tunnel object
Status
TunnelSvcImpl::TunnelUpdate(ServerContext *context,
                            const pds::TunnelRequest *proto_req,
                            pds::TunnelResponse *proto_rsp) {
    pds_svc_tunnel_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
TunnelSvcImpl::TunnelDelete(ServerContext *context,
                            const pds::TunnelDeleteRequest *proto_req,
                            pds::TunnelDeleteResponse *proto_rsp) {
    pds_svc_tunnel_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
TunnelSvcImpl::TunnelGet(ServerContext *context,
                         const pds::TunnelGetRequest *proto_req,
                         pds::TunnelGetResponse *proto_rsp) {
    pds_svc_tunnel_get(proto_req, proto_rsp);
    return Status::OK;
}
