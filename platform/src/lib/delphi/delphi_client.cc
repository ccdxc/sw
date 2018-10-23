// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include <string>

#include <nicmgr_delphi_client.hpp>
#include <nicmgr_upgrade.hpp>

using namespace std;
using namespace upgrade;

namespace nicmgr {

NicMgrService::NicMgrService(delphi::SdkPtr sk, string name) {
    
    sdk_ = sk;
    svcName_ = name;
    upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<nicmgr_upg_hndlr>(), name, NON_AGENT, (UpgAgentHandlerPtr)NULL);

    sysmgr_ = make_shared<sysmgr::Client>(sdk_, name);
}

void NicMgrService::OnMountComplete() {
    LogInfo("On mount complete got called");
    this->sysmgr_->init_done();
    // Delphi Object Iterator here
    //
    //vector <delphi::objects::EthDeviceInfoPtr> list = delphi::objects::EthDeviceInfo::List(sdk_);
    //for (vector<delphi::objects::EthDeviceInfoPtr>::iterator info=list.begin(); info!=list.end(); ++info) {  
    //}
}

}
