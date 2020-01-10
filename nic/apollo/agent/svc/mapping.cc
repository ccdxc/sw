//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/mapping.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/hooks.hpp"
#include "nic/apollo/api/debug.hpp"

Status
MappingSvcImpl::MappingCreate(ServerContext *context,
                              const pds::MappingRequest *proto_req,
                              pds::MappingResponse *proto_rsp) {
    pds_batch_ctxt_t bctxt;
    sdk_ret_t ret = SDK_RET_OK;
    Status status = Status::OK;
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
            PDS_TRACE_ERR("Failed to create a new batch, mapping creation "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i++) {
        pds_local_mapping_spec_t local_spec;
        pds_remote_mapping_spec_t remote_spec;
        if (proto_req->request(i).tunnelid() == 0) {
            memset(&local_spec, 0, sizeof(local_spec));
            pds_local_mapping_proto_to_api_spec(&local_spec,
                                                proto_req->request(i));
            hooks::local_mapping_create(&local_spec);
        } else {
            memset(&remote_spec, 0, sizeof(remote_spec));
            pds_remote_mapping_proto_to_api_spec(&remote_spec,
                                                 proto_req->request(i));
            hooks::remote_mapping_create(&remote_spec);
        }

        if (!core::agent_state::state()->pds_mock_mode()) {
            if (proto_req->request(i).tunnelid() == 0) {
                ret = pds_local_mapping_create(&local_spec, bctxt);
                if (ret != SDK_RET_OK) {
                    goto end;
                }
            } else {
                ret = pds_remote_mapping_create(&remote_spec, bctxt);
                if (ret != SDK_RET_OK) {
                    goto end;
                }
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
MappingSvcImpl::MappingUpdate(ServerContext *context,
                              const pds::MappingRequest *proto_req,
                              pds::MappingResponse *proto_rsp) {
    pds_batch_ctxt_t bctxt;
    sdk_ret_t ret = SDK_RET_OK;
    Status status = Status::OK;
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
            PDS_TRACE_ERR("Failed to create a new batch, mapping update "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i++) {
        pds_local_mapping_spec_t local_spec;
        pds_remote_mapping_spec_t remote_spec;
        if (proto_req->request(i).tunnelid() == 0) {
            memset(&local_spec, 0, sizeof(local_spec));
            pds_local_mapping_proto_to_api_spec(&local_spec,
                                                proto_req->request(i));
        } else {
            memset(&remote_spec, 0, sizeof(remote_spec));
            pds_remote_mapping_proto_to_api_spec(&remote_spec,
                                                 proto_req->request(i));
        }
        if (!core::agent_state::state()->pds_mock_mode()) {
            if (proto_req->request(i).tunnelid() == 0) {
                ret = pds_local_mapping_update(&local_spec, bctxt);
                if (ret != SDK_RET_OK) {
                    goto end;
                }
            } else {
                ret = pds_remote_mapping_update(&remote_spec, bctxt);
                if (ret != SDK_RET_OK) {
                    goto end;
                }
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
        // commit the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::CANCELLED;
}

Status
MappingSvcImpl::MappingDelete(ServerContext *context,
                              const pds::MappingDeleteRequest *proto_req,
                              pds::MappingDeleteResponse *proto_rsp) {
    pds_batch_ctxt_t bctxt;
    sdk_ret_t ret = SDK_RET_OK;
    Status status = Status::OK;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_mapping_key_t key;

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
            PDS_TRACE_ERR("Failed to create new batch, mapping delete failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return Status::CANCELLED;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        auto proto_key = proto_req->id(i);
        switch (proto_key.keyinfo_case()) {
        case pds::MappingKey::kIPKey:
            key.type = PDS_MAPPING_TYPE_L3;
            pds_obj_key_proto_to_api_spec(&key.vpc, proto_key.ipkey().vpcid());
            ipaddr_proto_spec_to_api_spec(&key.ip_addr,
                                          proto_key.ipkey().ipaddr());
            break;

        case pds::MappingKey::kMACKey:
            key.type = PDS_MAPPING_TYPE_L2;
            key.subnet.id = proto_key.mackey().subnetid();
            MAC_UINT64_TO_ADDR(key.mac_addr, proto_key.mackey().macaddr());
            break;

        default:
            ret = SDK_RET_INVALID_ARG;
            goto end;
            break;
        }
        ret = pds_local_mapping_delete(&key, bctxt);
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
        // commit the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    return Status::CANCELLED;
}
Status
MappingSvcImpl::MappingGet(ServerContext *context,
                           const pds::MappingGetRequest *proto_req,
                           pds::MappingGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_mapping_key_t key;
    pds_local_mapping_info_t local_info;
    pds_remote_mapping_info_t remote_info;

    if (proto_req == NULL || proto_req->id_size() == 0) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
    for (int i = 0; i < proto_req->id_size(); i++) {
        auto proto_key = proto_req->id(i);
        switch (proto_key.keyinfo_case()) {
        case pds::MappingKey::kIPKey:
            key.type = PDS_MAPPING_TYPE_L3;
            pds_obj_key_proto_to_api_spec(&key.vpc, proto_key.ipkey().vpcid());
            ipaddr_proto_spec_to_api_spec(&key.ip_addr,
                                          proto_key.ipkey().ipaddr());
            break;

        case pds::MappingKey::kMACKey:
            key.type = PDS_MAPPING_TYPE_L2;
            key.subnet.id = proto_key.mackey().subnetid();
            MAC_UINT64_TO_ADDR(key.mac_addr, proto_key.mackey().macaddr());
            break;

        default:
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
            return Status::OK;
        }
        ret = pds_local_mapping_read(&key, &local_info);
        if (ret == SDK_RET_OK) {
            pds_local_mapping_api_info_to_proto(&local_info, proto_rsp);
            continue;
        }
        ret = pds_remote_mapping_read(&key, &remote_info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        pds_remote_mapping_api_info_to_proto(&remote_info, proto_rsp);
    }
    return Status::OK;
}
