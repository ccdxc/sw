//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/service.hpp"
#include "nic/apollo/agent/svc/service_svc.hpp"
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
    pds_svc_mapping_spec_t api_spec;

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
            PDS_TRACE_ERR("Failed to create a new batch, service mapping "
                          "creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        pds_service_proto_to_api_spec(&api_spec, proto_req->request(i));
        hooks::svc_mapping_create(&api_spec);
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_svc_mapping_create(&api_spec, bctxt);
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
SvcImpl::SvcMappingUpdate(ServerContext *context,
                          const pds::SvcMappingRequest *proto_req,
                          pds::SvcMappingResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_svc_mapping_spec_t api_spec;

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
            PDS_TRACE_ERR("Failed to create a new batch, service mapping "
                          "update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        pds_service_proto_to_api_spec(&api_spec, proto_req->request(i));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_svc_mapping_update(&api_spec, bctxt);
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
SvcImpl::SvcMappingDelete(ServerContext *context,
                          const pds::SvcMappingDeleteRequest *proto_req,
                          pds::SvcMappingDeleteResponse *proto_rsp) {
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
            PDS_TRACE_ERR("Failed to create a new batch, service mapping "
                          "delete failed");
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_svc_mapping_delete(&key, bctxt);
            if (ret != SDK_RET_OK) {
                goto end;
            }
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
    pds_obj_key_t key;
    pds_svc_mapping_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    if (proto_req->id_size() == 0) {
        ret = pds_svc_mapping_read_all(pds_service_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_svc_mapping_read(&key, &info);
            if (ret == SDK_RET_OK) {
                pds_service_api_info_to_proto(&info, proto_rsp);
                proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
            } else {
                proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
                break;
            }
        }
    }
    return Status::OK;
}
