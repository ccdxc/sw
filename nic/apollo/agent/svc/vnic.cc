//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/vnic.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/vnic.hpp"
#include "nic/apollo/agent/svc/specs.hpp"

Status
VnicSvcImpl::VnicCreate(ServerContext *context,
                        const pds::VnicRequest *proto_req,
                        pds::VnicResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vnic_key_t key = {0};
    pds_vnic_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_vnic_spec_t *)
                    core::agent_state::state()->vnic_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.vnicid();
        ret = pds_vnic_proto_spec_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->vnic_slab()->free(api_spec);
            break;
        }
        ret = core::vnic_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
VnicSvcImpl::VnicUpdate(ServerContext *context,
                        const pds::VnicRequest *proto_req,
                        pds::VnicResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vnic_key_t key = {0};
    pds_vnic_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_vnic_spec_t *)
                    core::agent_state::state()->vnic_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.vnicid();
        ret = pds_vnic_proto_spec_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->vnic_slab()->free(api_spec);
            break;
        }
        ret = core::vnic_update(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
VnicSvcImpl::VnicDelete(ServerContext *context,
                        const pds::VnicDeleteRequest *proto_req,
                        pds::VnicDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vnic_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->vnicid_size(); i++) {
        key.id = proto_req->vnicid(i);
        ret = core::vnic_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

Status
VnicSvcImpl::VnicGet(ServerContext *context,
                     const pds::VnicGetRequest *proto_req,
                     pds::VnicGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vnic_key_t key = {0};
    pds_vnic_info_t info = {0};

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->vnicid_size(); i++) {
        key.id = proto_req->vnicid(i);
        ret = core::vnic_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        vnic_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->vnicid_size() == 0) {
        ret = core::vnic_get_all(vnic_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}
