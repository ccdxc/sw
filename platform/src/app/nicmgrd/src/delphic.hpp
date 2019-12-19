// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __NICMGR_DELPHI_CLIENT_HPP__
#define __NICMGR_DELPHI_CLIENT_HPP__

#include <memory>
#include <string>
#include <iostream>
#include <grpc++/grpc++.h>

#include "gen/proto/nicmgr/nicmgr.delphi.hpp"
#include "gen/proto/nicmgr/accel_metrics.delphi.hpp"
#include "gen/proto/port.delphi.hpp"
#include "gen/proto/hal.delphi.hpp"
#include "gen/proto/device.delphi.hpp"

#include "sysmgr.hpp"
#include "upgrade.hpp"

namespace dobj = delphi::objects;

namespace nicmgr {

using nicmgr::nicmgr_upg_hndlr;
using nicmgr::sysmgr_client;
using grpc::Status;
using delphi::error;
using dobj::PortStatusPtr;
using dobj::HalStatusPtr;
using dobj::SystemSpecPtr;

class NicMgrService : public delphi::Service,
                      public enable_shared_from_this<NicMgrService> {
private:
     std::shared_ptr<nicmgr::sysmgr_client>    sysmgr_;
    UpgSdkPtr                                  upgsdk_;
    delphi::SdkPtr                             sdk_;

public:
    delphi::SdkPtr sdk(void) const { return sdk_; }
    UpgSdkPtr upgsdk(void) const { return upgsdk_; }
    NicMgrService(delphi::SdkPtr sk);
    void OnMountComplete(void);
    virtual std::string Name();
};

// port_status_handler is the reactor for the Port object
class port_status_handler : public dobj::PortStatusReactor {
public:
    port_status_handler(delphi::SdkPtr sdk) {
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
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<port_status_handler> port_status_handler_ptr_t;

// init_port_status_handler creates a port reactor
Status init_port_status_handler(delphi::SdkPtr sdk);


// System Spec Watcher
class system_spec_handler : public dobj::SystemSpecReactor {
public:
    system_spec_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }

    virtual error OnSystemSpecCreate(SystemSpecPtr obj);
    virtual error OnSystemSpecUpdate(SystemSpecPtr obj);
    virtual error OnSystemSpecDelete(SystemSpecPtr obj);

    error update_system_spec(SystemSpecPtr obj);

private:
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<system_spec_handler> system_spec_handler_ptr_t;
Status init_system_spec_handler(delphi::SdkPtr sdk);


class hal_status_handler : public dobj::HalStatusReactor {
public:
    hal_status_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }
    virtual error OnHalStatusCreate(HalStatusPtr port);
    virtual error OnHalStatusUpdate(HalStatusPtr port);
private:
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<hal_status_handler> hal_status_handler_ptr_t;
// init_hal_status_handler creates a reactor for HAL status
Status init_hal_status_handler(delphi::SdkPtr sdk);

// init_accel_objects mounts accelerator objects
Status init_accel_objects(delphi::SdkPtr sdk);

extern shared_ptr<NicMgrService> g_nicmgr_svc;

void delphi_init(void);

void handle_hal_up (void);

}    // namespace nicmgr

#endif
