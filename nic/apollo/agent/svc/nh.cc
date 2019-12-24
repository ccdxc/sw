//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/nh.hpp"
#include "nic/apollo/agent/core/nh_group.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/nh.hpp"
#include "nic/apollo/agent/trace.hpp"

Status
NhSvcImpl::NexthopCreate(ServerContext *context,
                         const pds::NexthopRequest *proto_req,
                         pds::NexthopResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_nexthop_spec_t *api_spec;
    pds_nexthop_key_t key = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch, nexthop creation "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_nexthop_spec_t *)
                    core::agent_state::state()->nh_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto proto_spec = proto_req->request(i);
        key.id = proto_spec.id();
        pds_nh_proto_to_api_spec(api_spec, proto_spec);
        ret = core::nh_create(&key, api_spec, bctxt);
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
NhSvcImpl::NexthopUpdate(ServerContext *context,
                         const pds::NexthopRequest *proto_req,
                         pds::NexthopResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_nexthop_spec_t *api_spec;
    pds_nexthop_key_t key = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch, nexthop update "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_nexthop_spec_t *)
                    core::agent_state::state()->nh_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto proto_spec = proto_req->request(i);
        key.id = proto_spec.id();
        pds_nh_proto_to_api_spec(api_spec, proto_spec);
        ret = core::nh_update(&key, api_spec, bctxt);
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
NhSvcImpl::NexthopDelete(ServerContext *context,
                         const pds::NexthopDeleteRequest *proto_req,
                         pds::NexthopDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_nexthop_key_t key = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch, nexthop delete "
                          "failed");
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::nh_delete(&key, bctxt);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }
    return Status::OK;

end:

    if (batched_internally) {
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    return Status::CANCELLED;
}

Status
NhSvcImpl::NexthopGet(ServerContext *context,
                      const pds::NexthopGetRequest *proto_req,
                      pds::NexthopGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_nexthop_key_t key = { 0 };
    pds_nexthop_info_t info = { 0 };

    PDS_TRACE_VERBOSE("Nexthop Get Received")

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    switch (proto_req->gettype_case()) {
    case pds::NexthopGetRequest::kType:
        {
            pds_nh_type_t type = proto_nh_type_to_pds_nh_type(proto_req->type());
            ret = core::nh_get_all(pds_nh_api_info_to_proto, proto_rsp, type);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        }
        break;
    case pds::NexthopGetRequest::kId:
        key.id = proto_req->id();
        ret = core::nh_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_NOT_FOUND);
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        pds_nh_api_info_to_proto(&info, proto_rsp);
        break;
    default:
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        break;
    }

    return Status::OK;
}

Status
NhSvcImpl::NhGroupCreate(ServerContext *context,
                         const pds::NhGroupRequest *proto_req,
                         pds::NhGroupResponse *proto_rsp) {
    sdk_ret_t ret = SDK_RET_OK;
    pds_batch_ctxt_t bctxt;
    pds_nexthop_group_key_t key = { 0 };
    pds_nexthop_group_spec_t *api_spec = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch %u",
                          batch_params.epoch);
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_nexthop_group_spec_t *)
                    core::agent_state::state()->nh_group_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            goto end;
        }
        auto proto_spec = proto_req->request(i);
        key.id = proto_spec.id();
        pds_nh_group_proto_to_api_spec(api_spec, proto_spec);
        ret = core::nh_group_create(&key, api_spec, bctxt);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }
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
NhSvcImpl::NhGroupUpdate(ServerContext *context,
                         const pds::NhGroupRequest *proto_req,
                         pds::NhGroupResponse *proto_rsp) {
    sdk_ret_t ret = SDK_RET_OK;
    pds_batch_ctxt_t bctxt;
    pds_nexthop_group_key_t key = { 0 };
    pds_nexthop_group_spec_t *api_spec = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch %u",
                          batch_params.epoch);
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_nexthop_group_spec_t *)
                    core::agent_state::state()->nh_group_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            goto end;
        }
        auto proto_spec = proto_req->request(i);
        key.id = proto_spec.id();
        pds_nh_group_proto_to_api_spec(api_spec, proto_spec);
        ret = core::nh_group_update(&key, api_spec, bctxt);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }
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
NhSvcImpl::NhGroupDelete(ServerContext *context,
                         const pds::NhGroupDeleteRequest *proto_req,
                         pds::NhGroupDeleteResponse *proto_rsp) {
    sdk_ret_t ret = SDK_RET_OK;
    pds_batch_ctxt_t bctxt;
    pds_nexthop_group_key_t key = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch %u",
                          batch_params.epoch);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }


    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::nh_group_delete(&key, bctxt);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }
    return Status::OK;

end:

    if (batched_internally) {
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    // proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::CANCELLED;
}

Status
NhSvcImpl::NhGroupGet(ServerContext *context,
                      const pds::NhGroupGetRequest *proto_req,
                      pds::NhGroupGetResponse *proto_rsp) {
    sdk_ret_t ret = SDK_RET_OK;
    pds_nexthop_group_key_t key = { 0 };
    pds_nexthop_group_info_t info = { 0 };

    PDS_TRACE_VERBOSE("NhGroup Get Received")

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i ++) {
        key.id = proto_req->id(i);
        ret = core::nh_group_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_NOT_FOUND);
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        pds_nh_group_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::nh_group_get_all(pds_nh_group_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}
