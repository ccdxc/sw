// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_VPC_HPP__
#define __AGENT_SVC_VPC_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/bgp.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::BGP;
using pds::BGPGlobalSpec;
using pds::BGPGlobalSpecResponseMsg;

class BGPSvcImpl final : public BGP::Service {
public:
    Status BGPGlobalSpecCreate(ServerContext *context, const pds::BGPGlobalSpec *req,
                     pds::BGPGlobalSpecResponseMsg *rsp) override;
    Status BGPGlobalSpecUpdate(ServerContext *context, const pds::BGPGlobalSpec *req,
                     pds::BGPGlobalSpecResponseMsg *rsp) override;
    Status BGPGlobalSpecDelete(ServerContext *context,
                     const pds::BGPGlobalSpec *proto_req,
                     pds::BGPGlobalSpecResponseMsg *proto_rsp) override;
    Status BGPGlobalSpecGet(ServerContext *context,
                  const pds::BGPGlobalSpec *proto_req,
                  pds::BGPGlobalSpecResponseMsg *proto_rsp) override;
};

#endif    // __AGENT_SVC_VPC_HPP__
