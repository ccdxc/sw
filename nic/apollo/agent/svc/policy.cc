//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/policy_svc.hpp"

Status
SecurityPolicySvcImpl::SecurityPolicyCreate(ServerContext *context,
                                            const pds::SecurityPolicyRequest *proto_req,
                                            pds::SecurityPolicyResponse *proto_rsp) {
    pds_svc_security_policy_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityPolicyUpdate(ServerContext *context,
                                            const pds::SecurityPolicyRequest *proto_req,
                                            pds::SecurityPolicyResponse *proto_rsp) {
    pds_svc_security_policy_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityPolicyDelete(ServerContext *context,
                                            const pds::SecurityPolicyDeleteRequest *proto_req,
                                            pds::SecurityPolicyDeleteResponse *proto_rsp) {
    pds_svc_security_policy_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityPolicyGet(ServerContext *context,
                                         const pds::SecurityPolicyGetRequest *proto_req,
                                         pds::SecurityPolicyGetResponse *proto_rsp) {
    pds_svc_security_policy_get(proto_req, proto_rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityProfileCreate(ServerContext *context,
                                             const pds::SecurityProfileRequest *proto_req,
                                             pds::SecurityProfileResponse *proto_rsp) {
    pds_svc_security_profile_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityProfileUpdate(ServerContext *context,
                                             const pds::SecurityProfileRequest *proto_req,
                                             pds::SecurityProfileResponse *proto_rsp) {
    pds_svc_security_profile_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityProfileDelete(ServerContext *context,
                                             const pds::SecurityProfileDeleteRequest *proto_req,
                                             pds::SecurityProfileDeleteResponse *proto_rsp) {
    pds_svc_security_profile_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityProfileGet(ServerContext *context,
                                          const pds::SecurityProfileGetRequest *proto_req,
                                          pds::SecurityProfileGetResponse *proto_rsp) {
    pds_svc_security_profile_get(proto_req, proto_rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityRuleCreate(ServerContext *context,
                                          const pds::SecurityRuleRequest *proto_req,
                                          pds::SecurityRuleResponse *proto_rsp) {
    pds_svc_security_rule_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityRuleUpdate(ServerContext *context,
                                          const pds::SecurityRuleRequest *proto_req,
                                          pds::SecurityRuleResponse *proto_rsp) {
    pds_svc_security_rule_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityRuleDelete(ServerContext *context,
                                          const pds::SecurityRuleDeleteRequest *req,
                                          pds::SecurityRuleDeleteResponse *rsp) {
    pds_svc_security_rule_delete(req, rsp);
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityRuleGet(ServerContext *context,
                                       const pds::SecurityRuleGetRequest *req,
                                       pds::SecurityRuleGetResponse *rsp) {
    pds_svc_security_rule_get(req, rsp);
    return Status::OK;
}

