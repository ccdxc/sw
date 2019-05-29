// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_VPC_HPP__
#define __AGENT_SVC_VPC_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/vpc.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::VPCSvc;
using pds::VPCRequest;
using pds::VPCSpec;
using pds::VPCResponse;

class VPCSvcImpl final : public VPCSvc::Service {
public:
    Status VPCCreate(ServerContext *context, const pds::VPCRequest *req,
                     pds::VPCResponse *rsp) override;
    Status VPCUpdate(ServerContext *context, const pds::VPCRequest *req,
                     pds::VPCResponse *rsp) override;
    Status VPCDelete(ServerContext *context,
                     const pds::VPCDeleteRequest *proto_req,
                     pds::VPCDeleteResponse *proto_rsp) override;
    Status VPCGet(ServerContext *context,
                  const pds::VPCGetRequest *proto_req,
                  pds::VPCGetResponse *proto_rsp) override;
    Status VPCPeerCreate(ServerContext *context, const pds::VPCPeerRequest *req,
                         pds::VPCPeerResponse *rsp) override;
    Status VPCPeerDelete(ServerContext *context,
                         const pds::VPCPeerDeleteRequest *proto_req,
                         pds::VPCPeerDeleteResponse *proto_rsp) override;
    Status VPCPeerGet(ServerContext *context,
                      const pds::VPCPeerGetRequest *proto_req,
                      pds::VPCPeerGetResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_VPC_HPP__
