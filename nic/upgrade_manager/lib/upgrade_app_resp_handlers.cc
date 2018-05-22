// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"
#include "upgrade_mgr.hpp"
#include "upgrade_app_resp_handlers.hpp"

namespace upgrade {

using namespace std;

// OnUpgAppRespCreate gets called when UpgAppResp object is created
delphi::error UpgAppRespHdlr::OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp) {
    LogInfo("UpgAppResp got created for {}/{}", resp, resp->meta().ShortDebugString());

    if (this->upgMgr_->CanMoveStateMachine()) {
        return (this->upgMgr_->MoveStateMachine(this->upgMgr_->GetNextState()));
    } else {
        LogInfo("Cannot move state machine yet");
        return delphi::error::OK();
    }

    return delphi::error::OK();
}

string UpgAppRespHdlr::UpgRespStateTypeToStr(UpgRespStateType type) {
    switch (type) {
        case UpgReqRcvdPass:
            return ".Upgrade Request Received Pass.";
        case UpgReqRcvdFail:
            return ".Upgrade Request Received Fail.";
        case PreUpgStatePass:
            return "Compat check passed";
        case PreUpgStateFail:
            return "Compat check failed";
        case ProcessesQuiescedPass:
            return "Process Quiesce Pass";
        case ProcessesQuiescedFail:
            return "Process Quiesce Fail";
        case PostBinRestartPass:
            return "Post Process Restart Pass";
        case PostBinRestartFail:
            return "Post Process Restart Fail";
        case DataplaneDowntimeStartPass:
            return "Dataplane Downtime Success";
        case DataplaneDowntimeStartFail:
            return "Dataplane Downtime Fail";
        case CleanupPass:
            return "Cleanup Pass";
        case CleanupFail:
            return "Cleanup Fail";
        default:
            return "";
    }
}

delphi::error UpgAppRespHdlr::OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp) {
    if (UpgRespStateTypeToStr(resp->upgapprespval()) != "") 
        LogInfo("\n\n\n========== Got Response {} from {} application ==========", UpgRespStateTypeToStr(resp->upgapprespval()), resp->key());
    //LogInfo("UpgAppRespHdlr OnUpgAppRespVal got called for {}/{}/{}", 
                         //resp, resp->meta().ShortDebugString(), resp->upgapprespval());

    if (this->upgMgr_->CanMoveStateMachine()) {
        return (this->upgMgr_->MoveStateMachine(this->upgMgr_->GetNextState()));
    } else {
        LogInfo("Cannot move state machine yet");
        return delphi::error::OK();
    }
}

} // namespace upgrade
