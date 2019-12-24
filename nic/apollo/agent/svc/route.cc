//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/route.hpp"
#include "nic/apollo/agent/svc/route.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/hooks.hpp"

Status
RouteSvcImpl::RouteTableCreate(ServerContext *context,
                               const pds::RouteTableRequest *proto_req,
                               pds::RouteTableResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_route_table_spec_t *api_spec;
    pds_route_table_key_t key = { 0 };

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
            PDS_TRACE_ERR("Failed to create a new batch, route table "
                          "creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_route_table_spec_t *)
                    core::agent_state::state()->route_table_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_route_table_proto_to_api_spec(api_spec, request);
        if (unlikely(ret != SDK_RET_OK)) {
            goto end;
        }
        hooks::route_table_create(api_spec);
        ret = core::route_table_create(&key, api_spec, bctxt);
        // free the routes memory
        if (api_spec->routes != NULL) {
            SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, api_spec->routes);
            api_spec->routes = NULL;
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
RouteSvcImpl::RouteTableUpdate(ServerContext *context,
                               const pds::RouteTableRequest *proto_req,
                               pds::RouteTableResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_route_table_key_t key = { 0 };
    pds_route_table_spec_t *api_spec;

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
            PDS_TRACE_ERR("Failed to create a new batch, route table "
                          "update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_route_table_spec_t *)
                    core::agent_state::state()->route_table_slab()->alloc();
        if (api_spec == NULL) {
            ret = SDK_RET_OOM;
            goto end;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_route_table_proto_to_api_spec(api_spec, request);
        if (unlikely(ret != SDK_RET_OK)) {
            goto end;
        }
        ret = core::route_table_update(&key, api_spec, bctxt);
        // free the routes memory
        if (api_spec->routes != NULL) {
            SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, api_spec->routes);
            api_spec->routes = NULL;
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
RouteSvcImpl::RouteTableDelete(ServerContext *context,
                               const pds::RouteTableDeleteRequest *proto_req,
                               pds::RouteTableDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_route_table_key_t key = { 0 };

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
            PDS_TRACE_ERR("Failed to create a new batch, route table "
                          "delete failed");
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::route_table_delete(&key, bctxt);
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
RouteSvcImpl::RouteTableGet(ServerContext *context,
                            const pds::RouteTableGetRequest *proto_req,
                            pds::RouteTableGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_route_table_key_t key;
    pds_route_table_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::route_table_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        pds_route_table_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::route_table_get_all(pds_route_table_api_info_to_proto,
                                        proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

