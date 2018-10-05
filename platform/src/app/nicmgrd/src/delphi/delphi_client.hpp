// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __DELPHI_CLIENT_HPP__
#define __DELPHI_CLIENT_HPP__

#include "platform/src/app/nicmgrd/src/upgrade/upgrade.hpp"
#include "platform/src/app/nicmgrd/src/sysmgr/sysmgr.hpp"
#include "bazel-genfiles/nic/upgrade_manager/upgrade/upgrade.delphi.hpp"

namespace nicmgr {

using nicmgr::upgrade::nicmgr_upg_hndlr;
using nicmgr::sysmgr_client;

//Not reqd anymore ??
#if 0
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
    nicmgr_upg_hndlr     upghndlr_;
};
#endif

class NicMgrService : public delphi::Service, public enable_shared_from_this<NicMgrService> {
private:
    UpgSdkPtr          upgsdk_;
    delphi::SdkPtr     sdk_;
    string             svcName_;
    sysmgr_client     sysmgr_;

public:
    NicMgrService(delphi::SdkPtr sk);
    NicMgrService(delphi::SdkPtr sk, string name);

    virtual string Name() { return svcName_; }
    
    void OnMountComplete(void) {
        this->sysmgr_.init_done();
    }
};

}    // namespace nicmgr

#endif
