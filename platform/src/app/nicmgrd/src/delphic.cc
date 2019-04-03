// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include <string>

#include "nic/sdk/include/sdk/types.hpp"

#include "platform/src/lib/nicmgr/include/dev.hpp"

#include "delphic.hpp"
#include "nicmgrd.hpp"
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "platform/src/lib/devapi_iris/port.hpp"

using namespace std;
using namespace upgrade;
using grpc::Status;
using delphi::error;
using dobj::PortStatus;
using dobj::HalStatus;
using sdk::types::port_oper_status_t;

extern DeviceManager *devmgr;

namespace nicmgr {

// port reactors
port_status_handler_ptr_t g_port_status_handler;
hal_status_handler_ptr_t g_hal_status_handler;
shared_ptr<NicMgrService> g_nicmgr_svc;

// nicmgr delphi service
NicMgrService::NicMgrService(delphi::SdkPtr sk) {
    sdk_ = sk;
    upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<nicmgr_upg_hndlr>(),
                                  "nicmgr", NON_AGENT,
                                  (UpgAgentHandlerPtr)NULL);
    sysmgr_ = nicmgr::create_sysmgr_client(sdk_);
}

// OnMountComplete() gets called after all delphi objects are mounted
void NicMgrService::OnMountComplete() {
    NIC_LOG_DEBUG("On mount complete got called");

    this->sysmgr_->init_done();

    // walk all port status objects and handle them
    vector <delphi::objects::PortStatusPtr> list =
        delphi::objects::PortStatus::List(sdk_);
    for (vector<delphi::objects::PortStatusPtr>::iterator port=list.begin();
         port!=list.end(); ++port) {
        g_port_status_handler->update_port_status(*port);
    }
    devmgr->DelphiMountEventHandler(true);
}

// Name returns the name of the delphi service
std::string NicMgrService::Name() {
    return "nicmgr";
}

// get_port_handler gets the port reactor object
port_status_handler_ptr_t get_port_status_handler (void) {
    return g_port_status_handler;
}

// init_port_handler creates a port reactor
Status init_port_status_handler (delphi::SdkPtr sdk) {
    // create the PortStatus reactor
    g_port_status_handler = std::make_shared<port_status_handler>(sdk);

    // mount objects
    delphi::objects::PortStatus::Mount(sdk, delphi::ReadMode);

    // Register PortStatus reactor
    delphi::objects::PortStatus::Watch(sdk, g_port_status_handler);

    return Status::OK;
}

// OnPortStatusCreate gets called when PortStatus object is created
error port_status_handler::OnPortStatusCreate(PortStatusPtr portStatus) {
    NIC_LOG_DEBUG("Rcvd port status create");
    update_port_status(portStatus);
    return error::OK();
}

// OnPortStatusUpdate gets called when PortStatus object is updated
error port_status_handler::OnPortStatusUpdate(PortStatusPtr portStatus) {
    NIC_LOG_DEBUG("Rcvd port status update");
    update_port_status(portStatus);
    return error::OK();
}

// OnPortStatusDelete gets called when PortStatus object is deleted
error port_status_handler::OnPortStatusDelete(PortStatusPtr portStatus) {
    NIC_LOG_DEBUG("Rcvd port status delete");
    return error::OK();
}

// update_port_status_handler updates port status in delphi
error port_status_handler::update_port_status(PortStatusPtr port) {
    // create port status object
    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();
    }

    // ignore if there is no link/xcvr event
    if (!port->has_link_status() && !port->has_xcvr_status()) {
        return error::OK();
    }

    port_status_t st = {0};

    st.id = port->key_or_handle().port_id();
    if (port->has_link_status()) {
        NIC_FUNC_DEBUG("Received link status: id: {} oper_state: {}, speed: {}",
                       port->key_or_handle().port_id(),
                       port->link_status().oper_state(),
                       port->link_status().port_speed());
        st.status = port->link_status().oper_state();
        // st.speed = HalClient::PortSpeedInMbps(port->port_speed());
        st.speed = devapi_port::port_speed_enum_to_mbps(
                                        port->link_status().port_speed());
        devmgr->LinkEventHandler(&st);
    }
    if (port->has_xcvr_status()) {
        NIC_FUNC_DEBUG("Received xcvr status: id: {} state: {}, "
                       "cable_type: {}, pid: {}",
                       port->key_or_handle().port_id(),
                       port->xcvr_status().state(),
                       port->xcvr_status().cable_type(),
                       port->xcvr_status().pid());
        st.xcvr.state = port->xcvr_status().state();
        st.xcvr.pid = port->xcvr_status().pid();
        st.xcvr.phy = port->xcvr_status().cable_type();
        memcpy(st.xcvr.sprom,
            port->xcvr_status().xcvr_sprom().c_str(),
            MIN(port->xcvr_status().xcvr_sprom().size(),
            sizeof (st.xcvr.sprom)));
        devmgr->XcvrEventHandler(&st);
    }

    NIC_FUNC_DEBUG("id {} status {} speed {} "
        " xcvr.state {} xcvr.phy {} xcvr.pid {}",
        st.id, st.status, st.speed,
        st.xcvr.state, st.xcvr.phy, st.xcvr.pid);

    return error::OK();
}

// get_hal_status_handler gets the port reactor object
hal_status_handler_ptr_t get_hal_status_handler (void) {
    return g_hal_status_handler;
}

// init_hal_status_handler creates a HAL status reactor
Status init_hal_status_handler (delphi::SdkPtr sdk) {
    // create the HalStatus reactor
    g_hal_status_handler = std::make_shared<hal_status_handler>(sdk);

    // mount objects
    HalStatus::Mount(sdk, delphi::ReadMode);

    // register HalStatus reactor
    HalStatus::Watch(sdk, g_hal_status_handler);

    return Status::OK;
}

// OnHalStatusCreate gets called when HalStatus object is created
error hal_status_handler::OnHalStatusCreate(HalStatusPtr halStatus) {
    NIC_LOG_DEBUG("Rcvd OnHalStatusCreate notification for {}", halStatus->ShortDebugString());
    devmgr->HalEventHandler(halStatus->state() == ::hal::HAL_STATE_UP);
    return error::OK();
}

// OnHalStatusUpdate gets called when HalStatus object is updated
error hal_status_handler::OnHalStatusUpdate(HalStatusPtr halStatus) {
    NIC_LOG_DEBUG("Rcvd OnHalStatusUpdate notification for {}", halStatus->ShortDebugString());
    devmgr->HalEventHandler(halStatus->state() == ::hal::HAL_STATE_UP);
    return error::OK();
}

// init_accel_objects mounts accelerator objects
Status init_accel_objects(delphi::SdkPtr sdk) {
    dobj::AccelPfInfo::Mount(sdk, delphi::ReadWriteMode);
    return Status::OK;
}

// initialization function for delphi
void
delphi_init (void)
{
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    g_nicmgr_svc = make_shared<NicMgrService>(sdk);

    // override delphi's logger with our logger
    delphi::SetLogger(std::shared_ptr<spdlog::logger>(utils::logger::logger()));

    // register NicMgr as Delphi Service
    sdk->RegisterService(g_nicmgr_svc);

    // init port status handler
    init_port_status_handler(sdk);

    // init hal status handler
    init_hal_status_handler(sdk);

    // init accel dev related handlers
    init_accel_objects(sdk);

    // connect to delphi
    sdk->Connect();
}

}    // namespace nicmgr
