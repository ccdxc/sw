// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_MIRROR_HPP__
#define __AGENT_SVC_MIRROR_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/mirror.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::MirrorSvc;
using pds::MirrorPolicyRequest;
using pds::MirrorPolicySpec;
using pds::MirrorPolicyResponse;

class MirrorSvcImpl final : public MirrorSvc::Service {
public:
    Status MirrorPolicyCreate(ServerContext *context,
                              const pds::MirrorPolicyRequest *req,
                              pds::MirrorPolicyResponse *rsp) override;
    Status MirrorPolicyDelete(ServerContext *context,
                              const pds::MirrorPolicyDeleteRequest *proto_req,
                              pds::MirrorPolicyDeleteResponse *proto_rsp) override;
    Status MirrorPolicyGet(ServerContext *context,
                           const pds::MirrorPolicyGetRequest *proto_req,
                           pds::MirrorPolicyGetResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_MIRROR_HPP__
