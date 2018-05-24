// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_mgr_agent_resp_reactor.hpp"

namespace upgrade {

using namespace std;

string UpgMgrAgentRespReact::GetRespStr(delphi::objects::UpgRespPtr resp) {
    switch (resp->upgrespval()) {
        case upgrade::UpgPass:
            return ("Upgrade Successful");
        case upgrade::UpgFail:
            return ("Upgrade Failed");
        default:
            return ("");
    }
}

void UpgMgrAgentRespReact::InvokeAgentHandler(delphi::objects::UpgRespPtr resp) {
    switch (resp->upgrespval()) {
        case upgrade::UpgPass:
            upgAgentHandler_->UpgSuccessful();
            break;
        case upgrade::UpgFail:
            upgAgentHandler_->UpgFailed();
            break;
        default:
            break;
    }
}

delphi::error UpgMgrAgentRespReact::OnUpgRespCreate(delphi::objects::UpgRespPtr resp) {
    LogInfo("UpgRespHdlr::OnUpgRespCreate called with status {}", this->GetRespStr(resp));
    InvokeAgentHandler(resp);
    return delphi::error::OK();
}

delphi::error UpgMgrAgentRespReact::OnUpgRespVal(delphi::objects::UpgRespPtr
resp) {
    if (this->GetRespStr(resp) != "")
        LogInfo("UpgRespHdlr::OnUpgRespVal called with status: {}", this->GetRespStr(resp));
    InvokeAgentHandler(resp);
    return delphi::error::OK();
}

};
