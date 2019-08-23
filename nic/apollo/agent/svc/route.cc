//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/route.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/route.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/hooks.hpp"

Status
RouteSvcImpl::RouteTableCreate(ServerContext *context,
                               const pds::RouteTableRequest *proto_req,
                               pds::RouteTableResponse *proto_rsp) {
    sdk_ret_t ret = sdk::SDK_RET_OK;
    pds_route_table_key_t key = {0};
    pds_route_table_spec_t *api_spec;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_route_table_spec_t *)
                    core::agent_state::state()->route_table_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_agent_route_table_api_spec_fill(api_spec, request);
        if (unlikely(ret != SDK_RET_OK)) {
            return Status::CANCELLED;
        }
        hooks::route_table_create(api_spec);
        ret = core::route_table_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));

        // free the routes memory
        if (api_spec->routes != NULL) {
            SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, api_spec->routes);
            api_spec->routes = NULL;
        }
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
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
        route_table_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::route_table_get_all(route_table_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

Status
RouteSvcImpl::RouteTableUpdate(ServerContext *context,
                               const pds::RouteTableRequest *proto_req,
                               pds::RouteTableResponse *proto_rsp) {
    sdk_ret_t ret = sdk::SDK_RET_OK;
    pds_route_table_key_t key = {0};
    pds_route_table_spec_t *api_spec;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_route_table_spec_t *)
                    core::agent_state::state()->route_table_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_agent_route_table_api_spec_fill(api_spec, request);
        if (unlikely(ret != SDK_RET_OK)) {
            return Status::CANCELLED;
        }
        ret = core::route_table_update(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));

        // free the routes memory
        if (api_spec->routes != NULL) {
            SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, api_spec->routes);
            api_spec->routes = NULL;
        }
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
RouteSvcImpl::RouteTableDelete(ServerContext *context,
                               const pds::RouteTableDeleteRequest *proto_req,
                               pds::RouteTableDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_route_table_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::route_table_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}
