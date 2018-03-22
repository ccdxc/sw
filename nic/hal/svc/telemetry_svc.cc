#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/telemetry_svc.hpp"
#include "nic/hal/src/telemetry/telemetry.hpp"


Status
TelemetryServiceImpl::CollectorCreate(ServerContext* context,
                            const CollectorConfigMsg* request,
                            CollectorResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd Collector Create Request");
    Collector *resp;
    uint32_t i, nreqs = request->request_size();

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; ++i) {
        resp = response->add_response();
        auto spec = request->request(i);
        hal::collector_create(&spec, resp);
    }
    HAL_TRACE_DEBUG("Rcvd Collector Create Request Succeeded");
    return Status::OK;
}

Status
TelemetryServiceImpl::CollectorUpdate(ServerContext* context,
                        const CollectorConfigMsg* request,
                        CollectorResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd CollectorUpdate Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::CollectorList(ServerContext* context,
                        const ExportControlId* request,
                        CollectorResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd Collector List Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::CollectorDelete(ServerContext* context,
                        const CollectorConfigMsg* request,
                        CollectorResponseMsg* response)
{
     HAL_TRACE_DEBUG("Rcvd Collector Delete Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::ExportControlCreate(ServerContext* context,
                            const ExportControlConfigMsg* request,
                            ExportControlResponseMsg* response)
{
    HAL_TRACE_DEBUG("RcvdExportControlCreate Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::ExportControlUpdate(ServerContext* context,
                            const ExportControlConfigMsg* request,
                            ExportControlResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd ExportControlUpdate Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::ExportControlGet(ServerContext* context,
                        const ExportControlId* request,
                        ExportControlResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd ExportControlGet Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::ExportControlDelete(ServerContext* context,
                            const ExportControlId* request,
                            ExportControlResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd ExportControlDelete Request");
    return Status::OK;
}
                            
Status
TelemetryServiceImpl::FlowMonitorRuleCreate(ServerContext* context,
                            const FlowMonitorRuleConfigMsg* request,
                            FlowMonitorRuleResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd FlowMonitorRuleCreate Request");
    FlowMonitorRule *resp;
    uint32_t i, nreqs = request->request_size();

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; ++i) {
        resp = response->add_response();
        auto spec = request->request(i);
        hal::flow_monitor_rule_create(&spec, resp);
    }
    return Status::OK;
}

Status
TelemetryServiceImpl::FlowMonitorRuleDelete(ServerContext* context,
                            const FlowMonitorRuleConfigMsg* request,
                            FlowMonitorRuleResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd FlowMonitorRuleDelete Request");
    FlowMonitorRule *resp;
    uint32_t i, nreqs = request->request_size();

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; ++i) {
        resp = response->add_response();
        auto spec = request->request(i);
        hal::flow_monitor_rule_delete(&spec, resp);
    }
    return Status::OK;
}

Status
TelemetryServiceImpl::FlowMonitorRuleUpdate(ServerContext* context,
                            const FlowMonitorRuleConfigMsg* request,
                            FlowMonitorRuleResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd FlowMonitorRuleUpdate Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::FlowMonitorRuleList(ServerContext* context,
                            const ExportControlId* request,
                            FlowMonitorRuleResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd FlowMonitorRuleList Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::FlowMonitorRuleGet(ServerContext* context,
                            const FlowMonitorRuleConfigMsg* request,
                            FlowMonitorRuleResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd FlowMonitorRuleGet Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::MirrorSessionCreate(ServerContext* context,
                            const MirrorSessionConfigMsg* request,
                            MirrorSessionResponseMsg* response)
{
    HAL_TRACE_DEBUG("RcvdMirrorSessionCreate Request");
    MirrorSession *resp;
    uint32_t i, nreqs = request->request_size();

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; ++i) {
        resp = response->add_response();
        auto spec = request->request(i);
        hal::mirror_session_create(&spec, resp);
    }
    return Status::OK;
}

Status
TelemetryServiceImpl::MirrorSessionUpdate(ServerContext* context,
                            const MirrorSessionConfigMsg* request,
                            MirrorSessionResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd MirrorSession Update Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::MirrorSessionGet(ServerContext* context,
                        const MirrorSessionId* request,
                        MirrorSessionResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd MirrorSession Get Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::MirrorSessionList(ServerContext* context,
                        const Empty* request,
                        MirrorSessionResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd MirrorSession List Request");
    return Status::OK;
}

Status
TelemetryServiceImpl::MirrorSessionDelete(ServerContext* context,
                            const MirrorSessionDeleteMsg* request,
                            MirrorSessionResponseMsg* response)
{
    HAL_TRACE_DEBUG("Rcvd MirrorSession Delete Request");
    MirrorSession *resp;
    uint32_t i, nreqs = request->request_size();

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; ++i) {
        resp = response->add_response();
        auto spec = request->request(i);
        hal::mirror_session_delete(&spec, resp);
    }
    return Status::OK;
}
