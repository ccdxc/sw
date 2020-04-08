//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/session.hpp"
#include "nic/apollo/agent/svc/session_svc.hpp"

Status
SessionSvcImpl::SessionGet(ServerContext *context, const Empty *req,
                           grpc::ServerWriter<pds::SessionGetResponse> *writer) {
    debug::pds_session_get(pds_session_to_proto, writer);
    return Status::OK;
}

Status
SessionSvcImpl::FlowGet(ServerContext *context, const Empty *req,
                        grpc::ServerWriter<pds::FlowGetResponse> *writer) {
    flow_get_t flow;

    flow.writer = writer;
    flow.msg.clear_flow();
    flow.count = 0;

    debug::pds_flow_get(pds_flow_to_proto, &flow);

    return Status::OK;
}

Status
SessionSvcImpl::SessionClear(ServerContext *context,
                             const pds::SessionClearRequest *req,
                             pds::SessionClearResponse *rsp) {
    sdk_ret_t ret;
    uint32_t idx = 0;

    ret = debug::pds_session_clear(idx);
    rsp->set_apistatus(sdk_ret_to_api_status(ret));

    return Status::OK;
}

Status
SessionSvcImpl::FlowClear(ServerContext *context,
                          const pds::FlowClearRequest *req,
                          pds::FlowClearResponse *rsp) {
    sdk_ret_t ret;
    pds_flow_key_t flow_key;

    ret = pds_flow_proto_to_flow_key(&flow_key, req->filter());
    ret = debug::pds_flow_clear(flow_key);
    rsp->set_apistatus(sdk_ret_to_api_status(ret));

    return Status::OK;
}
