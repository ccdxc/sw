// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __NICMGR_DELPHI_CLIENT_HPP__
#define __NICMGR_DELPHI_CLIENT_HPP__

#include <memory>
#include <string>
#include <iostream>

#include "grpc++/grpc++.h"
#include "nicmgr_upgrade.hpp"
#include "nicmgr_sysmgr.hpp"
#include "nicmgr.delphi.hpp"
#include "port.delphi.hpp"

namespace nicmgr {

using nicmgr::nicmgr_upg_hndlr;
using nicmgr::sysmgr_client;
using grpc::Status;
using delphi::error;
using port::PortOperStatus;
using delphi::objects::PortStatusPtr;

class NicMgrService : public delphi::Service,
                      public enable_shared_from_this<NicMgrService> {
private:
    shared_ptr<sysmgr::Client>   sysmgr_;
    UpgSdkPtr                    upgsdk_;
    delphi::SdkPtr               sdk_;

public:
    delphi::SdkPtr sdk(void) const { return sdk_; }
    NicMgrService(delphi::SdkPtr sk);
    void OnMountComplete(void); 
};

// port_svc is the reactor for the Port object
class port_svc : public delphi::objects::PortStatusReactor {
public:
    port_svc(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }

    // OnPortCreate gets called when PortStatus object is created
    virtual error OnPortStatusCreate(PortStatusPtr port);

    // OnPortUpdate gets called when PortStatus object is updated
    virtual error OnPortStatusUpdate(PortStatusPtr port);

    // OnPortDelete gets called when PortStatus object is deleted
    virtual error OnPortStatusDelete(PortStatusPtr port);

    // update_port_status updates port status in delphi
    error update_port_status(PortStatusPtr port);
private:
    delphi::SdkPtr sdk_;
};
typedef std::shared_ptr<port_svc> port_svc_ptr_t;

// init_port_reactors creates a port reactor
Status init_port_reactors(delphi::SdkPtr sdk);
shared_ptr<NicMgrService> nicmgr_svc_;

}    // namespace nicmgr

#endif
