// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_POLICY_HPP__
#define __AGENT_SVC_POLICY_HPP__

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
    Status SecurityPolicyCreate(ServerContext *context,
                                const pds::SecurityPolicyRequest *req,
                                pds::SecurityPolicyResponse *rsp) override;
    Status SecurityPolicyUpdate(ServerContext *context,
                                const pds::SecurityPolicyRequest *req,
                                pds::SecurityPolicyResponse *rsp) override;
    Status SecurityPolicyGet(ServerContext *context,
                             const pds::SecurityPolicyGetRequest *req,
                             pds::SecurityPolicyGetResponse *rsp) override;
    Status SecurityPolicyDelete(ServerContext *context,
                                const pds::SecurityPolicyDeleteRequest *req,
                                pds::SecurityPolicyDeleteResponse *rsp) override;

    Status SecurityRuleCreate(ServerContext *context,
                                    const pds::SecurityRuleRequest *req,
                                    pds::SecurityRuleResponse*rsp) override;
    Status SecurityRuleUpdate(ServerContext *context,
                              const pds::SecurityRuleRequest *req,
                              pds::SecurityRuleResponse*rsp) override;
    Status SecurityRuleGet(ServerContext *context,
                           const pds::SecurityRuleGetRequest *req,
                           pds::SecurityRuleGetResponse *rsp) override;
    Status SecurityRuleDelete(ServerContext *context,
                              const pds::SecurityRuleDeleteRequest *req,
                              pds::SecurityRuleDeleteResponse *rsp) override;

    Status SecurityProfileCreate(ServerContext *context,
                                 const pds::SecurityProfileRequest *req,
                                 pds::SecurityProfileResponse *rsp) override;
    Status SecurityProfileUpdate(ServerContext *context,
                                 const pds::SecurityProfileRequest *req,
                                 pds::SecurityProfileResponse *rsp) override;
    Status SecurityProfileGet(ServerContext *context,
                              const pds::SecurityProfileGetRequest *req,
                              pds::SecurityProfileGetResponse *rsp) override;
    Status SecurityProfileDelete(ServerContext *context,
                                 const pds::SecurityProfileDeleteRequest *req,
                                 pds::SecurityProfileDeleteResponse *rsp) override;
};

#endif    // __AGENT_SVC_POLICY_HPP__
