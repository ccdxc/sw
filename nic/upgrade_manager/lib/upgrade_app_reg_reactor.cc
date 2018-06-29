// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_app_reg_reactor.hpp"

namespace upgrade {

using namespace std;

delphi::error UpgAppRegReact::OnUpgAppCreate(delphi::objects::UpgAppPtr app) {
    LogInfo("UpgAppRegReact::OnUpgAppCreate called for {}", app->key());
    upgMgr_->RegNewApp(app->key());
    return delphi::error::OK();
}

void UpgAppRegReact::OnMountComplete(void) {
    LogInfo("UpgAppRegReact OnMountComplete called");

    vector<delphi::objects::UpgAppPtr> upgApplist = delphi::objects::UpgApp::List(sdk_);
    for (vector<delphi::objects::UpgAppPtr>::iterator app=upgApplist.begin(); app!=upgApplist.end(); ++app) {
        OnUpgAppCreate(*app);
    }
}

} // namespace upgrade
