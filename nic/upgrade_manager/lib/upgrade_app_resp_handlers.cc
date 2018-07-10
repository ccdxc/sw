// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"
#include "upgrade_mgr.hpp"
#include "upgrade_app_resp_handlers.hpp"
#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"

namespace upgrade {

using namespace std;

// OnUpgAppRespCreate gets called when UpgAppResp object is created
delphi::error UpgAppRespHdlr::OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp) {
    LogInfo("UpgAppResp got created for {}/{}", resp, resp->meta().ShortDebugString());
    if (upgMgr_->IsRespTypeFail(resp->upgapprespval())) {
        string appRespStr = "App " + resp->key() + " returned failure: " + resp->upgapprespstr();
        upgMgr_->AppendAppRespFailStr(appRespStr);
        upgMgr_->SetAppRespFail();
        LogInfo("Adding string {} to list", appRespStr);
    }
    if (upgMgr_->CanMoveStateMachine()) {
        LogInfo("Can move state machine. Invoking post-state handler.");
        auto upgStateReq = upgMgr_->findUpgStateReq(10);
        UpgReqStateType type = upgStateReq->upgreqstate();
        if (!upgMgr_->InvokePrePostStateHandlers(type)) {
            LogInfo("PrePostState handlers returned false");
            type = UpgStateFailed;
            upgMgr_->SetAppRespFail();
        } else {
            type = upgMgr_->GetNextState();
        }
        return upgMgr_->MoveStateMachine(type);
    } else {
        LogInfo("Cannot move state machine yet");
        return delphi::error::OK();
    }

    return delphi::error::OK();
}


delphi::error UpgAppRespHdlr::OnUpgAppRespDelete(delphi::objects::UpgAppRespPtr resp) {
    LogInfo("UpgAppResp got deleted for {}/{}", resp, resp->meta().ShortDebugString());
    vector<delphi::objects::UpgAppRespPtr> upgAppResplist = delphi::objects::UpgAppResp::List(sdk_);
    if (upgAppResplist.empty()) {
        LogInfo("All UpgAppResp objects got deleted");
        upgMgr_->DeleteUpgMgrResp();
    }

    return delphi::error::OK();
}

string UpgAppRespHdlr::UpgStateRespTypeToStr(UpgStateRespType type) {
    return GetAppRespStrUtil(type);
}

delphi::error UpgAppRespHdlr::OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp) {
    if (UpgStateRespTypeToStr(resp->upgapprespval()) != "") 
        LogInfo("\n\n\n========== Got Response {} from {} application ==========", UpgStateRespTypeToStr(resp->upgapprespval()), resp->key());
    //LogInfo("UpgAppRespHdlr OnUpgAppRespVal got called for {}/{}/{}", 
                         //resp, resp->meta().ShortDebugString(), resp->upgapprespval());

    if (upgMgr_->IsRespTypeFail(resp->upgapprespval())) {
        string appRespStr = "App " + resp->key() + " returned failure: " + resp->upgapprespstr();
        upgMgr_->AppendAppRespFailStr(appRespStr);
        upgMgr_->SetAppRespFail();
        LogInfo("Adding string {} to list", appRespStr);
    }

    if (upgMgr_->CanMoveStateMachine()) {
        LogInfo("Can move state machine. Invoking post-state handler.");
        auto upgStateReq = upgMgr_->findUpgStateReq(10);
        UpgReqStateType type = upgStateReq->upgreqstate();
        if (!upgMgr_->InvokePrePostStateHandlers(type)) {
            LogInfo("PrePostState handlers returned false");
            type = UpgStateFailed;
            upgMgr_->SetAppRespFail();
        } else {
            type = upgMgr_->GetNextState();
        }
        return upgMgr_->MoveStateMachine(type);
    } else {
        LogInfo("Cannot move state machine yet");
        return delphi::error::OK();
    }
}

} // namespace upgrade
