// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_MAPPING_HPP__
#define __AGENT_SVC_MAPPING_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/policy.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::SecurityPolicySvc;
using pds::SecurityPolicyRequest;
using pds::SecurityPolicySpec;
using pds::SecurityPolicyResponse;

class SecurityPolicySvcImpl final : public SecurityPolicySvc::Service {
public:
    Status SecurityPolicyCreate(ServerContext *context, const pds::SecurityPolicyRequest *req,
                         pds::SecurityPolicyResponse *rsp) override;
};

#endif    // __AGENT_SVC_MAPPING_HPP__
