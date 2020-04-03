// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_app_reg_reactor.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

delphi::error UpgAppRegReact::OnUpgAppCreate(delphi::objects::UpgAppPtr app) {
    UPG_LOG_DEBUG("UpgAppRegReact::OnUpgAppCreate called for {}", app->key());
    upgMgr_->RegNewApp(app->key());
    return delphi::error::OK();
}

delphi::error UpgAppRegReact::OnUpgAppDelete(delphi::objects::UpgAppPtr app) {
    UPG_LOG_DEBUG("UpgAppRegReact::OnUpgAppDelete called for {}", app->key());
    upgMgr_->UnRegNewApp(app->key());
    return delphi::error::OK();
}

void UpgAppRegReact::OnMountComplete(void) {
    UPG_LOG_DEBUG("UpgAppRegReact OnMountComplete called");

    vector<delphi::objects::UpgAppPtr> upgApplist = delphi::objects::UpgApp::List(sdk_);
    for (vector<delphi::objects::UpgAppPtr>::iterator app=upgApplist.begin(); app!=upgApplist.end(); ++app) {
        OnUpgAppCreate(*app);
    }

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
        } else {
            UPG_LOG_DEBUG("Cannot move state machine yet");
        }
    }
}

} // namespace upgrade
