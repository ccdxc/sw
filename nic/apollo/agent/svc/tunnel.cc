//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"
#include "nic/apollo/agent/svc/tunnel.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/hooks.hpp"

// create tunnel object
Status
TunnelSvcImpl::TunnelCreate(ServerContext *context,
                            const pds::TunnelRequest *proto_req,
                            pds::TunnelResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_batch_ctxt_t bctxt;
    pds_tep_spec_t *api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, tunnel creation "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::OK;
        }
        batched_internally = true;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_tep_spec_t *)
                    core::agent_state::state()->tep_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto request = proto_req->request(i);
        pds_obj_key_proto_to_api_spec(&key, request.id());
        pds_tep_proto_to_api_spec(api_spec, request);
        hooks::tunnel_create(api_spec);
        ret = core::tep_create(&key, api_spec, bctxt);
        if (ret != SDK_RET_OK) {
            goto end;
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
    return Status::OK;
}

// update tunnel object
Status
TunnelSvcImpl::TunnelUpdate(ServerContext *context,
                            const pds::TunnelRequest *proto_req,
                            pds::TunnelResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_batch_ctxt_t bctxt;
    pds_tep_spec_t *api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, tunnel update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::OK;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec =
            (pds_tep_spec_t *)core::agent_state::state()->tep_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto request = proto_req->request(i);
        pds_obj_key_proto_to_api_spec(&key, request.id());
        pds_tep_proto_to_api_spec(api_spec, request);
        ret = core::tep_update(&key, api_spec, bctxt);
        if (ret != SDK_RET_OK) {
            goto end;
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
    return Status::OK;
}

Status
TunnelSvcImpl::TunnelDelete(ServerContext *context,
                            const pds::TunnelDeleteRequest *proto_req,
                            pds::TunnelDeleteResponse *proto_rsp) {
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
            PDS_TRACE_ERR("Failed to create a new batch, vpc delete failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::OK;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = core::tep_delete(&key, bctxt);
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
    return Status::OK;
}

Status
TunnelSvcImpl::TunnelGet(ServerContext *context,
                         const pds::TunnelGetRequest *proto_req,
                         pds::TunnelGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_tep_info_t info = {0};

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    if (proto_req->id_size() == 0) {
        ret = core::tep_get_all(pds_tep_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = core::tep_get(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            break;
        }
        auto response = proto_rsp->add_response();
        pds_tep_api_spec_to_proto(response->mutable_spec(), &info.spec);
        pds_tep_api_status_to_proto(response->mutable_status(),
                                    &info.status);
        pds_tep_api_stats_to_proto(response->mutable_stats(), &info.stats);
    }
    return Status::OK;
}
