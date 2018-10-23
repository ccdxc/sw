// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

// delphi reactor APIs for linkmgr

#ifndef __LINKMGR_DELPHI_HPP__
#define __LINKMGR_DELPHI_HPP__

#include <iostream>
#include "grpc++/grpc++.h"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/port.delphi.hpp"

namespace linkmgr {

using grpc::Status;
using delphi::error;
using delphi::objects::PortSpecPtr;
using port::PortOperStatus;

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
    error update_port_status(::google::protobuf::uint32 port_id,
                           PortOperStatus status);

private:
    delphi::SdkPtr sdk_;
};
typedef std::shared_ptr<port_svc> port_svc_ptr_t;

// linkmgr_get_port_reactor gets the port reactor object
port_svc_ptr_t linkmgr_get_port_reactor(void);

// linkmgr_init_port_reactors creates a port reactor
Status linkmgr_init_port_reactors(delphi::SdkPtr sdk);

}    // namespace linkmgr

#endif    // __LINKMGR_DELPHI_HPP__
