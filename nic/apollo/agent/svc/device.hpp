// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_SWITCH_HPP__
#define __AGENT_SVC_SWITCH_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/device.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::DeviceSvc;
using tpc::DeviceRequest;
using tpc::DeviceSpec;
using tpc::DeviceResponse;

class DeviceSvcImpl final : public DeviceSvc::Service {
public:
    Status DeviceCreate(ServerContext *context, const tpc::DeviceRequest *req,
                        tpc::DeviceResponse *rsp) override;
};

#endif    // __AGENT_SVC_SWITCH_HPP__
