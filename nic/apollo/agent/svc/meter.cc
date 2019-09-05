//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/meter.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/meter.hpp"

Status
MeterSvcImpl::MeterCreate(ServerContext *context,
                          const pds::MeterRequest *proto_req,
                          pds::MeterResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_meter_key_t key = {0};
    pds_meter_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_meter_spec_t *)
                    core::agent_state::state()->meter_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_meter_proto_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->meter_slab()->free(api_spec);
            break;
        }
        ret = core::meter_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));

        // free the rules memory
        if (api_spec->rules != NULL) {
            SDK_FREE(PDS_MEM_ALLOC_ID_METER, api_spec->rules);
            api_spec->rules = NULL;
        }
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
MeterSvcImpl::MeterUpdate(ServerContext *context,
                          const pds::MeterRequest *proto_req,
                          pds::MeterResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_meter_key_t key = {0};
    pds_meter_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_meter_spec_t *)
                    core::agent_state::state()->meter_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_meter_proto_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->meter_slab()->free(api_spec);
            break;
        }
        ret = core::meter_update(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));

        // free the rules memory
        if (api_spec->rules != NULL) {
            SDK_FREE(PDS_MEM_ALLOC_ID_METER, api_spec->rules);
            api_spec->rules = NULL;
        }
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
MeterSvcImpl::MeterDelete(ServerContext *context,
                          const pds::MeterDeleteRequest *proto_req,
                          pds::MeterDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_meter_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::meter_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

Status
MeterSvcImpl::MeterGet(ServerContext *context,
                       const pds::MeterGetRequest *proto_req,
                     pds::MeterGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_meter_key_t key = {0};
    pds_meter_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
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
