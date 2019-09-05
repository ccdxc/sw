//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/subnet.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/subnet.hpp"
#include "nic/apollo/agent/hooks.hpp"

Status
SubnetSvcImpl::SubnetCreate(ServerContext *context,
                            const pds::SubnetRequest *proto_req,
                            pds::SubnetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_subnet_key_t key = {0};
    pds_subnet_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_subnet_spec_t *)
                    core::agent_state::state()->subnet_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        pds_subnet_proto_to_api_spec(api_spec, request);
        hooks::subnet_create(api_spec);
        ret = core::subnet_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
SubnetSvcImpl::SubnetUpdate(ServerContext *context,
                            const pds::SubnetRequest *proto_req,
                            pds::SubnetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_subnet_key_t key = {0};
    pds_subnet_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_subnet_spec_t *)
                    core::agent_state::state()->subnet_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        pds_subnet_proto_to_api_spec(api_spec, request);
        ret = core::subnet_update(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
SubnetSvcImpl::SubnetDelete(ServerContext *context,
                            const pds::SubnetDeleteRequest *proto_req,
                            pds::SubnetDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_subnet_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::subnet_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

Status
SubnetSvcImpl::SubnetGet(ServerContext *context,
                         const pds::SubnetGetRequest *proto_req,
                         pds::SubnetGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_subnet_key_t key = {0};
    pds_subnet_info_t info = {0};

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    if (proto_req->id_size() == 0) {
        // get all
        ret = core::subnet_get_all(pds_subnet_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::subnet_get(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
        auto response = proto_rsp->add_response();
        pds_subnet_api_spec_to_proto(
                response->mutable_spec(), &info.spec);
        pds_subnet_api_status_to_proto(
                response->mutable_status(), &info.status);
        pds_subnet_api_stats_to_proto(
                response->mutable_stats(), &info.stats);
    }
    return Status::OK;
}
