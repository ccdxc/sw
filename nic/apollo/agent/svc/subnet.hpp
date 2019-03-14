// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_SUBNET_HPP__
#define __AGENT_SVC_SUBNET_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/subnet.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::SubnetSvc;
using tpc::SubnetRequest;
using tpc::SubnetSpec;
using tpc::SubnetResponse;

class SubnetSvcImpl final : public SubnetSvc::Service {
public:
    Status SubnetCreate(ServerContext *context, const tpc::SubnetRequest *req,
                        tpc::SubnetResponse *rsp) override;
};

#endif    // __AGENT_SVC_SUBNET_HPP__
