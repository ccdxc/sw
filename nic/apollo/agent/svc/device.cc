//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/device.hpp"
#include "nic/apollo/agent/svc/device_svc.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/device.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_device.hpp"
#include "nic/sdk/platform/fru/fru.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define DEVICE_CONF_FILE "/sysconfig/config0/device.conf"

sdk_ret_t
pds_svc_device_create (const pds::DeviceRequest *proto_req,
                       pds::DeviceResponse *proto_rsp)
{
    pds_batch_ctxt_t bctxt;
    sdk_ret_t ret = SDK_RET_OK;
    pds_device_spec_t *api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }
    api_spec = core::agent_state::state()->device();
    if (api_spec == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
        return SDK_RET_OOM;
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
    pds_ms::device_create(api_spec);
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
    return ret;
}

static inline void
device_profile_update (pds::DeviceProfile profile)
{
    boost::property_tree::ptree pt, output;
    boost::property_tree::ptree::iterator pos;

    try {
        // copy existing data from device.conf
        std::ifstream json_cfg(DEVICE_CONF_FILE);
        read_json(json_cfg, pt);

        for (pos = pt.begin(); pos != pt.end();) {
            output.put(pos->first.data(), pos->second.data());
            ++pos;
        }
    } catch (...) {}

    if (profile == pds::DEVICE_PROFILE_DEFAULT) {
        output.put("device-profile", "default");
    } else if (profile == pds::DEVICE_PROFILE_2PF) {
        output.put("device-profile", "2pf");
    } else if (profile == pds::DEVICE_PROFILE_3PF) {
        output.put("device-profile", "3pf");
    } else if (profile == pds::DEVICE_PROFILE_4PF) {
        output.put("device-profile", "4pf");
    } else if (profile == pds::DEVICE_PROFILE_5PF) {
        output.put("device-profile", "5pf");
    } else if (profile == pds::DEVICE_PROFILE_6PF) {
        output.put("device-profile", "6pf");
    } else if (profile == pds::DEVICE_PROFILE_7PF) {
        output.put("device-profile", "7pf");
    } else if (profile == pds::DEVICE_PROFILE_8PF) {
        output.put("device-profile", "8pf");
    }

    boost::property_tree::write_json(DEVICE_CONF_FILE, output);

    return;
}

static inline void
memory_profile_update (pds::MemoryProfile profile)
{
    boost::property_tree::ptree pt, output;
    boost::property_tree::ptree::iterator pos;

    try {
        // copy existing data from device.conf
        std::ifstream json_cfg(DEVICE_CONF_FILE);
        read_json(json_cfg, pt);

        for (pos = pt.begin(); pos != pt.end();) {
            output.put(pos->first.data(), pos->second.data());
            ++pos;
        }
    } catch (...) {}

    if (profile == pds::MEMORY_PROFILE_DEFAULT) {
        output.put("memory-profile", "default");
    }
    output.put("port-admin-state", "PORT_ADMIN_STATE_ENABLE");

    boost::property_tree::write_json(DEVICE_CONF_FILE, output);

    return;
}

sdk_ret_t
pds_svc_device_update (const pds::DeviceRequest *proto_req,
                       pds::DeviceResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_device_spec_t api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    // create an internal batch, if this is not part of an existing API batch
    bctxt = proto_req->batchctxt().batchcookie();
    auto memory_profile = proto_req->request().memoryprofile();
    auto device_profile = proto_req->request().deviceprofile();
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

    pds_device_proto_to_api_spec(&api_spec, proto_req->request());
    if (!core::agent_state::state()->pds_mock_mode()) {
        ret = pds_device_update(&api_spec, bctxt);
        if (ret != SDK_RET_OK) {
            if (batched_internally) {
                pds_batch_destroy(bctxt);
            }
            goto end;
        }
        memcpy(core::agent_state::state()->device(), &api_spec, sizeof(pds_device_spec_t));
    }

    // update device.conf with memory-profile
    // TODO: ideally this should be done during commit time (and we need to
    //       handle aborting/rollback here)
    memory_profile_update(memory_profile);

    // update device.conf with device-profile
    device_profile_update(device_profile);

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }

end:

    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return ret;
}

sdk_ret_t
pds_svc_device_delete (const pds::DeviceDeleteRequest *proto_req,
                       pds::DeviceDeleteResponse *proto_rsp)
{
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
    return ret;
}

static inline void
device_spec_fill (pds_device_spec_t *spec)
{
    spec->device_profile = api::g_pds_state.device_profile();
    spec->memory_profile = api::g_pds_state.memory_profile();
}

static inline sdk_ret_t
device_info_fill (pds_device_info_t *info)
{
    device_spec_fill(&info->spec);
    api::device_entry::fill_status(&info->status);
    return SDK_RET_OK;
}

sdk_ret_t
pds_svc_device_get (const pds::DeviceGetRequest *proto_req,
                    pds::DeviceGetResponse *proto_rsp)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_device_spec_t *api_spec = core::agent_state::state()->device();
    pds_device_info_t info;

    if (api_spec->dev_oper_mode != PDS_DEV_OPER_MODE_NONE) {
        memcpy(&info.spec, api_spec, sizeof(pds_device_spec_t));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_device_read(&info);
        }
    } else {
        memset(&info, 0, sizeof(pds_device_info_t));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = device_info_fill(&info);
        }
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Device object not found");
        return ret;
    }
    pds_device_api_spec_to_proto(
            proto_rsp->mutable_response()->mutable_spec(), &info.spec);
    pds_device_api_status_to_proto(
            proto_rsp->mutable_response()->mutable_status(), &info.status);
    pds_device_api_stats_to_proto(
            proto_rsp->mutable_response()->mutable_stats(), &info.stats);
    return ret;
}

Status
DeviceSvcImpl::DeviceCreate(ServerContext *context,
                            const pds::DeviceRequest *proto_req,
                            pds::DeviceResponse *proto_rsp) {
    pds_svc_device_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
DeviceSvcImpl::DeviceUpdate(ServerContext *context,
                            const pds::DeviceRequest *proto_req,
                            pds::DeviceResponse *proto_rsp) {
    pds_svc_device_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
DeviceSvcImpl::DeviceDelete(ServerContext *context,
                            const pds::DeviceDeleteRequest *proto_req,
                            pds::DeviceDeleteResponse *proto_rsp) {
    pds_svc_device_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
DeviceSvcImpl::DeviceGet(ServerContext *context,
                         const pds::DeviceGetRequest *proto_req,
                         pds::DeviceGetResponse *proto_rsp) {
    pds_svc_device_get(proto_req, proto_rsp);
    return Status::OK;
}
