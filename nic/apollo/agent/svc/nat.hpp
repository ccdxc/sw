// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_NAT_HPP__
#define __AGENT_SVC_NAT_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/nat.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::NatSvc;
using pds::NatPortBlockRequest;
using pds::NatPortBlockResponse;
using pds::NatPortBlockDeleteRequest;
using pds::NatPortBlockDeleteResponse;
using pds::NatPortBlockGetRequest;
using pds::NatPortBlockGetResponse;

class NatSvcImpl final : public NatSvc::Service {
public:
    Status NatPortBlockCreate(ServerContext *context,
                              const pds::NatPortBlockRequest *req,
                              pds::NatPortBlockResponse *rsp) override;
    Status NatPortBlockPolicyGet(ServerContext *context,
                                 const pds::NatPortBlockGetRequest *req,
                                 pds::NatPortBlockGetResponse *rsp) override;
    Status NatPortBlockDelete(ServerContext *context,
                              const pds::NatPortBlockDeleteRequest *proto_req,
                              pds::NatPortBlockDeleteResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_NAT_HPP__
