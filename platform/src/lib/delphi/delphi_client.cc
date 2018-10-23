// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include <string>

#include <nicmgr_delphi_client.hpp>
#include <nicmgr_upgrade.hpp>
#include <dev.hpp>
#include <eth_dev.hpp>
#include <sdk/types.hpp>

using namespace std;
using namespace upgrade;
using grpc::Status;
using delphi::error;
using port::PortResponse;
using port::PortOperStatus;
using delphi::objects::PortSpec;
using delphi::objects::PortStatus;
using delphi::objects::PortSpecPtr;
using delphi::objects::PortStatusPtr;
using sdk::types::port_oper_status_t;

extern DeviceManager *devmgr;
extern DeviceManager *devices[];

namespace nicmgr {

NicMgrService::NicMgrService(delphi::SdkPtr sk, string name) {
    
    sdk_ = sk;
    svcName_ = name;
    upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<nicmgr_upg_hndlr>(), name, NON_AGENT, (UpgAgentHandlerPtr)NULL);

    sysmgr_ = make_shared<sysmgr::Client>(sdk_, name);
}

void NicMgrService::OnMountComplete() {
    LogInfo("On mount complete got called");
    this->sysmgr_->init_done();
    // Delphi Object Iterator here
    //
    //vector <delphi::objects::EthDeviceInfoPtr> list = delphi::objects::EthDeviceInfo::List(sdk_);
    //for (vector<delphi::objects::EthDeviceInfoPtr>::iterator info=list.begin(); info!=list.end(); ++info) {  
    //}
}

// port reactors
port_svc_ptr_t g_port_rctr;

// linkmgr_get_port_reactor gets the port reactor object
port_svc_ptr_t linkmgr_get_port_reactor () {
    return g_port_rctr;
}

// linkmgr_init_port_reactors creates a port reactor
Status linkmgr_init_port_reactors (delphi::SdkPtr sdk) {
    // create the PortSpec reactor
    g_port_rctr = std::make_shared<port_svc>(sdk);

    // mount objects
    PortSpec::Mount(sdk, delphi::ReadMode);

    // Register PortSpec reactor
    PortSpec::Watch(sdk, g_port_rctr);

    // mount status objects
    PortStatus::Mount(sdk, delphi::ReadWriteMode);

    return Status::OK;
}

// OnPortSpecCreate gets called when PortSpec object is created
error port_svc::OnPortSpecCreate(PortSpecPtr portSpec) {
    return error::OK();
}

// OnPortSpecUpdate gets called when PortSpec object is updated
error port_svc::OnPortSpecUpdate(PortSpecPtr portSpec) {
    return error::OK();
}

// OnPortSpecDelete gets called when PortSpec object is deleted
error port_svc::OnPortSpecDelete(PortSpecPtr portSpec) {
    return error::OK();
}

// update_port_status updates port status in delphi
error port_svc::update_port_status(PortStatusPtr port) {
    // create port status object
    uint32_t port_id = port->mutable_key_or_handle()->port_id();
    port::PortOperStatus oper_status = port->oper_status();

    if (oper_status == port::PortOperStatus::PORT_OPER_STATUS_UP) {
        devmgr->DevLinkUpHandler(port_id);
    } else if (oper_status == port::PortOperStatus::PORT_OPER_STATUS_DOWN) {
        devmgr->DevLinkDownHandler(port_id);
    }


    return error::OK();
}


}
