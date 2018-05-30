// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"
#include "upgrade_mgr.hpp"
#include "upgrade_app_resp_handlers.hpp"
#include "nic/upgrade_manager/include/upgrade_state_machine.hpp"

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
    delphi::objects::UpgReq::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgResp::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadMode);

    // create upgrade manager event handler
    upgMgr_ = make_shared<UpgradeMgr>(sdk_);

    upgAppRespHdlr_ = make_shared<UpgAppRespHdlr>(sdk_, upgMgr_);

    // Register upgrade request reactor
    delphi::objects::UpgReq::Watch(sdk_, upgMgr_);
    delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespHdlr_);

    InitStateMachineVector();
    LogInfo("Upgrade service constructor got called");
}

// OnMountComplete gets called when all the objects are mounted
void UpgradeService::OnMountComplete() {
    LogInfo("UpgradeService OnMountComplete got called\n");

    // walk all upgrade request objects and reconcile them
    vector<delphi::objects::UpgReqPtr> upgReqlist = delphi::objects::UpgReq::List(sdk_);
    for (vector<delphi::objects::UpgReqPtr>::iterator req=upgReqlist.begin(); req!=upgReqlist.end(); ++req) {
        LogInfo("UpgReq found for {}/{}/{}", (*req), (*req)->key(), (*req)->meta().ShortDebugString());
        auto upgStateReq = upgMgr_->findUpgStateReq(10);//(*req)->key());
        if (upgStateReq == NULL) {
            LogInfo("Reconciling outstanding upgrade request with key: {}", (*req)->key());
            upgMgr_->OnUpgReqCreate(*req);
        } else {
            LogInfo("Update request in progress. Check if State Machine can be moved.");
            if (upgMgr_->CanMoveStateMachine()) {
                LogInfo("Can move state machine. Moving it forward.");
                upgMgr_->MoveStateMachine(upgMgr_->GetNextState());
                return;
            } else {
                LogInfo("Cannot move state machine yet");
                return;
            }
        }
    }

    LogInfo("============== UpgradeService Finished Reconciliation ==================\n");
}

} // namespace upgrade
