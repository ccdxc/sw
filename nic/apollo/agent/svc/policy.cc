//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/policy.hpp"

#if 0
#include "nic/apollo/test/flow_test/flow_test.hpp"
extern flow_test *g_flow_test_obj;
#endif

// Build policy API spec from protobuf spec
static inline void
pds_agent_policy_api_spec_fill (const pds::SecurityPolicySpec &proto_spec,
                                 pds_policy_spec_t *api_spec)
{
    // TODO
}

Status
SecurityPolicySvcImpl::SecurityPolicyCreate(ServerContext *context,
                              const pds::SecurityPolicyRequest *proto_req,
                              pds::SecurityPolicyResponse *proto_rsp) {
    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            pds_policy_spec_t api_spec; 
            pds_agent_policy_api_spec_fill(proto_req->request(i), &api_spec);
            if (pds_policy_create(&api_spec) == sdk::SDK_RET_OK)
                return Status::OK;
        }
    }
    return Status::CANCELLED;
}
