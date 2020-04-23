//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/subnet.hpp"
#include "nic/apollo/agent/svc/subnet_svc.hpp"

Status
SubnetSvcImpl::SubnetCreate(ServerContext *context,
                            const pds::SubnetRequest *proto_req,
                            pds::SubnetResponse *proto_rsp) {
    pds_svc_subnet_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
SubnetSvcImpl::SubnetUpdate(ServerContext *context,
                            const pds::SubnetRequest *proto_req,
                            pds::SubnetResponse *proto_rsp) {
    pds_svc_subnet_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
SubnetSvcImpl::SubnetDelete(ServerContext *context,
                            const pds::SubnetDeleteRequest *proto_req,
                            pds::SubnetDeleteResponse *proto_rsp) {
    pds_svc_subnet_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
SubnetSvcImpl::SubnetGet(ServerContext *context,
                         const pds::SubnetGetRequest *proto_req,
                         pds::SubnetGetResponse *proto_rsp) {
    pds_svc_subnet_get(proto_req, proto_rsp);
    return Status::OK;
}
