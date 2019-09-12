//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/device.hpp"
#include "nic/apollo/agent/svc/specs.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define DEVICE_CONF_FILE "/sysconfig/config0/device.conf"

Status
DeviceSvcImpl::DeviceCreate(ServerContext *context,
                            const pds::DeviceRequest *proto_req,
                            pds::DeviceResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_device_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    api_spec = core::agent_state::state()->device();
    if (api_spec == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
        return Status::OK;
    }
    pds_device_proto_to_api_spec(api_spec, proto_req->request());
    if (!core::agent_state::state()->pds_mock_mode()) {
        ret = pds_device_create(api_spec);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}

static inline sdk_ret_t
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

    return SDK_RET_OK;
}

Status
DeviceSvcImpl::DeviceUpdate(ServerContext *context,
                            const pds::DeviceRequest *proto_req,
                            pds::DeviceResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_device_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    api_spec = core::agent_state::state()->device();
    if (api_spec == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
        return Status::OK;
    }
    pds_device_proto_to_api_spec(api_spec, proto_req->request());
    if (!core::agent_state::state()->pds_mock_mode()) {
        ret = pds_device_update(api_spec);
    }

    // update device.conf with profile
    auto profile = proto_req->request().profile();
    ret = device_profile_update(profile);

    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}

Status
DeviceSvcImpl::DeviceDelete(ServerContext *context,
                            const types::Empty *empty,
                            pds::DeviceDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_device_spec_t *api_spec = NULL;

    api_spec = core::agent_state::state()->device();
    if (api_spec == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_NOT_FOUND);
        return Status::OK;
    }
    memset(api_spec, 0, sizeof(pds_device_spec_t));
    if (!core::agent_state::state()->pds_mock_mode()) {
        ret = pds_device_delete();
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}

Status
DeviceSvcImpl::DeviceGet(ServerContext *context,
                         const types::Empty *empty,
                         pds::DeviceGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_device_spec_t *api_spec = NULL;
    pds_device_info_t info;

    api_spec = core::agent_state::state()->device();
    if (api_spec == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_NOT_FOUND);
        return Status::OK;
    }
    memcpy(&info.spec, api_spec, sizeof(pds_device_spec_t));
    if (!core::agent_state::state()->pds_mock_mode()) {
        ret = pds_device_read(&info);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    if (ret != sdk::SDK_RET_OK) {
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
