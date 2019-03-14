// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_TUNNEL_HPP__
#define __AGENT_SVC_TUNNEL_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/tunnel.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::TunnelSvc;
using tpc::TunnelRequest;
using tpc::TunnelSpec;
using tpc::TunnelResponse;

class TunnelSvcImpl final : public TunnelSvc::Service {
public:
    Status TunnelCreate(ServerContext *context, const tpc::TunnelRequest *req,
                        tpc::TunnelResponse *rsp) override;
};

#endif    // __AGENT_SVC_TUNNEL_HPP__
