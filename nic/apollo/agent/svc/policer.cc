//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/policer_svc.hpp"

Status
PolicerSvcImpl::PolicerCreate(ServerContext *context,
                              const pds::PolicerRequest *proto_req,
                              pds::PolicerResponse *proto_rsp) {
    pds_svc_policer_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
PolicerSvcImpl::PolicerUpdate(ServerContext *context,
                              const pds::PolicerRequest *proto_req,
                              pds::PolicerResponse *proto_rsp) {
    pds_svc_policer_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
PolicerSvcImpl::PolicerDelete(ServerContext *context,
                              const pds::PolicerDeleteRequest *proto_req,
                              pds::PolicerDeleteResponse *proto_rsp) {
    pds_svc_policer_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
PolicerSvcImpl::PolicerGet(ServerContext *context,
                           const pds::PolicerGetRequest *proto_req,
                           pds::PolicerGetResponse *proto_rsp) {
    pds_svc_policer_get(proto_req, proto_rsp);
    return Status::OK;
}
