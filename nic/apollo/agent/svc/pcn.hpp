// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_PCN_HPP__
#define __AGENT_SVC_PCN_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/pcn.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::PCNSvc;
using tpc::PCNSpec;
using tpc::PCNStatus;

class PCNSvcImpl final : public PCNSvc::Service {
public:
    Status PCNCreate(ServerContext *context, const tpc::PCNSpec *spec,
                     tpc::PCNStatus *status) override;
};

#endif    // __AGENT_SVC_PCN_HPP__
