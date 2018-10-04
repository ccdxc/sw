// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __DELPHI_CLIENT_HPP__
#define __DELPHI_CLIENT_HPP__

#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/hal/sysmgr/sysmgr.hpp"
#include "bazel-genfiles/nic/upgrade_manager/upgrade/upgrade.delphi.hpp"

namespace hal {

using hal::upgrade::hal_upg_hndlr;
using hal::sysmgr_client;

// delphi_client is the HAL’s delphi client obj
class delphi_client : public delphi::Service {
public:
    delphi_client(delphi::SdkPtr sdk) : sysmgr_(sdk, "sysmgr_client"), upghndlr_() {
        sdk_ = sdk;
        UpgReq::Mount(sdk_, delphi::ReadMode);

        // upg_req_hndlr_ = make_shared<UpgReqReact>(sdk_);
        // UpgReq::Watch(sdk_, upg_req_handler_);

        UpgReq::Watch(sdk_, make_shared<UpgReqReact>(sdk_));
   }
   // void OnMountComplete();

private:
    delphi::SdkPtr    sdk_;
    sysmgr_client     sysmgr_;
    hal_upg_hndlr     upghndlr_;
};

}    // namespace hal

#endif
