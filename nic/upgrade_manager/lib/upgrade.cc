// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"
#include "upgrade_mgr.hpp"
#include "upgrade_resp_handlers.hpp"

namespace upgrade {

using namespace std;

// UpgradeService constructor
UpgradeService::UpgradeService(delphi::SdkPtr sk) : UpgradeService(sk, "UpgradeService") {
}

// UpgradeService constructor
UpgradeService::UpgradeService(delphi::SdkPtr sk, string name) {
    // save a pointer to sdk
    this->sdk_ = sk;
    this->svcName_ = name;

    // mount objects
    delphi::objects::UpgReq::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgReqStatus::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadMode);

    // create upgrade manager event handler
    upgMgr_ = make_shared<UpgradeMgr>(sdk_);

    upgAppRespHdlr_ = make_shared<UpgAppRespHdlr>(sdk_, upgMgr_);

    // Register upgrade request reactor
    delphi::objects::UpgReq::Watch(sdk_, upgMgr_);
    delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespHdlr_);

    LogInfo("Upgrade service constructor got called");
}

// OnMountComplete gets called when all the objects are mounted
void UpgradeService::OnMountComplete() {
    LogInfo("UpgradeService OnMountComplete got called\n");

    // walk all upgrade request objects and reconcile them
    vector<delphi::objects::UpgReqPtr> upgReqlist = delphi::objects::UpgReq::List(sdk_);
    for (vector<delphi::objects::UpgReqPtr>::iterator req=upgReqlist.begin(); req!=upgReqlist.end(); ++req) {
        upgMgr_->OnUpgReqCreate(*req);
    }

    LogInfo("============== UpgradeService Finished Reconciliation ==================\n");
}

// createUpgReqSpec creates a dummy upgrade request 
void UpgradeService::createUpgReqSpec() {
    // create an object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    srand(time(NULL));
    req->mutable_key()->set_id(rand());
    req->set_upgreqcmd(upgrade::InvalidCmd);

    // add it to database
    sdk_->SetObject(req);

    req->set_upgreqcmd(upgrade::UpgStart);
    sdk_->SetObject(req);
}

// createTimerHandler creates a dummy code upgrade request
void UpgradeService::createTimerHandler(ev::timer &watcher, int revents) {
    LogInfo("Creating a dummy upgrade request");

    // create a dummy upgrade request
    this->createUpgReqSpec();
}
} // namespace upgrade
