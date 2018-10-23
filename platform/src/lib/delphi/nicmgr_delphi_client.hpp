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
using delphi::objects::PortSpecPtr;
using port::PortOperStatus;
using delphi::objects::PortStatusPtr;

class NicMgrService : public delphi::Service, public enable_shared_from_this<NicMgrService> {
private:
    string                       svcName_;

public:
    shared_ptr<sysmgr::Client>   sysmgr_;
    UpgSdkPtr                    upgsdk_;
    delphi::SdkPtr               sdk_;
    NicMgrService(delphi::SdkPtr sk);
    NicMgrService(delphi::SdkPtr sk, string name);
 
    virtual string Name() { return svcName_; }
    
    void OnMountComplete(void); 
};

// port_svc is the reactor for the Port object
class port_svc : public delphi::objects::PortSpecReactor {
public:
    port_svc(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }

    // OnPortCreate gets called when PortSpec object is created
    virtual error OnPortSpecCreate(PortSpecPtr port);

    // OnPortUpdate gets called when PortSpec object is updated
    virtual error OnPortSpecUpdate(PortSpecPtr port);

    // OnPortDelete gets called when PortSpec object is deleted
    virtual error OnPortSpecDelete(PortSpecPtr port);

    // update_port_status updates port status in delphi
    error update_port_status(PortStatusPtr port);
private:
    delphi::SdkPtr sdk_;
};
typedef std::shared_ptr<port_svc> port_svc_ptr_t;

// linkmgr_get_port_reactor gets the port reactor object
port_svc_ptr_t linkmgr_get_port_reactor(void);

// linkmgr_init_port_reactors creates a port reactor
Status linkmgr_init_port_reactors(delphi::SdkPtr sdk);



shared_ptr<NicMgrService> nicmgr_svc_;

}    // namespace nicmgr

#endif
