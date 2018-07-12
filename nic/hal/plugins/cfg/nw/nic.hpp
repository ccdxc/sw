//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NIC_HPP__
#define __NIC_HPP__

#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/nic.pb.h"

using device::DeviceMode;
using device::DeviceSpec;
using device::DeviceResponse;
using device::DeviceResponseMsg;
using device::DeviceRequest;
using device::DeviceGetResponse;
using device::DeviceGetResponseMsg;
using device::DeviceGetRequest;

namespace hal {

hal_ret_t device_create(DeviceRequest *req,
                        DeviceResponseMsg *rsp);
hal_ret_t device_update(DeviceRequest *req,
                        DeviceResponseMsg *rsp);
hal_ret_t device_get(DeviceGetRequest *req,
                     DeviceGetResponseMsg *rsp);

} // namespace hal

#endif // __NIC_HPP__
