//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/vpc.hpp"
#include "nic/apollo/agent/svc/vpc_svc.hpp"

Status
VPCSvcImpl::VPCCreate(ServerContext *context,
                      const pds::VPCRequest *proto_req,
                      pds::VPCResponse *proto_rsp) {
    pds_svc_vpc_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
VPCSvcImpl::VPCUpdate(ServerContext *context,
                      const pds::VPCRequest *proto_req,
                      pds::VPCResponse *proto_rsp) {
    pds_svc_vpc_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
VPCSvcImpl::VPCDelete(ServerContext *context,
                      const pds::VPCDeleteRequest *proto_req,
                      pds::VPCDeleteResponse *proto_rsp) {
    pds_svc_vpc_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
VPCSvcImpl::VPCGet(ServerContext *context,
                   const pds::VPCGetRequest *proto_req,
                   pds::VPCGetResponse *proto_rsp) {
    pds_svc_vpc_get(proto_req, proto_rsp);
    return Status::OK;
}

Status
VPCSvcImpl::VPCPeerCreate(ServerContext *context,
                          const pds::VPCPeerRequest *proto_req,
                          pds::VPCPeerResponse *proto_rsp) {
    pds_svc_vpc_peer_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
VPCSvcImpl::VPCPeerDelete(ServerContext *context,
                          const pds::VPCPeerDeleteRequest *proto_req,
                          pds::VPCPeerDeleteResponse *proto_rsp) {
    pds_svc_vpc_peer_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
VPCSvcImpl::VPCPeerGet(ServerContext *context,
                       const pds::VPCPeerGetRequest *proto_req,
                       pds::VPCPeerGetResponse *proto_rsp) {
    pds_svc_vpc_peer_get(proto_req, proto_rsp);
    return Status::OK;
}
