// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_cfg.hpp"
#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/svc/nic_svc.hpp"
#include "nic/hal/src/nw/nic.hpp"

using grpc::ServerContext;
using grpc::Status;
using device::Nic;

Status
NicServiceImpl::DeviceCreate(ServerContext *context,
                             const device::DeviceRequestMsg *req,
                             device::DeviceResponseMsg *rsp)
{
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    {
        auto request = ((device::DeviceRequestMsg *)req)->mutable_request();
        hal::device_create(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
NicServiceImpl::DeviceUpdate(ServerContext *context,
                             const device::DeviceRequestMsg *req,
                             device::DeviceResponseMsg *rsp)
{
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    {
        auto request = ((device::DeviceRequestMsg *)req)->mutable_request();
        hal::device_update(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
NicServiceImpl::DeviceGet(ServerContext *context,
                          const device::DeviceGetRequestMsg *req,
                          device::DeviceGetResponseMsg *rsp)
{
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    {
        auto request = ((device::DeviceGetRequestMsg *)req)->mutable_request();
        hal::device_get(request, rsp);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
