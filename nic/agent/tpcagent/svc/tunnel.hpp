// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __SVC_TUNNEL_HPP__
#define __SVC_TUNNEL_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/tunnel.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::TunnelSvc;
using tpc::TunnelSpec;
using tpc::TunnelStatus;

class TunnelSvcImpl final : public TunnelSvc::Service {
public:
    Status TunnelCreate(ServerContext *context,
                       const tpc::TunnelSpec *spec,
                       tpc::TunnelStatus *status) override;
};

#endif    // __SVC_TUNNEL_HPP__
