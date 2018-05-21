// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "hal_svc.hpp"

namespace hal {
namespace svc {

// HalService constructor
HalService::HalService(delphi::SdkPtr sk) {
    sdk_ = sk;

    // instantiate endpoint manager
    epmgr_ = make_shared<EndpointMgr>(sdk_);

    // interface manager
    ifmgr_ = make_shared<InterfaceMgr>(sdk_);

    // lif manager
    lifmgr_ = make_shared<LifMgr>(sdk_);

    // mount and watch endpoints
    delphi::objects::EndpointSpec::Mount(sdk_, delphi::ReadMode);
    delphi::objects::EndpointSpec::Watch(sdk_, epmgr_);
    delphi::objects::EndpointStatus::Mount(sdk_, delphi::ReadWriteMode);

    // mount and watch interfaces
    delphi::objects::InterfaceSpec::Mount(sdk_, delphi::ReadMode);
    delphi::objects::InterfaceSpec::Watch(sdk_, ifmgr_);
    delphi::objects::InterfaceStatus::Mount(sdk_, delphi::ReadWriteMode);

    // mount and watch Lifs
    delphi::objects::LifSpec::Mount(sdk_, delphi::ReadMode);
    delphi::objects::LifSpec::Watch(sdk_, lifmgr_);
    delphi::objects::LifStatus::Mount(sdk_, delphi::ReadWriteMode);
}

// OnMountComplete on mount complete callback
void HalService::OnMountComplete() {
    LogInfo("On mount complete got called");
}

} // namespace svc
} // namespace hal
