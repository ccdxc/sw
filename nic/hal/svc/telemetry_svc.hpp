// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_SVC_TELEMETRY_HPP__
#define __HAL_SVC_TELEMETRY_HPP__

#include "nic/include/base.hpp"
#include "gen/hal/include/hal_api_stats.hpp"
#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/telemetry.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;
using telemetry::Telemetry;

class TelemetryServiceImpl final : public Telemetry::Service {
public:
    Status CollectorCreate(ServerContext *context,
                        const telemetry::CollectorRequestMsg *req,
                        telemetry::CollectorResponseMsg *rsp) override;

    Status CollectorUpdate(ServerContext *context,
                        const telemetry::CollectorRequestMsg *req,
                        telemetry::CollectorResponseMsg *rsp) override;

    Status CollectorDelete(ServerContext *context,
                        const telemetry::CollectorDeleteRequestMsg *req,
                        telemetry::CollectorDeleteResponseMsg *rsp) override;

    Status CollectorGet(ServerContext *context,
                        const telemetry::CollectorGetRequestMsg *req,
                        telemetry::CollectorGetResponseMsg *rsp) override;

    Status FlowMonitorRuleCreate(ServerContext *context,
                        const telemetry::FlowMonitorRuleRequestMsg *req,
                        telemetry::FlowMonitorRuleResponseMsg *rsp) override;

    Status FlowMonitorRuleUpdate(ServerContext *context,
                        const telemetry::FlowMonitorRuleRequestMsg *req,
                        telemetry::FlowMonitorRuleResponseMsg *rsp) override;

    Status FlowMonitorRuleDelete(ServerContext *context,
                        const telemetry::FlowMonitorRuleDeleteRequestMsg *req,
                        telemetry::FlowMonitorRuleDeleteResponseMsg *rsp) override;

    Status FlowMonitorRuleGet(ServerContext *context,
                        const telemetry::FlowMonitorRuleGetRequestMsg *req,
                        telemetry::FlowMonitorRuleGetResponseMsg *rsp) override;

    Status DropMonitorRuleCreate(ServerContext *context,
                        const telemetry::DropMonitorRuleRequestMsg *req,
                        telemetry::DropMonitorRuleResponseMsg *rsp) override;

    Status DropMonitorRuleUpdate(ServerContext *context,
                        const telemetry::DropMonitorRuleRequestMsg *req,
                        telemetry::DropMonitorRuleResponseMsg *rsp) override;

    Status DropMonitorRuleDelete(ServerContext *context,
                        const telemetry::DropMonitorRuleDeleteRequestMsg *req,
                        telemetry::DropMonitorRuleDeleteResponseMsg *rsp) override;

    Status DropMonitorRuleGet(ServerContext *context,
                        const telemetry::DropMonitorRuleGetRequestMsg *req,
                        telemetry::DropMonitorRuleGetResponseMsg *rsp) override;

    Status MirrorSessionCreate(ServerContext *context,
                        const telemetry::MirrorSessionRequestMsg *req,
                        telemetry::MirrorSessionResponseMsg *rsp) override;

    Status MirrorSessionUpdate(ServerContext *context,
                        const telemetry::MirrorSessionRequestMsg *req,
                        telemetry::MirrorSessionResponseMsg *rsp) override;

    Status MirrorSessionDelete(ServerContext *context,
                        const telemetry::MirrorSessionDeleteRequestMsg *req,
                        telemetry::MirrorSessionDeleteResponseMsg *rsp) override;

    Status MirrorSessionGet(ServerContext *context,
                        const telemetry::MirrorSessionGetRequestMsg *req,
                        telemetry::MirrorSessionGetResponseMsg *rsp) override;

};


#endif   // __HAL_SVC_TELEMETRY_HPP__

