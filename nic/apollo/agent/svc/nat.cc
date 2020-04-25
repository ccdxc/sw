//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/nat_svc.hpp"

Status
NatSvcImpl::NatPortBlockCreate(ServerContext *context,
                               const pds::NatPortBlockRequest *proto_req,
                               pds::NatPortBlockResponse *proto_rsp) {
    pds_svc_nat_port_block_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
NatSvcImpl::NatPortBlockDelete(ServerContext *context,
                               const pds::NatPortBlockDeleteRequest *proto_req,
                               pds::NatPortBlockDeleteResponse *proto_rsp) {
    pds_svc_nat_port_block_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
NatSvcImpl::NatPortBlockGet(ServerContext *context,
                            const pds::NatPortBlockGetRequest *proto_req,
                            pds::NatPortBlockGetResponse *proto_rsp) {
    pds_svc_nat_port_block_get(proto_req, proto_rsp);
    return Status::OK;
}
