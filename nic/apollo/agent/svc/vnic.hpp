// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_VNIC_HPP__
#define __AGENT_SVC_VNIC_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/vnic.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::VnicSvc;
using tpc::VnicRequest;
using tpc::VnicSpec;
using tpc::VnicResponse;

class VnicSvcImpl final : public VnicSvc::Service {
public:
    Status VnicCreate(ServerContext *context, const tpc::VnicRequest *req,
                      tpc::VnicResponse *rsp) override;
};

#endif    // __AGENT_SVC_VNIC_HPP__
