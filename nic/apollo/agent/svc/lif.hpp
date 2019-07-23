// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_LIF_HPP__
#define __AGENT_SVC_LIF_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include "gen/proto/types.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::IfSvc;
using pds::LifGetRequest;
using pds::LifGetResponse;
using pds::Lif;
using types::LifType;

class IfSvcImpl final : public IfSvc::Service {
public:
    Status LifGet(ServerContext *context, const pds::LifGetRequest *req,
                  pds::LifGetResponse *rsp) override;
};

#endif    // __AGENT_SVC_LIF_HPP__
