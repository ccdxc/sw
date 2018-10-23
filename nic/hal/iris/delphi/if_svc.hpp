//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_IRIS_DELPHI_IF_SVC_HPP__
#define __HAL_IRIS_DELPHI_IF_SVC_HPP__

#include <stdio.h>
#include <iostream>
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/interface.delphi.hpp"

namespace dobj = delphi::objects;

namespace hal {
namespace svc {

using delphi::error;

// if_svc handles interface configuration
class if_svc : public dobj::InterfaceSpecReactor {
public:
    if_svc(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }

    // OnPortCreate gets called when InterfaceSpec object is created
    virtual error OnInterfaceSpecCreate(dobj::InterfaceSpecPtr ifSpec);

    // OnPortUpdate gets called when InterfaceSpec object is updated
    virtual error OnInterfaceSpecUpdate(dobj::InterfaceSpecPtr ifSpec);

    // OnPortDelete gets called when InterfaceSpec object is deleted
    virtual error OnInterfaceSpecDelete(dobj::InterfaceSpecPtr ifSpec);

private:
    delphi::SdkPtr    sdk_;
};

typedef std::shared_ptr<if_svc> if_svc_ptr_t;

}    // namespace svc
}    // namespace hal

#endif    // __HAL_IRIS_DELPHI_IF_SVC_HPP__

