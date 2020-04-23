//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/vnic.hpp"
#include "nic/apollo/agent/svc/vnic_svc.hpp"

Status
VnicSvcImpl::VnicCreate(ServerContext *context,
                        const pds::VnicRequest *proto_req,
                        pds::VnicResponse *proto_rsp) {
    pds_svc_vnic_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
VnicSvcImpl::VnicUpdate(ServerContext *context,
                        const pds::VnicRequest *proto_req,
                        pds::VnicResponse *proto_rsp) {
    pds_svc_vnic_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
VnicSvcImpl::VnicDelete(ServerContext *context,
                        const pds::VnicDeleteRequest *proto_req,
                        pds::VnicDeleteResponse *proto_rsp) {
    pds_svc_vnic_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
VnicSvcImpl::VnicGet(ServerContext *context,
                     const pds::VnicGetRequest *proto_req,
                     pds::VnicGetResponse *proto_rsp) {
    pds_svc_vnic_get(proto_req, proto_rsp);
    return Status::OK;
}
