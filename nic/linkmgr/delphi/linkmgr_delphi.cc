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
#include "nic/sdk/include/sdk/port_mac.hpp"


namespace linkmgr {

using grpc::Status;
using delphi::error;
using port::PortResponse;
using port::PortOperStatus;
using port::PortXcvrState;
using port::PortXcvrPid;
using delphi::objects::PortSpec;
using delphi::objects::PortStatus;
using delphi::objects::PortSpecPtr;
using delphi::objects::PortStatusPtr;

typedef struct linkmgr_svc_s {
    delphi::SdkPtr    sdk;
    port_svc_ptr_t    port_svc;
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

// update_port_status updates port status in delphi
error port_svc::update_port_status(google::protobuf::uint32 port_id,
                                    PortOperStatus status) {
    // create port status object
    PortStatusPtr port = std::make_shared<PortStatus>();
    port->mutable_key_or_handle()->set_port_id(port_id);
    port->set_oper_status(status);

    // add it to database
    sdk_->QueueUpdate(port);

    HAL_TRACE_DEBUG("Updated port status object for port_id {} oper state {} "
                    "port: {}", port_id, status, port);

    return error::OK();
}

void
port_event_cb (uint32_t port_num, port_event_t event)
{
    switch (event) {
    case port_event_t::PORT_EVENT_LINK_UP:
        HAL_TRACE_DEBUG("port: {}, Link UP", port_num);
        port_svc_get()->update_port_status(port_num, port::PORT_OPER_STATUS_UP);
        break;

    case port_event_t::PORT_EVENT_LINK_DOWN:
        HAL_TRACE_DEBUG("port: {}, Link DOWN", port_num);
        port_svc_get()->update_port_status(port_num, port::PORT_OPER_STATUS_DOWN);
        break;

    default:
        break;
    }
}

// update_xcvr_status updates xcvr status in delphi
error port_svc::update_xcvr_status(google::protobuf::uint32 port_id,
                                   PortXcvrState state,
                                   PortXcvrPid pid) {
    // create port status object
    PortStatusPtr port = std::make_shared<PortStatus>();
    port->mutable_key_or_handle()->set_port_id(port_id);
    port->mutable_xcvr_status()->set_state(state);
    port->mutable_xcvr_status()->set_pid(pid);

    // add it to database
    sdk_->QueueUpdate(port);

    HAL_TRACE_DEBUG("Updated port status object for xcvr port_id {} state {} pid {}"
                    " port: {}", port_id + 1, state, pid, port);

    return error::OK();
}

void
xcvr_event_cb (uint32_t port_num, xcvr_state_t state, xcvr_pid_t pid)
{
    switch (state) {
    case xcvr_state_t::XCVR_REMOVED:
        HAL_TRACE_DEBUG("Xcvr removed; port: {}", port_num + 1);
        port_svc_get()->update_xcvr_status(port_num, port::XCVR_STATE_REMOVED, port::XCVR_PID_UNKNOWN);
        break;

    case xcvr_state_t::XCVR_INSERTED:
        HAL_TRACE_DEBUG("Xcvr inserted; port: {}", port_num + 1);
        port_svc_get()->update_xcvr_status(port_num, port::XCVR_STATE_INSERTED, port::XCVR_PID_UNKNOWN);
        break;

    case xcvr_state_t::XCVR_SPROM_READ:
        HAL_TRACE_DEBUG("Xcvr sprom read; port: {}, pid: {}", port_num + 1,
                        (pid == 0) ? "Unknown" : (pid == 1) ? "SFP-10G-CR" : "QSFP-100G-CR4");
        port_svc_get()->update_xcvr_status(port_num, port::XCVR_STATE_SPROM_READ,
                        (pid == 0) ? port::XCVR_PID_UNKNOWN : (pid == 1) ? port::XCVR_PID_SFP_10G_CR : port::XCVR_PID_QSFP_100G_CR4);
        break;

    case xcvr_state_t::XCVR_SPROM_READ_ERR:
        HAL_TRACE_DEBUG("Xcvr sprom read error; port: {}", port_num + 1);
        port_svc_get()->update_xcvr_status(port_num, port::XCVR_STATE_SPROM_READ_ERR, port::XCVR_PID_UNKNOWN);
        break;

    default:
        break;
    }
}

}    // namespace linkmgr
