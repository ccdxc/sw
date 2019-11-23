//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/service.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/service.hpp"
#include "nic/apollo/agent/hooks.hpp"

Status
SvcImpl::SvcMappingCreate(ServerContext *context,
                          const pds::SvcMappingRequest *proto_req,
                          pds::SvcMappingResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_svc_mapping_spec_t *api_spec;
    pds_svc_mapping_key_t key = { 0 };

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
            PDS_TRACE_ERR("Failed to create a new batch, vpc creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_svc_mapping_spec_t *)
                    core::agent_state::state()->service_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto request = proto_req->request(i);
        key.vpc.id = request.key().vpcid();
        key.backend_port = request.key().backendport();
        ipaddr_proto_spec_to_api_spec(&key.backend_ip, request.key().backendip());
        pds_service_proto_to_api_spec(api_spec, request);
        hooks::svc_mapping_create(api_spec);
        ret = core::service_create(&key, api_spec, bctxt);
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
    return Status::CANCELLED;
}

Status
SvcImpl::SvcMappingUpdate(ServerContext *context,
                          const pds::SvcMappingRequest *proto_req,
                          pds::SvcMappingResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_svc_mapping_spec_t *api_spec;
    pds_svc_mapping_key_t key = { 0 };

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
            PDS_TRACE_ERR("Failed to create a new batch, vpc creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_svc_mapping_spec_t *)
                    core::agent_state::state()->service_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto request = proto_req->request(i);
        key.vpc.id = request.key().vpcid();
        key.backend_port = request.key().backendport();
        ipaddr_proto_spec_to_api_spec(&key.backend_ip, request.key().backendip());
        pds_service_proto_to_api_spec(api_spec, request);
        ret = core::service_update(&key, api_spec, bctxt);
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
    return Status::CANCELLED;
}

Status
SvcImpl::SvcMappingDelete(ServerContext *context,
                          const pds::SvcMappingDeleteRequest *proto_req,
                          pds::SvcMappingDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_svc_mapping_key_t key = { 0 };

    if ((proto_req == NULL) || (proto_req->key_size() == 0)) {
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
            PDS_TRACE_ERR("Failed to create a new batch, vpc creation failed");
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->key_size(); i++) {
        key.vpc.id = proto_req->key(i).vpcid();
        key.backend_port = proto_req->key(i).backendport();
        ipaddr_proto_spec_to_api_spec(&key.backend_ip, proto_req->key(i).backendip());
        ret = core::service_delete(&key, bctxt);
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
SvcImpl::SvcMappingGet(ServerContext *context,
                       const pds::SvcMappingGetRequest *proto_req,
                       pds::SvcMappingGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_svc_mapping_key_t key = { 0 };
    pds_svc_mapping_info_t info = { 0 };

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    if (proto_req->key_size() == 0) {
        // get all
        ret = core::service_get_all(pds_service_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->key_size(); i++) {
        key.vpc.id = proto_req->key(i).vpcid();
        key.backend_port = proto_req->key(i).backendport();
        ipaddr_proto_spec_to_api_spec(&key.backend_ip, proto_req->key(i).backendip());
        ret = core::service_get(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            break;
        }
        auto response = proto_rsp->add_response();
        pds_service_api_spec_to_proto(
                response->mutable_spec(), &info.spec);
        pds_service_api_status_to_proto(
                response->mutable_status(), &info.status);
        pds_service_api_stats_to_proto(
                response->mutable_stats(), &info.stats);
    }
    return Status::OK;
}
