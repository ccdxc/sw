// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_TUNNEL_HPP__
#define __AGENT_SVC_TUNNEL_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/tunnel.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::TunnelSvc;
using pds::TunnelRequest;
using pds::TunnelSpec;
using pds::TunnelResponse;

class TunnelSvcImpl final : public TunnelSvc::Service {
public:
    Status TunnelCreate(ServerContext *context, const pds::TunnelRequest *req,
                        pds::TunnelResponse *rsp) override;
    Status TunnelDelete(ServerContext *context,
                        const pds::TunnelDeleteRequest *proto_req,
                        pds::TunnelDeleteResponse *proto_rsp);
};

#endif    // __AGENT_SVC_TUNNEL_HPP__
