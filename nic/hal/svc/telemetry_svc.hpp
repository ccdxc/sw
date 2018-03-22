#ifndef __TELEMETRY_SVC_HPP__
#define __TELEMETRY_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/telemetry.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using telemetry::Telemetry;
using telemetry::CollectorConfigMsg;
using telemetry::CollectorResponseMsg;
using telemetry::ExportControlId;
using telemetry::ExportControlConfigMsg;
using telemetry::ExportControlResponseMsg;
using telemetry::ExportControlResponseMsg;
using telemetry::MirrorSessionConfigMsg;
using telemetry::MirrorSessionDeleteMsg;
using telemetry::MirrorSessionResponseMsg;
using telemetry::MirrorSessionId;
using telemetry::MirrorSession;
using telemetry::FlowMonitorRule;
using telemetry::FlowMonitorRuleConfigMsg;
using telemetry::FlowMonitorRuleResponseMsg;
using types::Empty;

class TelemetryServiceImpl final : public Telemetry::Service {
    public:
    // Collector related methods
    Status CollectorCreate(ServerContext* context,
                            const CollectorConfigMsg* request,
                            CollectorResponseMsg* response) override;

    Status CollectorUpdate(ServerContext* context,
                            const CollectorConfigMsg* request,
                            CollectorResponseMsg* response) override;

    Status CollectorList(ServerContext* context,
                            const ExportControlId* request,
                            CollectorResponseMsg* response) override;

    Status CollectorDelete(ServerContext* context,
                            const CollectorConfigMsg* request,
                            CollectorResponseMsg* response) override;

     // ExportControl related methods
    Status ExportControlCreate(ServerContext* context,
                                const ExportControlConfigMsg* request,
                                ExportControlResponseMsg* response) override;

    Status ExportControlUpdate(ServerContext* context,
                                const ExportControlConfigMsg* request,
                                ExportControlResponseMsg* response) override;

    Status ExportControlGet(ServerContext* context,
                            const ExportControlId* request,
                            ExportControlResponseMsg* response) override;

    Status ExportControlDelete(ServerContext* context,
                                const ExportControlId* request,
                                ExportControlResponseMsg* response) override;

     // FlowMonitorRules CRUD Operations
    Status FlowMonitorRuleCreate(ServerContext* context,
                                const FlowMonitorRuleConfigMsg* request,
                                FlowMonitorRuleResponseMsg* response) override;

    Status FlowMonitorRuleUpdate(ServerContext* context,
                                const FlowMonitorRuleConfigMsg* request,
                                FlowMonitorRuleResponseMsg* response) override;

    Status FlowMonitorRuleList(ServerContext* context,
                                const ExportControlId* request,
                                FlowMonitorRuleResponseMsg* response) override;

    Status FlowMonitorRuleGet(ServerContext* context,
                                const FlowMonitorRuleConfigMsg* request,
                                FlowMonitorRuleResponseMsg* response) override;
    Status FlowMonitorRuleDelete(ServerContext* context,
                                const FlowMonitorRuleConfigMsg* request,
                                FlowMonitorRuleResponseMsg* response) override;

     // MirrorSession CRUD Operations
    Status MirrorSessionCreate(ServerContext* context,
                                const MirrorSessionConfigMsg* request,
                                MirrorSessionResponseMsg* response) override;

    Status MirrorSessionUpdate(ServerContext* context,
                                const MirrorSessionConfigMsg* request,
                                MirrorSessionResponseMsg* response) override;

    Status MirrorSessionGet(ServerContext* context,
                            const MirrorSessionId* request,
                            MirrorSessionResponseMsg* response) override;

    Status MirrorSessionList(ServerContext* context,
                            const Empty* request,
                            MirrorSessionResponseMsg* response) override;

    Status MirrorSessionDelete(ServerContext* context,
                                const MirrorSessionDeleteMsg* request,
                                MirrorSessionResponseMsg* response) override;
};


#endif // __TELEMETRY_SVC_HPP__
