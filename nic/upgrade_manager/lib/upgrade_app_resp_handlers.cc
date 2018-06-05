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
        return (upgMgr_->MoveStateMachine(upgMgr_->GetNextState()));
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

string UpgAppRespHdlr::UpgRespStateTypeToStr(UpgRespStateType type) {
    return ((type%2==0)?StateMachine[type/2].upgRespStateTypeToStrPass:StateMachine[type/2].upgRespStateTypeToStrFail);
}

delphi::error UpgAppRespHdlr::OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp) {
    if (UpgRespStateTypeToStr(resp->upgapprespval()) != "") 
        LogInfo("\n\n\n========== Got Response {} from {} application ==========", UpgRespStateTypeToStr(resp->upgapprespval()), resp->key());
    //LogInfo("UpgAppRespHdlr OnUpgAppRespVal got called for {}/{}/{}", 
                         //resp, resp->meta().ShortDebugString(), resp->upgapprespval());

    if (upgMgr_->IsRespTypeFail(resp->upgapprespval())) {
        string appRespStr = "App " + resp->key() + " returned failure: " + resp->upgapprespstr();
        upgMgr_->AppendAppRespFailStr(appRespStr);
        upgMgr_->SetAppRespFail();
        LogInfo("Adding string {} to list", appRespStr);
    }

    if (upgMgr_->CanMoveStateMachine()) {
        return (upgMgr_->MoveStateMachine(upgMgr_->GetNextState()));
    } else {
        LogInfo("Cannot move state machine yet");
        return delphi::error::OK();
    }
}

} // namespace upgrade
