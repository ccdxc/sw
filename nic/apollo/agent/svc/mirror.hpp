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
using pds::MirrorSessionRequest;
using pds::MirrorSessionSpec;
using pds::MirrorSessionResponse;

class MirrorSvcImpl final : public MirrorSvc::Service {
public:
    Status MirrorSessionCreate(ServerContext *context,
                               const pds::MirrorSessionRequest *req,
                               pds::MirrorSessionResponse *rsp) override;
    Status MirrorSessionUpdate(ServerContext *context,
                               const pds::MirrorSessionRequest *req,
                               pds::MirrorSessionResponse *rsp) override;
    Status MirrorSessionDelete(ServerContext *context,
                               const pds::MirrorSessionDeleteRequest *proto_req,
                               pds::MirrorSessionDeleteResponse *proto_rsp) override;
    Status MirrorSessionGet(ServerContext *context,
                            const pds::MirrorSessionGetRequest *proto_req,
                            pds::MirrorSessionGetResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_MIRROR_HPP__
