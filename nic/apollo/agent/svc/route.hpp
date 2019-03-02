// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_ROUTE_HPP__
#define __AGENT_SVC_ROUTE_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/route.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::RouteSvc;
using tpc::RouteTableSpec;
using tpc::RouteTableStatus;

class RouteSvcImpl final : public RouteSvc::Service {
public:
    Status RouteTableCreate(ServerContext *context,
                            const tpc::RouteTableSpec *spec,
                            tpc::RouteTableStatus *status) override;
};

#endif    // __AGENT_SVC_ROUTE_HPP__
