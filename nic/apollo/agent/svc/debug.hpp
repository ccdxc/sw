// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_DEBUG_HPP__
#define __AGENT_SVC_DEBUG_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/debug.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::DebugSvc;
using pds::ClockFrequencyRequest;
using pds::ClockFrequencyResponse;
using types::Empty;
using pds::SystemTemperatureGetResponse;
using pds::SystemPowerGetResponse;
using pds::TraceRequest;
using pds::TraceResponse;
using pds::TableStatsGetResponse;
using pds::LlcStatsGetResponse;
using pds::LlcSetupRequest;
using pds::LlcSetupResponse;
using pds::MeterStatsGetRequest;
using pds::MeterStatsGetResponse;
using pds::SessionStatsGetRequest;
using pds::SessionStatsGetResponse;
using pds::FteStatsGetResponse;
using pds::FteStatsClearRequest;
using pds::FteStatsGetResponse;
using pds::QueueCreditsGetResponse;
using pds::AacsRequest;
using pds::SlabGetResponse;

class DebugSvcImpl final : public DebugSvc::Service {
public:
    Status ClockFrequencyUpdate(ServerContext *context,
                                const pds::ClockFrequencyRequest *req,
                                pds::ClockFrequencyResponse *rsp) override;
    Status SystemTemperatureGet(ServerContext *context, const Empty *req,
                                pds::SystemTemperatureGetResponse *rsp) override;
    Status SystemPowerGet(ServerContext *context, const Empty *req,
                          pds::SystemPowerGetResponse *rsp) override;
    Status TraceUpdate(ServerContext *context, const pds::TraceRequest *req,
                       pds::TraceResponse *rsp) override;
    Status TableStatsGet(ServerContext *context, const Empty *req,
                         pds::TableStatsGetResponse *rsp) override;
    Status LlcSetup(ServerContext *context, const pds::LlcSetupRequest *req,
                    pds::LlcSetupResponse *rsp) override;
    Status LlcStatsGet(ServerContext *context, const Empty *req,
                       pds::LlcStatsGetResponse *rsp) override;
    Status PbStatsGet(ServerContext *context, const Empty *proto_req,
                      pds::PbStatsGetResponse *proto_rsp) override;
    Status TraceFlush(ServerContext *context, const Empty *req,
                      Empty *rsp) override;
    Status HeapGet(ServerContext *context, const Empty *req,
                   pds::HeapGetResponse *rsp) override;
    Status MeterStatsGet(ServerContext *context, const pds::MeterStatsGetRequest *req,
                         pds::MeterStatsGetResponse *rsp) override;
    Status SessionStatsGet(ServerContext *context, const pds::SessionStatsGetRequest *req,
                           pds::SessionStatsGetResponse *rsp) override;
    Status FteStatsGet(ServerContext *context, const Empty *req,
                       pds::FteStatsGetResponse *rsp) override;
    Status FteStatsClear(ServerContext *context, const pds::FteStatsClearRequest *req,
                         pds::FteStatsClearResponse *rsp) override;
    Status MemoryTrim(ServerContext *context, const Empty *req,
                      Empty *rsp) override;
    Status QueueCreditsGet(ServerContext *context, const Empty *req,
                           QueueCreditsGetResponse *rsp) override;
    Status StartAacsServer(ServerContext *context, const pds::AacsRequest *req,
                           Empty *rsp) override;
    Status StopAacsServer(ServerContext *context, const Empty *req,
                           Empty *rsp) override;
    Status SlabGet(ServerContext *context, const Empty *req,
                   pds::SlabGetResponse *rsp) override;
    Status InternalPortGet(ServerContext *context, const pds::InternalPortRequestMsg *req,
                           pds::InternalPortResponseMsg *rsp) override;
};

#endif    // __AGENT_SVC_DEBUG_HPP__
