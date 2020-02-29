// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_OPER_HPP__
#define __AGENT_SVC_OPER_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/oper.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;
using types::Empty;

using pds::OperSvc;
using pds::TechSupportRequest;
using pds::TechSupportResponse;

class OperSvcImpl final : public OperSvc::Service {
public:
    Status TechSupportCollect(ServerContext* context,
                              const TechSupportRequest *req,
                              TechSupportResponse* rsp) override;
};

#endif // __AGENT_SVC_OPER_HPP__
