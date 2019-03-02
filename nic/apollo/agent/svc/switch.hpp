// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_SWITCH_HPP__
#define __AGENT_SVC_SWITCH_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/switch.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::SwitchSvc;
using tpc::SwitchSpec;
using tpc::SwitchStatus;

class SwitchSvcImpl final : public SwitchSvc::Service {
public:
    Status SwitchCreate(ServerContext *context, const tpc::SwitchSpec *spec,
                        tpc::SwitchStatus *status) override;
};

#endif    // __AGENT_SVC_SWITCH_HPP__
