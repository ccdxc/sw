// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_SUBNET_HPP__
#define __AGENT_SVC_SUBNET_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/subnet.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::SubnetSvc;
using pds::SubnetRequest;
using pds::SubnetSpec;
using pds::SubnetResponse;

class SubnetSvcImpl final : public SubnetSvc::Service {
public:
    Status SubnetCreate(ServerContext *context, const pds::SubnetRequest *req,
                        pds::SubnetResponse *rsp) override;
    Status SubnetUpdate(ServerContext *context, const pds::SubnetRequest *req,
                        pds::SubnetResponse *rsp) override;
    Status SubnetDelete(ServerContext *context,
                        const pds::SubnetDeleteRequest *proto_req,
                        pds::SubnetDeleteResponse *proto_rsp) override;
    Status SubnetGet(ServerContext *context,
                     const pds::SubnetGetRequest *proto_req,
                     pds::SubnetGetResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_SUBNET_HPP__
