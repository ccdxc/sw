// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"
#include "upgrade_mgr.hpp"
#include "upgrade_resp_handlers.hpp"

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

delphi::error UpgAppRespHdlr::OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp) {
    LogInfo("UpgAppRespHdlr OnUpgAppRespVal got called for {}/{}/{}", 
                         resp, resp->meta().ShortDebugString(), resp->upgapprespval());

    if (this->upgMgr_->CanMoveStateMachine()) {
        return (this->upgMgr_->MoveStateMachine(this->upgMgr_->GetNextState()));
    } else {
        LogInfo("Cannot move state machine yet");
        return delphi::error::OK();
    }
}

} // namespace upgrade
