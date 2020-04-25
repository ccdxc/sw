// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_SWITCH_HPP__
#define __AGENT_SVC_SWITCH_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/device.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::DeviceSvc;
using pds::DeviceRequest;
using pds::DeviceSpec;
using pds::DeviceDeleteRequest;
using pds::DeviceResponse;

class DeviceSvcImpl final : public DeviceSvc::Service {
public:
    Status DeviceCreate(ServerContext *context, const pds::DeviceRequest *req,
                        pds::DeviceResponse *rsp) override;
    Status DeviceUpdate(ServerContext *context, const pds::DeviceRequest *req,
                        pds::DeviceResponse *rsp) override;
    Status DeviceDelete(ServerContext *context,
                        const pds::DeviceDeleteRequest *req,
                        pds::DeviceDeleteResponse *proto_rsp) override;
    Status DeviceGet(ServerContext *context,
                     const pds::DeviceGetRequest *proto_req,
                     pds::DeviceGetResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_SWITCH_HPP__
