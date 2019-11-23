// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_POLICER_HPP__
#define __AGENT_SVC_POLICER_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/policer.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::PolicerSvc;
using pds::PolicerRequest;
using pds::PolicerSpec;
using pds::PolicerResponse;

class PolicerSvcImpl final : public PolicerSvc::Service {
public:
    Status PolicerCreate(ServerContext *context, const pds::PolicerRequest *req,
                         pds::PolicerResponse *rsp) override;
    Status PolicerUpdate(ServerContext *context, const pds::PolicerRequest *req,
                         pds::PolicerResponse *rsp) override;
    Status PolicerDelete(ServerContext *context,
                         const pds::PolicerDeleteRequest *proto_req,
                         pds::PolicerDeleteResponse *proto_rsp) override;
    Status PolicerGet(ServerContext *context,
                      const pds::PolicerGetRequest *proto_req,
                      pds::PolicerGetResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_POLICER_HPP__
