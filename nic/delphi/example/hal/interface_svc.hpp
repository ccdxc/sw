// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_EXAMPLE_HAL_INTERFACE_SVC_H_
#define _DELPHI_EXAMPLE_HAL_INTERFACE_SVC_H_

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/example/hal/proto/interface.delphi.hpp"

namespace hal {
namespace svc {

using namespace std;
using namespace delphi;

// InterfaceMgr is the reactor for the InterfaceSpec object
class InterfaceMgr : public delphi::objects::InterfaceSpecReactor {
public:
    InterfaceMgr(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }

    // OnInterfaceSpecCreate gets called when InterfaceSpec object is created
    virtual error OnInterfaceSpecCreate(delphi::objects::InterfaceSpecPtr ep);

    // OnInterfaceSpecUpdate gets called when InterfaceSpec object is updated
    virtual error OnInterfaceSpecUpdate(delphi::objects::InterfaceSpecPtr ep);

    // OnInterfaceSpecDelete gets called when InterfaceSpec object is deleted
    virtual error OnInterfaceSpecDelete(delphi::objects::InterfaceSpecPtr ep);

private:
    delphi::SdkPtr sdk_;
};
typedef std::shared_ptr<InterfaceMgr> InterfaceMgrPtr;

} // namespace svc
} // namespace hal

#endif // _DELPHI_EXAMPLE_HAL_INTERFACE_SVC_H_
