//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_dhcp.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/dhcp_svc.hpp"
#include "nic/apollo/agent/trace.hpp"

Status
DHCPSvcImpl::DHCPPolicyCreate(ServerContext *context,
                              const pds::DHCPPolicyRequest *proto_req,
                              pds::DHCPPolicyResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_dhcp_policy_spec_t api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if ((proto_req == NULL) || (proto_req->request_size() == 0)) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::CANCELLED;
    }

    // create an internal batch, if this is not part of an existing API batch
    bctxt = proto_req->batchctxt().batchcookie();
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        batch_params.epoch = core::agent_state::state()->new_epoch();
        batch_params.async = false;
        bctxt = pds_batch_start(&batch_params);
        if (bctxt == PDS_BATCH_CTXT_INVALID) {
            PDS_TRACE_ERR("Failed to create a new batch, DHCP policy "
                          "creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        auto request = proto_req->request(i);
        pds_dhcp_policy_proto_to_api_spec(&api_spec, request);
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_dhcp_policy_create(&api_spec, bctxt);
            if (ret != SDK_RET_OK) {
                goto end;
            }
        }
    }
    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;

end:

    if (batched_internally) {
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::CANCELLED;
}

Status
DHCPSvcImpl::DHCPPolicyUpdate(ServerContext *context,
                              const pds::DHCPPolicyRequest *proto_req,
                              pds::DHCPPolicyResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_dhcp_policy_spec_t api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if ((proto_req == NULL) || (proto_req->request_size() == 0)) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::CANCELLED;
    }

    // create an internal batch, if this is not part of an existing API batch
    bctxt = proto_req->batchctxt().batchcookie();
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        batch_params.epoch = core::agent_state::state()->new_epoch();
        batch_params.async = false;
        bctxt = pds_batch_start(&batch_params);
        if (bctxt == PDS_BATCH_CTXT_INVALID) {
            PDS_TRACE_ERR("Failed to create a new batch, DHCP policy "
                          "update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        auto request = proto_req->request(i);
        pds_dhcp_policy_proto_to_api_spec(&api_spec, request);
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_dhcp_policy_update(&api_spec, bctxt);
            if (ret != SDK_RET_OK) {
                goto end;
            }
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;

end:

    if (batched_internally) {
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::CANCELLED;
}

Status
DHCPSvcImpl::DHCPPolicyDelete(ServerContext *context,
                              const pds::DHCPPolicyDeleteRequest *proto_req,
                              pds::DHCPPolicyDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if ((proto_req == NULL) || (proto_req->id_size() == 0)) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::CANCELLED;
    }

    // create an internal batch, if this is not part of an existing API batch
    bctxt = proto_req->batchctxt().batchcookie();
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        batch_params.epoch = core::agent_state::state()->new_epoch();
        batch_params.async = false;
        bctxt = pds_batch_start(&batch_params);
        if (bctxt == PDS_BATCH_CTXT_INVALID) {
            PDS_TRACE_ERR("Failed to create a new batch, DHCP policy "
                          "deletion failed");
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_dhcp_policy_delete(&key, bctxt);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }
    proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;

end:

    if (batched_internally) {
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    return Status::CANCELLED;
}

Status
DHCPSvcImpl::DHCPPolicyGet(ServerContext *context,
                           const pds::DHCPPolicyGetRequest *proto_req,
                           pds::DHCPPolicyGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_obj_key_t key = { 0 };
    pds_dhcp_policy_info_t info = { 0 };

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    if (proto_req->id_size() == 0) {
        // get all
        ret = pds_dhcp_policy_read_all(pds_dhcp_policy_api_info_to_proto,
                                       proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    for (int i = 0; i < proto_req->id_size(); i ++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_dhcp_policy_read(&key, &info);
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        if (ret != SDK_RET_OK) {
            break;
        }
        pds_dhcp_policy_api_info_to_proto(&info, proto_rsp);
    }
    return Status::OK;
}
