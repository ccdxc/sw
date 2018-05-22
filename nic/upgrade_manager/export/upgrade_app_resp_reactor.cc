// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_app_resp_reactor.hpp"

namespace upgrade {

using namespace std;

delphi::error UpgAppRespReact::OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp) {
    LogInfo("UpgAppRespHdlr::OnUpgAppRespCreate called for {}", resp->key());
    return delphi::error::OK();
}

string UpgAppRespReact::GetAppRespStr(delphi::objects::UpgAppRespPtr resp) {
    switch (resp->upgapprespval()) {
        case upgrade::PreUpgStatePass:
            return ("Pre upgrade checks passed");
        case upgrade::ProcessesQuiescedPass:
            return ("Processes Quiesce end");
        case upgrade::PostBinRestartPass:
            return ("Binaries Restarted");
        case upgrade::DataplaneDowntimeStartPass:
            return ("Dataplane downtime end");
        case upgrade::CleanupPass:
            return ("Cleanup finished");
        default:
            return ("");
    }
}

void UpgAppRespReact::InvokeAgentHandler(delphi::objects::UpgAppRespPtr resp) {
    switch (resp->upgapprespval()) {
        case upgrade::PreUpgStatePass:
            upgAgentHandler_->UpgStatePreUpgCheckComplete("pass", resp->key());
            break;
        case upgrade::ProcessesQuiescedPass:
            upgAgentHandler_->UpgStatePreUpgCheckComplete("pass", resp->key());
            break;
        case upgrade::PostBinRestartPass:
            upgAgentHandler_->UpgStatePostBinRestartComplete("pass", resp->key());
            break;
        case upgrade::DataplaneDowntimeStartPass:
            upgAgentHandler_->UpgStateDataplaceDowntimeComplete("pass", resp->key());
            break;
        case upgrade::CleanupPass:
            upgAgentHandler_->UpgStateCleanupComplete("pass", resp->key());
            break;
        default:
            break;
    }
    return;
}

delphi::error UpgAppRespReact::OnUpgAppRespVal(delphi::objects::UpgAppRespPtr
resp) {
    if (GetAppRespStr(resp) != "")
        LogInfo("UpgAppRespHdlr::OnUpgAppRespVal called for {} with status: {}", resp->key(), GetAppRespStr(resp));
    InvokeAgentHandler(resp);
    return delphi::error::OK();
}


} // namespace upgrade
