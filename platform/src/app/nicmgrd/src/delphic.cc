// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include <string>

#include "nic/sdk/include/sdk/types.hpp"
#include "platform/src/lib/sysmgr/sysmgr.hpp"
#include "platform/src/lib/upgrade/upgrade.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"

#include "delphic.hpp"
#include "nicmgrd.hpp"

using namespace std;
using namespace upgrade;
using grpc::Status;
using delphi::error;
using dobj::PortStatus;
using dobj::HalStatus;
using sdk::types::port_oper_status_t;

extern DeviceManager *devmgr;
extern DeviceManager *devices[];
extern bool g_hal_up;

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
    PortStatus::Mount(sdk, delphi::ReadMode);

    // Register PortStatus reactor
    PortStatus::Watch(sdk, g_port_status_handler);

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
    uint32_t port_id = port->mutable_key_or_handle()->port_id();
    port::PortOperStatus oper_status = port->oper_status();
    NIC_LOG_DEBUG("Updating port {} status to {}",
                  port_id, oper_status);

    if (!devmgr) {
        NIC_LOG_ERR("devmgr ptr is null");
        return error::OK();    // TODO: rameshp, pleaes fix this ???
    }
    if (oper_status == port::PortOperStatus::PORT_OPER_STATUS_UP) {
        devmgr->DevLinkUpHandler(port_id);
    } else if (oper_status == port::PortOperStatus::PORT_OPER_STATUS_DOWN) {
        devmgr->DevLinkDownHandler(port_id);
    }

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

// helper function to handle HalStatus update event
void
handle_hal_up (void)
{
    NIC_LOG_DEBUG("Rcvd HAL_STATUS_UP notification");
    devicemanager_init();
    // spawn thread to create mnets
    nicmgrd_mnet_thread_init();
    g_hal_up = true;
}

// OnHalStatusCreate gets called when HalStatus object is created
error hal_status_handler::OnHalStatusCreate(HalStatusPtr halStatus) {
    NIC_LOG_DEBUG("Rcvd OnHalStatusCreate notification for {}", halStatus->ShortDebugString());

    if (halStatus->state() == ::hal::HAL_STATE_UP) {
        handle_hal_up();
    }
    return error::OK();
}

// OnHalStatusUpdate gets called when HalStatus object is updated
error hal_status_handler::OnHalStatusUpdate(HalStatusPtr halStatus) {
    NIC_LOG_DEBUG("Rcvd OnHalStatusUpdate notification for {}", halStatus->ShortDebugString());

    if (halStatus->state() == ::hal::HAL_STATE_UP) {
        handle_hal_up();
    }
    return error::OK();
}

// init_accel_objects mounts accelerator objects
Status init_accel_objects (delphi::SdkPtr sdk) {
    dobj::AccelPfInfo::Mount(sdk, delphi::ReadWriteMode);
    dobj::AccelSeqQueueInfo::Mount(sdk, delphi::ReadWriteMode);
    dobj::AccelHwRingInfo::Mount(sdk, delphi::ReadWriteMode);
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
