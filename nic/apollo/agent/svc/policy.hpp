// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_MAPPING_HPP__
#define __AGENT_SVC_MAPPING_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/policy.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::SecurityPolicySvc;
using tpc::SecurityPolicyRequest;
using tpc::SecurityPolicySpec;
using tpc::SecurityPolicyResponse;

class SecurityPolicySvcImpl final : public SecurityPolicySvc::Service {
public:
    Status SecurityPolicyCreate(ServerContext *context, const tpc::SecurityPolicyRequest *req,
                         tpc::SecurityPolicyResponse *rsp) override;
};

#endif    // __AGENT_SVC_MAPPING_HPP__
