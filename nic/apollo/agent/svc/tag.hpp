// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_TAG_HPP__
#define __AGENT_SVC_TAG_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/tags.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::TagSvc;
using pds::TagRequest;
using pds::TagSpec;
using pds::TagResponse;
using pds::TagGetRequest;
using pds::TagGetResponse;
using pds::TagDeleteRequest;
using pds::TagDeleteResponse;

class TagSvcImpl final : public TagSvc::Service {
public:
    Status TagCreate(ServerContext *context, const pds::TagRequest *req,
                     pds::TagResponse *rsp) override;
    Status TagUpdate(ServerContext *context, const pds::TagRequest *req,
                     pds::TagResponse *rsp) override;
    Status TagDelete(ServerContext *context,
                     const pds::TagDeleteRequest *proto_req,
                     pds::TagDeleteResponse *proto_rsp) override;
    Status TagGet(ServerContext *context,
                  const pds::TagGetRequest *proto_req,
                  pds::TagGetResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_TAG_HPP__
