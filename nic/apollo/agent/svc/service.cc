//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/service.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/service.hpp"

Status
SvcImpl::SvcMappingCreate(ServerContext *context,
                          const pds::SvcMappingRequest *proto_req,
                          pds::SvcMappingResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_svc_mapping_key_t key = {0};
    pds_svc_mapping_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_svc_mapping_spec_t *)
                    core::agent_state::state()->service_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.vpc.id = request.key().vpcid();
        key.svc_port = request.key().svcport();
        ipaddr_proto_spec_to_api_spec(&key.vip, request.key().ipaddr());
        service_proto_spec_to_api_spec(api_spec, request);
        ret = core::service_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
SvcImpl::SvcMappingUpdate(ServerContext *context,
                          const pds::SvcMappingRequest *proto_req,
                          pds::SvcMappingResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_svc_mapping_key_t key = {0};
    pds_svc_mapping_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_svc_mapping_spec_t *)
                    core::agent_state::state()->service_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.vpc.id = request.key().vpcid();
        key.svc_port = request.key().svcport();
        ipaddr_proto_spec_to_api_spec(&key.vip, request.key().ipaddr());
        service_proto_spec_to_api_spec(api_spec, request);
        ret = core::service_update(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
SvcImpl::SvcMappingDelete(ServerContext *context,
                          const pds::SvcMappingDeleteRequest *proto_req,
                          pds::SvcMappingDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_svc_mapping_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->key_size(); i++) {
        key.vpc.id = proto_req->key(i).vpcid();
        key.svc_port = proto_req->key(i).svcport();
        ipaddr_proto_spec_to_api_spec(&key.vip, proto_req->key(i).ipaddr());
        ret = core::service_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

Status
SvcImpl::SvcMappingGet(ServerContext *context,
                       const pds::SvcMappingGetRequest *proto_req,
                       pds::SvcMappingGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_svc_mapping_key_t key = {0};
    pds_svc_mapping_info_t info = {0};

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    if (proto_req->key_size() == 0) {
        // get all
        ret = core::service_get_all(service_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->key_size(); i++) {
        key.vpc.id = proto_req->key(i).vpcid();
        key.svc_port = proto_req->key(i).svcport();
        ipaddr_proto_spec_to_api_spec(&key.vip, proto_req->key(i).ipaddr());
        ret = core::service_get(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
        auto response = proto_rsp->add_response();
        service_api_spec_to_proto_spec(
                response->mutable_spec(), &info.spec);
        service_api_status_to_proto_status(
                response->mutable_status(), &info.status);
        service_api_stats_to_proto_stats(
                response->mutable_stats(), &info.stats);
    }
    return Status::OK;
}
