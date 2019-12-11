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

// update_port_status updates port status in delphi
error port_svc::update_port_status(google::protobuf::uint32 port_num,
                                   PortOperState            status,
                                   PortSpeed                speed)
{
    if (sdk_ == NULL) {
        error::OK();
    }
    // create port status object
    PortStatusPtr port = std::make_shared<PortStatus>();

    port->mutable_key_or_handle()->set_port_id(port_num);
    port->mutable_link_status()->set_oper_state(status);
    port->mutable_link_status()->set_port_speed(speed);

    // add it to database
    sdk_->QueueUpdate(port);

    HAL_TRACE_DEBUG("Updated port status object for "
                    "port_id: {} oper_state: {} "
                    "speed: {}, port: {}",
                    port_num, status, speed, port);

    return error::OK();
}

void
port_event_notify (port_event_info_t *port_event_info)
{
    kh::PortKeyHandle port_key_handle;
    const char *port_str = NULL;
    uint32_t logical_port = port_event_info->logical_port;
    port_event_t port_event = port_event_info->event;
    port_speed_t port_speed = port_event_info->speed;
    port_type_t port_type = port_event_info->type;

    if (port_svc_get() == NULL) {
        return;
    }

    port_str =
        sdk::lib::catalog::logical_port_to_str(logical_port, port_type).c_str();

    port_key_handle.set_port_id(logical_port);
    switch (port_event) {
    case port_event_t::PORT_EVENT_LINK_UP:
        HAL_TRACE_DEBUG("port: {}, {}, Link UP",
                        logical_port, port_str);
        port_svc_get()->update_port_status(
                            logical_port,
                            port::PORT_OPER_STATUS_UP,
                            sdk_port_speed_to_port_speed_spec(port_speed));
        events_recorder_get()->event(
                            eventtypes::LINK_UP,
                            "Port: %s, Link UP", port_str);
        break;

    case port_event_t::PORT_EVENT_LINK_DOWN:
        HAL_TRACE_DEBUG("port: {}, {}, Link DOWN",
                        logical_port, port_str);
        port_svc_get()->update_port_status(
                            logical_port,
                            port::PORT_OPER_STATUS_DOWN,
                            sdk_port_speed_to_port_speed_spec(port_speed));
        events_recorder_get()->event(
                            eventtypes::LINK_DOWN,
                            "Port: %s, Link DOWN", port_str);
        break;

    default:
        break;
    }
}

// update_xcvr_status updates xcvr status in delphi
error port_svc::update_xcvr_status(google::protobuf::uint32 port_num,
                                   PortXcvrState state,
                                   PortXcvrPid pid,
                                   CableType cable_type,
                                   void *xcvr_sprom) {
    if (sdk_ == NULL) {
        error::OK();
    }

    // create port status object
    PortStatusPtr port = std::make_shared<PortStatus>();
    port->mutable_key_or_handle()->set_port_id(port_num);

    auto xcvr_status = port->mutable_xcvr_status();

    xcvr_status->set_port(port_num);
    xcvr_status->set_state(state);
    xcvr_status->set_pid(pid);
    xcvr_status->set_cable_type(cable_type);

    port_populate_xcvr_status_sprom(xcvr_status, xcvr_sprom);

    // add it to database
    sdk_->QueueUpdate(port);

    HAL_TRACE_DEBUG("Updated port status object for xcvr "
                    "port_id: {} state: {} pid: {} port: {}",
                    port_num, state, pid, port);

    return error::OK();
}

static CableType
xcvr_sdk_cable_type_to_spec_cable_type (cable_type_t cable_type)
{
    switch (cable_type) {
    case cable_type_t::CABLE_TYPE_CU:
        return port::CABLE_TYPE_COPPER;

    case cable_type_t::CABLE_TYPE_FIBER:
        return port::CABLE_TYPE_FIBER;

    case cable_type_t::CABLE_TYPE_NONE:
    default:
        return port::CABLE_TYPE_NONE;

    }
}

