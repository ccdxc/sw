// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_MAPPING_HPP__
#define __AGENT_SVC_MAPPING_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/mapping.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::MappingSvc;
using tpc::MappingRequest;
using tpc::MappingSpec;
using tpc::MappingResponse;

class MappingSvcImpl final : public MappingSvc::Service {
public:
    Status MappingCreate(ServerContext *context, const tpc::MappingRequest *req,
                         tpc::MappingResponse *rsp) override;
};

#endif    // __AGENT_SVC_MAPPING_HPP__
