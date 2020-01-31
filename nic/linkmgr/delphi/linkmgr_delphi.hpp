//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
// delphi handler APIs for linkmgr
//------------------------------------------------------------------------------

#ifndef __LINKMGR_DELPHI_HPP__
#define __LINKMGR_DELPHI_HPP__

#include <iostream>
#include "grpc++/grpc++.h"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/port.delphi.hpp"
#include "nic/sdk/include/sdk/types.hpp"

namespace dobj = delphi::objects;

namespace linkmgr {

using grpc::Status;
using delphi::error;
using delphi::objects::PortSpecPtr;
using port::PortSpeed;
using port::PortOperState;
using port::PortXcvrState;
using port::PortXcvrPid;
using port::CableType;
using sdk::types::port_event_t;
using sdk::types::xcvr_state_t;
using sdk::types::xcvr_pid_t;

// port_svc is the handler for the Port object
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

private:
    delphi::SdkPtr sdk_;
};
typedef std::shared_ptr<port_svc> port_svc_ptr_t;

// initialize port service
Status port_svc_init(delphi::SdkPtr sdk);

// get port service object
port_svc_ptr_t port_svc_get(void);
delphi::SdkPtr delphi_sdk_get(void);

}    // namespace linkmgr

#endif    // __LINKMGR_DELPHI_HPP__
