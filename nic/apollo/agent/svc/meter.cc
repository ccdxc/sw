//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/meter.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/meter.hpp"

Status
MeterSvcImpl::MeterCreate(ServerContext *context,
                          const pds::MeterRequest *proto_req,
                          pds::MeterResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    Status status = Status::OK;
    pds_obj_key_t key = { 0 };
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_meter_spec_t *api_spec = NULL;

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
            PDS_TRACE_ERR("Failed to create a new batch, meter policy "
                          "creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_meter_spec_t *)
                    core::agent_state::state()->meter_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto request = proto_req->request(i);
        pds_obj_key_proto_to_api_spec(&key, request.id());
        ret = pds_meter_proto_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->meter_slab()->free(api_spec);
            goto end;
        }
        ret = core::meter_create(&key, api_spec, bctxt);

        // free the rules memory
        if (api_spec->rules != NULL) {
            SDK_FREE(PDS_MEM_ALLOC_ID_METER, api_spec->rules);
            api_spec->rules = NULL;
        }
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
MeterSvcImpl::MeterUpdate(ServerContext *context,
                          const pds::MeterRequest *proto_req,
                          pds::MeterResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_obj_key_t key = { 0 };
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_meter_spec_t *api_spec = NULL;

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
            PDS_TRACE_ERR("Failed to create a new batch, mirror policy update "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_meter_spec_t *)
                    core::agent_state::state()->meter_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto request = proto_req->request(i);
        pds_obj_key_proto_to_api_spec(&key, request.id());
        ret = pds_meter_proto_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->meter_slab()->free(api_spec);
            goto end;
        }
        ret = core::meter_update(&key, api_spec, bctxt);

        // free the rules memory
        if (api_spec->rules != NULL) {
            SDK_FREE(PDS_MEM_ALLOC_ID_METER, api_spec->rules);
            api_spec->rules = NULL;
        }
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

    // destroy the internal batch
    if (batched_internally) {
        pds_batch_destroy(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::CANCELLED;
}

Status
MeterSvcImpl::MeterDelete(ServerContext *context,
                          const pds::MeterDeleteRequest *proto_req,
                          pds::MeterDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_obj_key_t key = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch, mirror policy delete "
                          "failed");
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = core::meter_delete(&key, bctxt);
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

    // destroy the internal batch
    if (batched_internally) {
        pds_batch_destroy(bctxt);
    }
    proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    return Status::CANCELLED;
}

Status
MeterSvcImpl::MeterGet(ServerContext *context,
                       const pds::MeterGetRequest *proto_req,
                       pds::MeterGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_obj_key_t key = { 0 };
    pds_meter_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = core::meter_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        pds_meter_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::meter_get_all(pds_meter_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}
