//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/policy.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/policy.hpp"

#if 0
#include "nic/apollo/test/flow_test/flow_test.hpp"
extern flow_test *g_flow_test_obj;
#endif

Status
SecurityPolicySvcImpl::SecurityPolicyCreate(ServerContext *context,
                                            const pds::SecurityPolicyRequest *proto_req,
                                            pds::SecurityPolicyResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_policy_spec_t api_spec = {};
    pds_policy_key_t key = {0};

    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            ret = pds_policy_proto_to_api_spec(&api_spec,
                                               proto_req->request(i));
            if (unlikely(ret != SDK_RET_OK)) {
                return Status::CANCELLED;
            }
            auto request = proto_req->request(i);
            key.id = request.id();
            ret = core::policy_create(&key, &api_spec);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec.rules);
            api_spec.rules = NULL;
            if (ret != SDK_RET_OK) {
                return Status::CANCELLED;
            }
        }
    } else  {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityPolicyUpdate(ServerContext *context,
                                            const pds::SecurityPolicyRequest *proto_req,
                                            pds::SecurityPolicyResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_policy_spec_t api_spec = {};
    pds_policy_key_t key = {0};

    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            ret = pds_policy_proto_to_api_spec(&api_spec,
                                               proto_req->request(i));
            if (unlikely(ret != SDK_RET_OK)) {
                return Status::CANCELLED;
            }
            auto request = proto_req->request(i);
            key.id = request.id();
            ret = core::policy_update(&key, &api_spec);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec.rules);
            api_spec.rules = NULL;
            if (ret != SDK_RET_OK) {
                return Status::CANCELLED;
            }
        }
    } else  {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    return Status::OK;
}
