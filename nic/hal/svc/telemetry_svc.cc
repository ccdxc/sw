#include "nic/include/hal_cfg.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/svc/telemetry_svc.hpp"
#include "nic/hal/plugins/cfg/telemetry/telemetry.hpp"
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using telemetry::Telemetry;
Status
TelemetryServiceImpl::CollectorCreate(ServerContext *context,
                                       const telemetry::CollectorRequestMsg *req,
                                       telemetry::CollectorResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd CollectorCreate");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::collector_create(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::CollectorUpdate(ServerContext *context,
                                       const telemetry::CollectorRequestMsg *req,
                                       telemetry::CollectorResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd CollectorUpdate");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::collector_update(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::CollectorDelete(ServerContext *context,
                                       const telemetry::CollectorDeleteRequestMsg *req,
                                       telemetry::CollectorDeleteResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd CollectorDelete");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::collector_delete(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::CollectorGet(ServerContext *context,
                                       const telemetry::CollectorGetRequestMsg *req,
                                       telemetry::CollectorGetResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd CollectorGet");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::collector_get(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::FlowMonitorRuleCreate(ServerContext *context,
                                       const telemetry::FlowMonitorRuleRequestMsg *req,
                                       telemetry::FlowMonitorRuleResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd FlowMonitorRuleCreate");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::flow_monitor_rule_create(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::FlowMonitorRuleUpdate(ServerContext *context,
                                       const telemetry::FlowMonitorRuleRequestMsg *req,
                                       telemetry::FlowMonitorRuleResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd FlowMonitorRuleUpdate");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::flow_monitor_rule_update(request, response, 
                                      (i == (nreqs - 1) ? true : false));
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::FlowMonitorRuleDelete(ServerContext *context,
                                       const telemetry::FlowMonitorRuleDeleteRequestMsg *req,
                                       telemetry::FlowMonitorRuleDeleteResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd FlowMonitorRuleDelete");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::flow_monitor_rule_delete(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::FlowMonitorRuleGet(ServerContext *context,
                                       const telemetry::FlowMonitorRuleGetRequestMsg *req,
                                       telemetry::FlowMonitorRuleGetResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd FlowMonitorRuleGet");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::flow_monitor_rule_get(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::DropMonitorRuleCreate(ServerContext *context,
                                       const telemetry::DropMonitorRuleRequestMsg *req,
                                       telemetry::DropMonitorRuleResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd DropMonitorRuleCreate");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::drop_monitor_rule_create(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::DropMonitorRuleUpdate(ServerContext *context,
                                       const telemetry::DropMonitorRuleRequestMsg *req,
                                       telemetry::DropMonitorRuleResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd DropMonitorRuleUpdate");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::drop_monitor_rule_update(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::DropMonitorRuleDelete(ServerContext *context,
                                       const telemetry::DropMonitorRuleDeleteRequestMsg *req,
                                       telemetry::DropMonitorRuleDeleteResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd DropMonitorRuleDelete");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::drop_monitor_rule_delete(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::DropMonitorRuleGet(ServerContext *context,
                                       const telemetry::DropMonitorRuleGetRequestMsg *req,
                                       telemetry::DropMonitorRuleGetResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd DropMonitorRuleGet");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::drop_monitor_rule_get(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::MirrorSessionCreate(ServerContext *context,
                                       const telemetry::MirrorSessionRequestMsg *req,
                                       telemetry::MirrorSessionResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd MirrorSessionCreate");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::mirror_session_create(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::MirrorSessionUpdate(ServerContext *context,
                                       const telemetry::MirrorSessionRequestMsg *req,
                                       telemetry::MirrorSessionResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd MirrorSessionUpdate");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::mirror_session_update(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::MirrorSessionDelete(ServerContext *context,
                                       const telemetry::MirrorSessionDeleteRequestMsg *req,
                                       telemetry::MirrorSessionDeleteResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd MirrorSessionDelete");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        auto response = rsp->add_response();
        hal::mirror_session_delete(request, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
Status
TelemetryServiceImpl::MirrorSessionGet(ServerContext *context,
                                       const telemetry::MirrorSessionGetRequestMsg *req,
                                       telemetry::MirrorSessionGetResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    //HAL_TRACE_DEBUG("Rcvd MirrorSessionGet");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::mirror_session_get(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
