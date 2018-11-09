// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include <string>

#include "nic/sdk/include/sdk/types.hpp"
#include "platform/src/lib/sysmgr/sysmgr.hpp"
#include "platform/src/lib/upgrade/upgrade.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"

#include "delphic.hpp"

using namespace std;
using namespace upgrade;
using grpc::Status;
using delphi::error;
using port::PortResponse;
using port::PortOperStatus;
using delphi::objects::PortStatus;
using delphi::objects::PortStatusPtr;
using sdk::types::port_oper_status_t;

using delphi::objects::AccelHwRingInfo;
using delphi::objects::AccelHwRingInfoPtr;

extern DeviceManager *devmgr;
extern DeviceManager *devices[];

namespace nicmgr {

// port reactors
port_svc_ptr_t g_port_rctr;
shared_ptr<NicMgrService> g_nicmgr_svc;

// accelerator reactors
accel_hw_ring_svc_ptr_t g_accel_hw_ring_rctr;

// NicMgr delphi service
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
        g_port_rctr->update_port_status(*port);
    }

    // walk all accelerator objects and handle them
    vector <delphi::objects::AccelHwRingInfoPtr> hw_ring_list =
        delphi::objects::AccelHwRingInfo::List(sdk_);
    for (vector<delphi::objects::AccelHwRingInfoPtr>::iterator ring=hw_ring_list.begin();
         ring!=hw_ring_list.end(); ++ring) {  
        g_accel_hw_ring_rctr->OnAccelHwRingInfoCreate(*ring);
    }
}

// get_port_reactor gets the port reactor object
port_svc_ptr_t get_port_reactor (void) {
    return g_port_rctr;
}

// init_port_reactors creates a port reactor
Status init_port_reactors (delphi::SdkPtr sdk) {
    // create the PortStatus reactor
    g_port_rctr = std::make_shared<port_svc>(sdk);

    // mount objects
    PortStatus::Mount(sdk, delphi::ReadMode);

    // Register PortStatus reactor
    PortStatus::Watch(sdk, g_port_rctr);

    return Status::OK;
}

// OnPortStatusCreate gets called when PortStatus object is created
error port_svc::OnPortStatusCreate(PortStatusPtr portStatus) {
    printf("Received port status create\n");
    update_port_status(portStatus);
    return error::OK();
}

// OnPortStatusUpdate gets called when PortStatus object is updated
error port_svc::OnPortStatusUpdate(PortStatusPtr portStatus) {
    printf("Received port status update\n");
    update_port_status(portStatus);
    return error::OK();
}

// OnPortStatusDelete gets called when PortStatus object is deleted
error port_svc::OnPortStatusDelete(PortStatusPtr portStatus) {
    printf("Received port status delete\n");
    return error::OK();
}

// update_port_status updates port status in delphi
error port_svc::update_port_status(PortStatusPtr port) {
    // create port status object
    uint32_t port_id = port->mutable_key_or_handle()->port_id();
    port::PortOperStatus oper_status = port->oper_status();

    printf("Delphi setting port %u status to %u\n", port_id, oper_status);

    if (!devmgr) {
        printf("devmgr ptr is null\n");
        return error::OK();
    }
    if (oper_status == port::PortOperStatus::PORT_OPER_STATUS_UP) {
        devmgr->DevLinkUpHandler(port_id);
    } else if (oper_status == port::PortOperStatus::PORT_OPER_STATUS_DOWN) {
        devmgr->DevLinkDownHandler(port_id);
    }

    return error::OK();
}

// get_accel_hw_ring_reactor gets the accelerator HW ring reactor object
accel_hw_ring_svc_ptr_t get_accel_hw_ring_reactor (void) {
    return g_accel_hw_ring_rctr;
}

// init_accel_hw_ring_reactors creates an accelerator HW ring reactor
Status init_accel_hw_ring_reactors (delphi::SdkPtr sdk) {
    // create the PortStatus reactor
    g_accel_hw_ring_rctr = std::make_shared<accel_hw_ring_svc>(sdk);

    // mount objects
    AccelHwRingInfo::Mount(sdk, delphi::ReadMode);

    // Register PortStatus reactor
    AccelHwRingInfo::Watch(sdk, g_accel_hw_ring_rctr);

    return Status::OK;
}

error accel_hw_ring_svc::OnAccelHwRingInfoCreate(AccelHwRingInfoPtr ring) {
    printf("Received %s\n", __FUNCTION__);
    return error::OK();
}

error accel_hw_ring_svc::OnAccelHwRingInfoUpdate(AccelHwRingInfoPtr ring) {
    printf("Received %s\n", __FUNCTION__);
    return error::OK();
}

error accel_hw_ring_svc::OnAccelHwRingInfoDelete(AccelHwRingInfoPtr ring) {
    printf("Received %s\n", __FUNCTION__);
    return error::OK();
}

}    // namespace nicmgr
