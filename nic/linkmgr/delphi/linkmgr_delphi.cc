//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <iostream>
#include "grpc++/grpc++.h"
#include "nic/linkmgr/delphi/linkmgr_delphi.hpp"
#include "nic/linkmgr/linkmgr.hpp"
#include "nic/linkmgr/linkmgr_svc.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/include/hal.hpp"
#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/linkmgr/linkmgr_utils.hpp"
#include "nic/utils/events/recorder/recorder.hpp"
#include "gen/proto/port.pb.h"
#include "gen/proto/eventtypes.pb.h"
#include "gen/proto/kh.pb.h"

namespace linkmgr {

using grpc::Status;
using delphi::error;
using port::PortResponse;
using port::PortOperState;
using port::PortXcvrState;
using port::PortXcvrPid;
using delphi::objects::PortSpec;
using delphi::objects::PortStatus;
using delphi::objects::PortSpecPtr;
using delphi::objects::PortStatusPtr;

typedef struct linkmgr_svc_s {
    delphi::SdkPtr    sdk;
    port_svc_ptr_t    port_svc;
    events_recorder   *recorder;
} linkmgr_svc_t;
linkmgr_svc_t g_linkmgr_svc;

// port_svc_init creates a port object handler
Status port_svc_init(delphi::SdkPtr sdk) {
    g_linkmgr_svc.sdk = sdk;
    // create the PortSpec handler
    g_linkmgr_svc.port_svc = std::make_shared<port_svc>(sdk);

    // mount objects
    PortSpec::Mount(sdk, delphi::ReadMode);

    // Register PortSpec handler
    PortSpec::Watch(sdk, g_linkmgr_svc.port_svc);

    // mount status objects
    PortStatus::Mount(sdk, delphi::ReadWriteMode);

    HAL_TRACE_DEBUG("Linkmgr: Mounted port objects from delphi...");

    // initialize events recorder
    g_linkmgr_svc.recorder = events_recorder::init("linkmgr",
        std::shared_ptr<logger>(hal::utils::hal_logger()));

    if (g_linkmgr_svc.recorder == nullptr) {
        HAL_TRACE_ERR("events recorder init failed");
        return Status::CANCELLED;
    }

    return Status::OK;
}

// delphi_sdk_get returns pointer to delphi sdk
delphi::SdkPtr
delphi_sdk_get (void)
{
    return g_linkmgr_svc.sdk;
}

// port_svc_get returns pointer to port service object
port_svc_ptr_t
port_svc_get (void)
{
    return g_linkmgr_svc.port_svc;
}

// events_recorder_get returns the pointer to event recorder object
events_recorder*
events_recorder_get(void)
{
    return g_linkmgr_svc.recorder;
}

// OnPortSpecCreate gets called when PortSpec object is created
error port_svc::OnPortSpecCreate(PortSpecPtr portSpec) {
    PortResponse    response;
    port_args_t     port_args  = {0};
    hal_ret_t       hal_ret    = HAL_RET_OK;
    hal_handle_t    hal_handle = 0;

    // validate port params
    if (validate_port_create(*portSpec.get(), &response) == false) {
        HAL_TRACE_ERR("port create validation failed");
        return error::New("port create validation failed");
    }

    // set the params in port_args
    populate_port_create_args(*portSpec.get(), &port_args);

    // create the port
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    hal_ret = linkmgr::port_create(&port_args, &hal_handle);
    if (hal_ret != HAL_RET_OK) {
        HAL_TRACE_ERR("port create failed with error {}", hal_ret);
        return error::New("port create failed");
    }
    hal::hal_cfg_db_close();

    HAL_TRACE_DEBUG("Linkmgr: Port {} got created", port_args.port_num);

    return error::OK();
}

// OnPortSpecUpdate gets called when PortSpec object is updated
error port_svc::OnPortSpecUpdate(PortSpecPtr portSpec) {
    PortResponse    response;
    port_args_t     port_args  = {0};
    hal_ret_t       hal_ret    = HAL_RET_OK;

    // validate port params
    if (validate_port_update(*portSpec.get(), &response) == false) {
        HAL_TRACE_ERR("port update validation failed");
        return error::New("port update validation failed");
    }

    // set the params in port_args
    populate_port_update_args (*portSpec.get(), &port_args);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // create the port
    hal_ret = linkmgr::port_update(&port_args);
    if (hal_ret != HAL_RET_OK) {
        HAL_TRACE_ERR("port update failed with error {}", hal_ret);
        return error::New("port update failed");
    }

    hal::hal_cfg_db_close();

    HAL_TRACE_DEBUG("Linkmgr: Port {} updated", port_args.port_num);

    return error::OK();
}

// OnPortSpecDelete gets called when PortSpec object is deleted
error port_svc::OnPortSpecDelete(PortSpecPtr portSpec) {
    PortResponse    response;
    port_args_t     port_args  = {0};
    hal_ret_t       hal_ret    = HAL_RET_OK;

    // set port number in port_args
    sdk::linkmgr::port_args_init(&port_args);
    port_args.port_num = portSpec->key_or_handle().port_id();

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // create the port
    hal_ret = linkmgr::port_delete(&port_args);
    if (hal_ret != HAL_RET_OK) {
        HAL_TRACE_ERR("port delete failed with error {}", hal_ret);
        return error::New("port delete failed");
    }
    hal::hal_cfg_db_close();

    HAL_TRACE_DEBUG("Linkmgr: Port {} got deleted", port_args.port_num);

    return error::OK();
}

}    // namespace linkmgr