static PortXcvrPid
xcvr_sdk_pid_to_spec_pid (xcvr_pid_t pid)
{
    switch (pid) {
    case xcvr_pid_t::XCVR_PID_QSFP_100G_CR4:
        return port::XCVR_PID_QSFP_100G_CR4;

    case xcvr_pid_t::XCVR_PID_SFP_25GBASE_CR_S:
        return port::XCVR_PID_SFP_25GBASE_CR_S;

    case xcvr_pid_t::XCVR_PID_SFP_25GBASE_CR_L:
        return port::XCVR_PID_SFP_25GBASE_CR_L;

    case xcvr_pid_t::XCVR_PID_SFP_25GBASE_CR_N:
        return port::XCVR_PID_SFP_25GBASE_CR_N;

    case xcvr_pid_t::XCVR_PID_QSFP_100G_AOC:
        return port::XCVR_PID_QSFP_100G_AOC;

    case xcvr_pid_t::XCVR_PID_QSFP_100G_ACC:
        return port::XCVR_PID_QSFP_100G_ACC;

    case xcvr_pid_t::XCVR_PID_QSFP_100G_SR4:
        return port::XCVR_PID_QSFP_100G_SR4;

    case xcvr_pid_t::XCVR_PID_QSFP_100G_LR4:
        return port::XCVR_PID_QSFP_100G_LR4;

    case xcvr_pid_t::XCVR_PID_QSFP_100G_ER4:
        return port::XCVR_PID_QSFP_100G_ER4;

    case xcvr_pid_t::XCVR_PID_QSFP_100G_CWDM4:
        return port::XCVR_PID_QSFP_100G_CWDM4;

    case xcvr_pid_t::XCVR_PID_QSFP_100G_PSM4:
        return port::XCVR_PID_QSFP_100G_PSM4;

    case xcvr_pid_t::XCVR_PID_QSFP_40GBASE_ER4:
        return port::XCVR_PID_QSFP_40GBASE_ER4;

    case xcvr_pid_t::XCVR_PID_QSFP_40GBASE_SR4:
        return port::XCVR_PID_QSFP_40GBASE_SR4;

    case xcvr_pid_t::XCVR_PID_QSFP_40GBASE_LR4:
        return port::XCVR_PID_QSFP_40GBASE_LR4;

    case xcvr_pid_t::XCVR_PID_QSFP_40GBASE_CR4:
        return port::XCVR_PID_QSFP_40GBASE_CR4;

    case xcvr_pid_t::XCVR_PID_QSFP_40GBASE_AOC:
        return port::XCVR_PID_QSFP_40GBASE_AOC;

    case xcvr_pid_t::XCVR_PID_SFP_25GBASE_SR:
        return port::XCVR_PID_SFP_25GBASE_SR;

    case xcvr_pid_t::XCVR_PID_SFP_25GBASE_LR:
        return port::XCVR_PID_SFP_25GBASE_LR;

    case xcvr_pid_t::XCVR_PID_SFP_25GBASE_ER:
        return port::XCVR_PID_SFP_25GBASE_ER;

    case xcvr_pid_t::XCVR_PID_SFP_25GBASE_AOC:
        return port::XCVR_PID_SFP_25GBASE_AOC;

    case xcvr_pid_t::XCVR_PID_SFP_25GBASE_ACC:
        return port::XCVR_PID_SFP_25GBASE_ACC;

    case xcvr_pid_t::XCVR_PID_SFP_10GBASE_SR:
        return port::XCVR_PID_SFP_10GBASE_SR;

    case xcvr_pid_t::XCVR_PID_SFP_10GBASE_LR:
        return port::XCVR_PID_SFP_10GBASE_LR;

    case xcvr_pid_t::XCVR_PID_SFP_10GBASE_LRM:
        return port::XCVR_PID_SFP_10GBASE_LRM;

    case xcvr_pid_t::XCVR_PID_SFP_10GBASE_ER:
        return port::XCVR_PID_SFP_10GBASE_ER;

    case xcvr_pid_t::XCVR_PID_SFP_10GBASE_AOC:
        return port::XCVR_PID_SFP_10GBASE_AOC;

    case xcvr_pid_t::XCVR_PID_SFP_10GBASE_CU:
        return port::XCVR_PID_SFP_10GBASE_CU;

    default:
        return port::XCVR_PID_UNKNOWN;
    }

    return port::XCVR_PID_UNKNOWN;
}

void
xcvr_event_notify (xcvr_event_info_t *xcvr_event_info)
{
    uint32_t     port_num = xcvr_event_info->port_num;
    xcvr_state_t state    = xcvr_event_info->state;
    xcvr_pid_t   pid      = xcvr_event_info->pid;

    switch (state) {
    case xcvr_state_t::XCVR_REMOVED:
        HAL_TRACE_DEBUG("Xcvr removed; port: {}", port_num);

        port_svc_get()->update_xcvr_status(
                            port_num,
                            port::XCVR_STATE_REMOVED,
                            port::XCVR_PID_UNKNOWN,
                            xcvr_sdk_cable_type_to_spec_cable_type(
                                            xcvr_event_info->cable_type),
                            xcvr_event_info->xcvr_sprom);
        break;

    case xcvr_state_t::XCVR_INSERTED:
        HAL_TRACE_DEBUG("Xcvr inserted; port: {}", port_num);

        port_svc_get()->update_xcvr_status(
                            port_num,
                            port::XCVR_STATE_INSERTED,
                            port::XCVR_PID_UNKNOWN,
                            xcvr_sdk_cable_type_to_spec_cable_type(
                                            xcvr_event_info->cable_type),
                            xcvr_event_info->xcvr_sprom);
        break;

    case xcvr_state_t::XCVR_SPROM_READ:
        HAL_TRACE_DEBUG("Xcvr sprom read; port: {}, pid: {}", port_num, pid);

        port_svc_get()->update_xcvr_status(
                            port_num,
                            port::XCVR_STATE_SPROM_READ,
                            xcvr_sdk_pid_to_spec_pid(pid),
                            xcvr_sdk_cable_type_to_spec_cable_type(
                                            xcvr_event_info->cable_type),
                            xcvr_event_info->xcvr_sprom);
        break;

    case xcvr_state_t::XCVR_SPROM_READ_ERR:
        HAL_TRACE_DEBUG("Xcvr sprom read error; port: {}", port_num);

        port_svc_get()->update_xcvr_status(
                            port_num,
                            port::XCVR_STATE_SPROM_READ_ERR,
                            port::XCVR_PID_UNKNOWN,
                            xcvr_sdk_cable_type_to_spec_cable_type(
                                            xcvr_event_info->cable_type),
                            xcvr_event_info->xcvr_sprom);
        break;

    default:
        break;
    }
}

}    // namespace linkmgr
