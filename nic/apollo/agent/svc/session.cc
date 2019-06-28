//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/session.hpp"

Status
SessionSvcImpl::SessionGet(ServerContext *context, const Empty *req,
                           grpc::ServerWriter<pds::SessionGetResponse> *writer) {
    return Status::OK;
}

Status
SessionSvcImpl::FlowGet(ServerContext *context, const Empty *req,
                        grpc::ServerWriter<pds::FlowGetResponse> *writer) {
    return Status::OK;
}

Status
SessionSvcImpl::SessionClear(ServerContext *context, const pds::SessionClearRequest *req,
                             pds::SessionClearResponse *rsp) {
    return Status::OK;
}

Status
SessionSvcImpl::FlowClear(ServerContext *context, const pds::FlowClearRequest *req,
                          pds::FlowClearResponse *rsp) {
    return Status::OK;
}
