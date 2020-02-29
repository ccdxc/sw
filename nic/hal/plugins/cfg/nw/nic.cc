//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/include/hal_cfg.hpp"
#include "gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/plugins/cfg/nw/nic.hpp"
#include "nic/sdk/include/sdk/eth.hpp"

namespace hal {

static sdk::lib::dev_forwarding_mode_t
device_mode_to_hal_fwd_mode (DeviceMode dev_mode)
{
    switch (dev_mode) {
    case device::DEVICE_MODE_NONE:
        return sdk::lib::FORWARDING_MODE_NONE;
    case device::DEVICE_MODE_MANAGED_SWITCH:
        return sdk::lib::FORWARDING_MODE_SWITCH;
    case device::DEVICE_MODE_MANAGED_HOST_PIN:
        return sdk::lib::FORWARDING_MODE_HOSTPIN;
    case device::DEVICE_MODE_STANDALONE:
        return sdk::lib::FORWARDING_MODE_CLASSIC;
    default:
        break;
    }

    return sdk::lib::FORWARDING_MODE_NONE;
}

static DeviceMode
hal_fwd_mode_to_device_mode (sdk::lib::dev_forwarding_mode_t hal_mode)
{
    switch (hal_mode) {
    case sdk::lib::FORWARDING_MODE_NONE:
        return device::DEVICE_MODE_NONE;
    case sdk::lib::FORWARDING_MODE_SWITCH:
        return device::DEVICE_MODE_MANAGED_SWITCH;
    case sdk::lib::FORWARDING_MODE_HOSTPIN:
        return device::DEVICE_MODE_MANAGED_HOST_PIN;
    case sdk::lib::FORWARDING_MODE_CLASSIC:
        return device::DEVICE_MODE_STANDALONE;
    default:
        break;
    }

    return device::DEVICE_MODE_NONE;
}

//------------------------------------------------------------------------------
// process a device create request
//------------------------------------------------------------------------------
hal_ret_t
device_create (DeviceRequest *req, DeviceResponseMsg *rsp)
{
    mac_addr_t local_mac_address;
    auto response = rsp->mutable_response();

    HAL_TRACE_DEBUG("Device Create: Mode change {} -> {}, "
                    "allow_dynamic_pinning: {} -> {}",
                    g_hal_state->forwarding_mode(),
                    device_mode_to_hal_fwd_mode(req->device().device_mode()),
                    g_hal_state->allow_dynamic_pinning(),
                    req->device().allow_dynamic_pinning());

    g_hal_state->set_forwarding_mode(
            device_mode_to_hal_fwd_mode(req->device().device_mode()));
    g_hal_state->set_allow_dynamic_pinning(req->device().allow_dynamic_pinning());

    MAC_UINT64_TO_ADDR(local_mac_address, req->device().local_mac_address());
    g_hal_state->set_local_mac_address(local_mac_address);


    response->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a device create request
//------------------------------------------------------------------------------
hal_ret_t
device_update (DeviceRequest *req, DeviceResponseMsg *rsp)
{
    auto response = rsp->mutable_response();

    HAL_TRACE_DEBUG("Device Create: Mode change {} -> {}, "
                    "allow_dynamic_pinning: {} -> {}",
                    g_hal_state->forwarding_mode(),
                    device_mode_to_hal_fwd_mode(req->device().device_mode()),
                    g_hal_state->allow_dynamic_pinning(),
                    req->device().allow_dynamic_pinning());

    g_hal_state->set_forwarding_mode(
            device_mode_to_hal_fwd_mode(req->device().device_mode()));
    g_hal_state->set_allow_dynamic_pinning(req->device().allow_dynamic_pinning());

    response->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

hal_ret_t
device_get (DeviceGetRequest *req, DeviceGetResponseMsg *rsp)
{
    DeviceGetResponse *response = rsp->mutable_response();

    response->mutable_device()->set_device_mode(
            hal_fwd_mode_to_device_mode(g_hal_state->forwarding_mode()));

    response->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

} // namespace hal
