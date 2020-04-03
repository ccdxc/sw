// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"
#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

// UpgradeService constructor
UpgradeService::UpgradeService(delphi::SdkPtr sk) : UpgradeService(sk, "UpgradeService") {
}

// UpgradeService constructor
UpgradeService::UpgradeService(delphi::SdkPtr sk, string name) {
    // create sysmgr client
    sysMgr_ = sysmgr::CreateClient(sk, name);

    // save a pointer to sdk
    sdk_ = sk;
    svcName_ = name;

    // mount objects
    delphi::objects::UpgReq::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgResp::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadWriteMode);
    delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgApp::Mount(sdk_, delphi::ReadMode);
    delphi::objects::UpgAppPtr objUpgAppPtr = make_shared<delphi::objects::UpgApp>();
    objUpgAppPtr->set_key(svcName_);
    delphi::objects::UpgApp::MountKey(sdk_, objUpgAppPtr, delphi::ReadWriteMode);

    // create upgrade manager event handler
    upgMgr_ = make_shared<UpgReqReact>(sdk_, sysMgr_);

    upgAppRespHdlr_ = make_shared<UpgAppRespReact>(sdk_, upgMgr_);
    upgAppRegHdlr_ = make_shared<UpgAppRegReact>(upgMgr_, sdk_);

    // Register upgrade request reactor
    delphi::objects::UpgReq::Watch(sdk_, upgMgr_);
    delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespHdlr_);
    delphi::objects::UpgApp::Watch(sdk_, upgAppRegHdlr_);
    sdk_->WatchMountComplete(upgAppRegHdlr_);

    InitStateMachineVector();
    UPG_LOG_DEBUG("Upgrade service constructor got called for {}", name);
}

// OnMountComplete gets called when all the objects are mounted
void UpgradeService::OnMountComplete() {
    sysMgr_->init_done();
    UPG_LOG_DEBUG("UpgradeService OnMountComplete got called\n");

    // walk all upgrade request objects and reconcile them
    auto upgReq = upgMgr_->findUpgReq();
    if (upgReq == NULL) {
        UPG_LOG_DEBUG("No active upgrade request");
        return;
    }
    UPG_LOG_DEBUG("UpgReq found for {}", upgReq->meta().ShortDebugString());
    auto upgStateReq = upgMgr_->findUpgStateReq();
    if (upgStateReq == NULL) {
        UPG_LOG_DEBUG("Reconciling outstanding upgrade request with key: {}", upgReq->GetKey());
        upgMgr_->OnUpgReqCreate(upgReq);
    } else if (upgStateReq->upgreqstate() != UpgStateTerminal) {
        UPG_LOG_DEBUG("Update request in progress. Check if State Machine can be moved.");
        upgMgr_->SetStateMachine(upgReq);
        if (upgMgr_->CanMoveStateMachine(true)) {
            UPG_LOG_DEBUG("Can move state machine. Moving it forward.");
            UpgReqStateType type = upgStateReq->upgreqstate();
            if (!upgMgr_->InvokePrePostStateHandlers(type)) {
                UPG_LOG_DEBUG("PrePostState handlers returned false");
                type = UpgStateFailed;
                upgMgr_->SetAppRespFail();
            } else {
                type = upgMgr_->GetNextState();
            }
            upgMgr_->MoveStateMachine(type);
            return;
        } else {
            UPG_LOG_DEBUG("Cannot move state machine yet");
            return;
        }
    }

    UPG_LOG_DEBUG("============== UpgradeService Finished Reconciliation ==================\n");
}

} // namespace upgrade
