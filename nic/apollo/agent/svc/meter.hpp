// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_METER_HPP__
#define __AGENT_SVC_METER_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/meter.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::MeterSvc;
using pds::MeterRequest;
using pds::MeterSpec;
using pds::MeterResponse;
using pds::MeterGetRequest;
using pds::MeterGetResponse;
using pds::MeterDeleteRequest;
using pds::MeterDeleteResponse;

class MeterSvcImpl final : public MeterSvc::Service {
public:
    Status MeterCreate(ServerContext *context, const pds::MeterRequest *req,
                       pds::MeterResponse *rsp) override;
    Status MeterUpdate(ServerContext *context, const pds::MeterRequest *req,
                       pds::MeterResponse *rsp) override;
    Status MeterDelete(ServerContext *context,
                       const pds::MeterDeleteRequest *proto_req,
                       pds::MeterDeleteResponse *proto_rsp) override;
    Status MeterGet(ServerContext *context,
                    const pds::MeterGetRequest *proto_req,
                    pds::MeterGetResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_METER_HPP__
