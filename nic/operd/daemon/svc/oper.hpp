//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//------------------------------------------------------------------------------

#ifndef __OPERD_SVC_OPER_HPP__
#define __OPERD_SVC_OPER_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/oper.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using operd::OperSvc;
using operd::TechSupportRequest;
using operd::TechSupportResponse;

class OperSvcImpl final : public OperSvc::Service {
public:
    Status TechSupportCollect(ServerContext* context,
                              const TechSupportRequest *req,
                              TechSupportResponse* rsp) override;
};

#endif // __OPERD_SVC_OPER_HPP__
