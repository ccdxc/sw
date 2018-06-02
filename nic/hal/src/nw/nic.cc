//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/nw/nic.hpp"

namespace hal {

static hal_forwarding_mode_t
device_mode_to_hal_fwd_mode (DeviceMode dev_mode)
{
    switch (dev_mode) {
    case device::DEVICE_MODE_NONE:
        return HAL_FORWARDING_MODE_NONE;
    case device::DEVICE_MODE_MANAGED_SWITCH:
        return HAL_FORWARDING_MODE_SMART_SWITCH;
    case device::DEVICE_MODE_MANAGED_HOST_PIN:
        return HAL_FORWARDING_MODE_SMART_HOST_PINNED;
    case device::DEVICE_MODE_STANDALONE:
        return HAL_FORWARDING_MODE_CLASSIC;
    default:
        break;
    }

    return HAL_FORWARDING_MODE_NONE;
}

static DeviceMode
hal_fwd_mode_to_device_mode (hal_forwarding_mode_t hal_mode)
{
    switch (hal_mode) {
    case HAL_FORWARDING_MODE_NONE:
        return device::DEVICE_MODE_NONE;
    case HAL_FORWARDING_MODE_SMART_SWITCH:
        return device::DEVICE_MODE_MANAGED_SWITCH;
    case HAL_FORWARDING_MODE_SMART_HOST_PINNED:
        return device::DEVICE_MODE_MANAGED_HOST_PIN;
    case HAL_FORWARDING_MODE_CLASSIC:
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
    auto response = rsp->mutable_response();

    g_hal_state->set_forwarding_mode(
            device_mode_to_hal_fwd_mode(req->device().device_mode()));

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

    g_hal_state->set_forwarding_mode(
            device_mode_to_hal_fwd_mode(req->device().device_mode()));

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
