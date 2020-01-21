//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_lif.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/agent/svc/interface.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/core/interface.hpp"

Status
IfSvcImpl::InterfaceCreate(ServerContext *context,
                           const pds::InterfaceRequest *proto_req,
                           pds::InterfaceResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    Status status = Status::OK;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_if_spec_t *api_spec;

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
            PDS_TRACE_ERR("Failed to create a new batch, interface creation "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_if_spec_t *)
                    core::agent_state::state()->if_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto request = proto_req->request(i);
        ret = pds_if_proto_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->if_slab()->free(api_spec);
            goto end;
        }
        ret = core::interface_create(api_spec, bctxt);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->if_slab()->free(api_spec);
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
IfSvcImpl::InterfaceUpdate(ServerContext *context,
                           const pds::InterfaceRequest *proto_req,
                           pds::InterfaceResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    Status status = Status::OK;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_if_spec_t *api_spec;

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
            PDS_TRACE_ERR("Failed to create a new batch, interface update "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_if_spec_t *)
                    core::agent_state::state()->if_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto request = proto_req->request(i);
        ret = pds_if_proto_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->if_slab()->free(api_spec);
            goto end;
        }
        ret = core::interface_update(api_spec, bctxt);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->if_slab()->free(api_spec);
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
IfSvcImpl::InterfaceDelete(ServerContext *context,
                           const pds::InterfaceDeleteRequest *proto_req,
                           pds::InterfaceDeleteResponse *proto_rsp) {
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
            PDS_TRACE_ERR("Failed to create a new batch, interface delete "
                          "failed");
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = core::interface_delete(&key, bctxt);
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
IfSvcImpl::InterfaceGet(ServerContext *context,
                        const pds::InterfaceGetRequest *proto_req,
                        pds::InterfaceGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_if_info_t info;
    pds_obj_key_t key = { 0 };

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = core::interface_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        pds_if_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::interface_get_all(pds_if_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

Status
IfSvcImpl::LifGet(ServerContext *context,
                  const pds::LifGetRequest *proto_req,
                  pds::LifGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_obj_key_t key;

    if (proto_req) {
        for (int i = 0; i < proto_req->id_size(); i ++) {
            pds_lif_info_t info = { 0 };
            pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
            ret = pds_lif_read(&key, &info);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            pds_lif_api_info_to_proto(&info, proto_rsp);
        }
        if (proto_req->id_size() == 0) {
            ret = pds_lif_read_all(pds_lif_api_info_to_proto, proto_rsp);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        }
    }
    return Status::OK;
}
