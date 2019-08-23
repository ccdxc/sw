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

Status
SecurityPolicySvcImpl::SecurityPolicyCreate(ServerContext *context,
                                            const pds::SecurityPolicyRequest *proto_req,
                                            pds::SecurityPolicyResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_policy_spec_t *api_spec;
    pds_policy_key_t key = {0};

    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            api_spec = (pds_policy_spec_t *)
                       core::agent_state::state()->policy_slab()->alloc();
            if (api_spec == NULL) {
                proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
                break;
            }
            ret = pds_policy_proto_to_api_spec(api_spec,
                                               proto_req->request(i));
            if (unlikely(ret != SDK_RET_OK)) {
                return Status::CANCELLED;
            }
            auto request = proto_req->request(i);
            key.id = request.id();
            ret = core::policy_create(&key, api_spec);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            if (api_spec->rules != NULL) {
                SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec->rules);
                api_spec->rules = NULL;
            }
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
SecurityPolicySvcImpl::SecurityPolicyGet(ServerContext *context,
                                         const pds::SecurityPolicyGetRequest *proto_req,
                                         pds::SecurityPolicyGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_policy_key_t key;
    pds_policy_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::policy_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        policy_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::policy_get_all(policy_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityPolicyUpdate(ServerContext *context,
                                            const pds::SecurityPolicyRequest *proto_req,
                                            pds::SecurityPolicyResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_policy_spec_t *api_spec;
    pds_policy_key_t key = {0};

    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            api_spec = (pds_policy_spec_t *)
                       core::agent_state::state()->policy_slab()->alloc();
            if (api_spec == NULL) {
                proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
                break;
            }
            ret = pds_policy_proto_to_api_spec(api_spec,
                                               proto_req->request(i));
            if (unlikely(ret != SDK_RET_OK)) {
                return Status::CANCELLED;
            }
            auto request = proto_req->request(i);
            key.id = request.id();
            ret = core::policy_update(&key, api_spec);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            if (api_spec->rules != NULL) {
                SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec->rules);
                api_spec->rules = NULL;
            }
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
SecurityPolicySvcImpl::SecurityPolicyDelete(ServerContext *context,
                                            const pds::SecurityPolicyDeleteRequest *proto_req,
                                            pds::SecurityPolicyDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_policy_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::policy_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}