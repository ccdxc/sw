// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "example.hpp"
#include "nic/upgrade_manager/export/upgrade.hpp"

namespace example {

using namespace std;
using namespace upgrade;

// ExUpgSvc constructor
ExUpgSvc::ExUpgSvc(delphi::SdkPtr sk, string name) {
    // save a pointer to sdk
    this->sdk_ = sk;
    this->svcName_ = name;

    // create upgrade request status manager event handler
    upgReqStatusMgr_ = make_shared<UpgReqStatusMgr>(sdk_);

    LogInfo("Example upgrade service constructor got called");
}

#if 0
delphi::error ExUpgSvc::OnUpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("ExUpgSvc OnUpgReqStatusCreate got called.");
    return delphi::error::OK();
}
#endif

// OnMountComplete gets called when all the objects are mounted
void ExUpgSvc::OnMountComplete() {
    string out_str;

    LogInfo("ExUpgSvc OnMountComplete got called\n");

    upgReqStatusMgr_->OnMountComplete();
    upgReqStatusMgr_->Watch(upgReqStatusMgr_);
    LogInfo("============== ExUpgSvc Finished Reconciliation ==================\n");
}

// createTimerHandler creates a dummy code upgrade request
void ExUpgSvc::createTimerHandler(ev::timer &watcher, int revents) {
    LogInfo("Creating the timer");
}
} // namespace example 
