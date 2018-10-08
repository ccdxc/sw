// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __DELPHI_CLIENT_HPP__
#define __DELPHI_CLIENT_HPP__

#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/hal/iris/sysmgr/sysmgr.hpp"
#include "bazel-genfiles/nic/upgrade_manager/upgrade/upgrade.delphi.hpp"

namespace hal {
namespace delphi {

using hal::upgrade::upgrade_handler;
using hal::sysmgr_client;

// delphi_client is the HAL’s delphi client obj
class delphi_client : public delphi::Service {
public:
    delphi_client(delphi::SdkPtr sdk) : sysmgr_(sdk, "sysmgr_client"), upghndlr_() {
        sdk_ = sdk;
        upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<upgrade_handler(), name, NON_AGENT);
   }
   void OnMountComplete(void) {}

private:
    delphi::SdkPtr    sdk_;
    sysmgr_client     sysmgr_;
    UpgSdkPtr         upgsdk_;
};

}    // namespace delphi
}    // namespace hal

#endif    // __DELPHI_CLIENT_HPP__
