//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/device.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/platform/fru/fru.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define DEVICE_CONF_FILE "/sysconfig/config0/device.conf"

Status
DeviceSvcImpl::DeviceCreate(ServerContext *context,
                            const pds::DeviceRequest *proto_req,
                            pds::DeviceResponse *proto_rsp) {
    pds_batch_ctxt_t bctxt;
    sdk_ret_t ret = SDK_RET_OK;
    pds_device_spec_t *api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::CANCELLED;
    }
    api_spec = core::agent_state::state()->device();
    if (api_spec == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
        return Status::OK;
    }

    // create an internal batch, if this is not part of an existing API batch
    bctxt = proto_req->batchctxt().batchcookie();
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        batch_params.epoch = core::agent_state::state()->new_epoch();
        batch_params.async = false;
        bctxt = pds_batch_start(&batch_params);
        if (bctxt == PDS_BATCH_CTXT_INVALID) {
            PDS_TRACE_ERR("Failed to create a new batch, device object "
                          "creation failed");
            ret = SDK_RET_ERR;
            goto end;
        }
        batched_internally = true;
    }

    pds_device_proto_to_api_spec(api_spec, proto_req->request());
    if (!core::agent_state::state()->pds_mock_mode()) {
        ret = pds_device_create(api_spec, bctxt);
        if (ret != SDK_RET_OK) {
            if (batched_internally) {
                pds_batch_destroy(bctxt);
            }
            goto end;
        }
        memcpy(core::agent_state::state()->device(), api_spec, sizeof(pds_device_spec_t));
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);

    }

end:

    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}

static inline void
device_profile_update (pds::DeviceProfile profile)
{
    boost::property_tree::ptree pt, output;
    boost::property_tree::ptree::iterator pos;

    if (profile == pds::DEVICE_PROFILE_DEFAULT) {
        output.put("profile", "default");
    } else if (profile == pds::DEVICE_PROFILE_P1) {
        output.put("profile", "p1");
    } else if (profile == pds::DEVICE_PROFILE_P2) {
        output.put("profile", "p2");
    }
    output.put("port-admin-state", "PORT_ADMIN_STATE_ENABLE");

    try {
        // copy existing data from device.conf
        std::ifstream json_cfg(DEVICE_CONF_FILE);
        read_json(json_cfg, pt);

        for (pos = pt.begin(); pos != pt.end();) {
            output.put(pos->first.data(), pos->second.data());
            ++pos;
        }
    } catch (...) {}
    boost::property_tree::write_json(DEVICE_CONF_FILE, output);

    return;
}

Status
DeviceSvcImpl::DeviceUpdate(ServerContext *context,
                            const pds::DeviceRequest *proto_req,
                            pds::DeviceResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_device_spec_t *api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::CANCELLED;
    }
    api_spec = core::agent_state::state()->device();
    if (api_spec == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
        return Status::OK;
    }

    // create an internal batch, if this is not part of an existing API batch
    bctxt = proto_req->batchctxt().batchcookie();
    auto profile = proto_req->request().profile();
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        batch_params.epoch = core::agent_state::state()->new_epoch();
        batch_params.async = false;
        bctxt = pds_batch_start(&batch_params);
        if (bctxt == PDS_BATCH_CTXT_INVALID) {
            PDS_TRACE_ERR("Failed to create a new batch, device object update "
                          "failed");
            ret = SDK_RET_ERR;
            goto end;
        }
        batched_internally = true;
    }

    pds_device_proto_to_api_spec(api_spec, proto_req->request());
    if (!core::agent_state::state()->pds_mock_mode()) {
        ret = pds_device_update(api_spec, bctxt);
        if (ret != SDK_RET_OK) {
            if (batched_internally) {
                pds_batch_destroy(bctxt);
            }
            goto end;
        }
        memcpy(core::agent_state::state()->device(), api_spec, sizeof(pds_device_spec_t));
    }

    // update device.conf with profile
    // TODO: ideally this should be done during commit time (and we need to
    //       handle aborting/rollback here)
    device_profile_update(profile);
    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }

end:

    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}

Status
DeviceSvcImpl::DeviceDelete(ServerContext *context,
                            const pds::DeviceDeleteRequest *proto_req,
                            pds::DeviceDeleteResponse *proto_rsp) {
    pds_batch_ctxt_t bctxt;
    sdk_ret_t ret = SDK_RET_OK;
    pds_device_spec_t *api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    api_spec = core::agent_state::state()->device();

    // create an internal batch, if this is not part of an existing API batch
    bctxt = proto_req->batchctxt().batchcookie();
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        batch_params.epoch = core::agent_state::state()->new_epoch();
        batch_params.async = false;
        bctxt = pds_batch_start(&batch_params);
        if (bctxt == PDS_BATCH_CTXT_INVALID) {
            PDS_TRACE_ERR("Failed to create a new batch, device object delete "
                          "failed");
            ret = SDK_RET_ERR;
            goto end;
        }
        batched_internally = true;
    }

    if (!core::agent_state::state()->pds_mock_mode()) {
        ret = pds_device_delete(bctxt);
        if (ret != SDK_RET_OK) {
            if (batched_internally) {
                pds_batch_destroy(bctxt);
            }
            goto end;
        }
        memset(api_spec, 0, sizeof(pds_device_spec_t));
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }

end:

    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}

static sdk_ret_t
device_status_fill (pds_device_status_t *status)
{
    std::string   mac_str;
    std::string   mem_str;

    // fill fru mac in status
    sdk::platform::readFruKey(MACADDRESS_KEY, mac_str);
    mac_str_to_addr((char *)mac_str.c_str(), status->fru_mac);

    mem_str = api::g_pds_state.catalogue()->memory_capacity_str();
    if (mem_str == "4g") {
        status->memory_cap = 4;
    } else if (mem_str == "8g") {
        status->memory_cap = 8;
    }

    return SDK_RET_OK;
}

Status
DeviceSvcImpl::DeviceGet(ServerContext *context,
                         const types::Empty *empty,
                         pds::DeviceGetResponse *proto_rsp) {
    sdk_ret_t ret = SDK_RET_OK;
    pds_device_spec_t *api_spec = core::agent_state::state()->device();
    pds_device_info_t info;

    if (api_spec->dev_oper_mode != PDS_DEV_OPER_MODE_NONE) {
        api_spec = core::agent_state::state()->device();
        memcpy(&info.spec, api_spec, sizeof(pds_device_spec_t));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_device_read(&info);
        }
    } else {
        memset(&info, 0, sizeof(pds_device_info_t));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = device_status_fill(&info.status);
        }
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Device object not found");
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_NOT_FOUND);
        return Status::OK;
    }
    pds_device_api_spec_to_proto(
            proto_rsp->mutable_response()->mutable_spec(), &info.spec);
    pds_device_api_status_to_proto(
            proto_rsp->mutable_response()->mutable_status(), &info.status);
    pds_device_api_stats_to_proto(
            proto_rsp->mutable_response()->mutable_stats(), &info.stats);
    return Status::OK;
}
