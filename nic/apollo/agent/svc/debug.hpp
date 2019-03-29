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

class DebugSvcImpl final : public DebugSvc::Service {
public:
    Status ClockFrequencyUpdate(ServerContext *context,
                                const pds::ClockFrequencyRequest *req,
                                pds::ClockFrequencyResponse *rsp) override;
    Status SystemTemperatureGet(ServerContext *context, const Empty *req,
                                pds::SystemTemperatureGetResponse *rsp) override;
    Status SystemPowerGet(ServerContext *context, const Empty *req,
                          pds::SystemPowerGetResponse *rsp) override;
};

#endif    // __AGENT_SVC_DEBUG_HPP__
