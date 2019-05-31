// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_SERVICE_HPP__
#define __AGENT_SVC_SERVICE_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/service.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::Svc;
using pds::SvcMappingRequest;
using pds::SvcMappingSpec;
using pds::SvcMappingResponse;

class SvcImpl final : public Svc::Service {
public:
    Status SvcMappingCreate(ServerContext *context, const pds::SvcMappingRequest *req,
                            pds::SvcMappingResponse *rsp) override;
    Status SvcMappingUpdate(ServerContext *context, const pds::SvcMappingRequest *req,
                            pds::SvcMappingResponse *rsp) override;
    Status SvcMappingDelete(ServerContext *context,
                            const pds::SvcMappingDeleteRequest *proto_req,
                            pds::SvcMappingDeleteResponse *proto_rsp) override;
    Status SvcMappingGet(ServerContext *context,
                         const pds::SvcMappingGetRequest *proto_req,
                         pds::SvcMappingGetResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_SERVICE_HPP__
