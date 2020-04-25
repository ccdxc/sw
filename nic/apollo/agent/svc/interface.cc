//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/interface_svc.hpp"

Status
IfSvcImpl::InterfaceCreate(ServerContext *context,
                           const pds::InterfaceRequest *proto_req,
                           pds::InterfaceResponse *proto_rsp) {
    pds_svc_interface_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
IfSvcImpl::InterfaceUpdate(ServerContext *context,
                           const pds::InterfaceRequest *proto_req,
                           pds::InterfaceResponse *proto_rsp) {
    pds_svc_interface_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
IfSvcImpl::InterfaceDelete(ServerContext *context,
                           const pds::InterfaceDeleteRequest *proto_req,
                           pds::InterfaceDeleteResponse *proto_rsp) {
    pds_svc_interface_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
IfSvcImpl::InterfaceGet(ServerContext *context,
                        const pds::InterfaceGetRequest *proto_req,
                        pds::InterfaceGetResponse *proto_rsp) {
    pds_svc_interface_get(proto_req, proto_rsp);
    return Status::OK;
}

Status
IfSvcImpl::LifGet(ServerContext *context,
                  const pds::LifGetRequest *proto_req,
                  pds::LifGetResponse *proto_rsp) {
    pds_svc_lif_get(proto_req, proto_rsp);
    return Status::OK;
}
