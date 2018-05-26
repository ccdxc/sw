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

void UpgAppRespReact::SetAppRespSuccess(HdlrResp &resp) {
    resp.resp = SUCCESS;
    resp.errStr = "";
}

void UpgAppRespReact::SetAppRespFail(HdlrResp &resp, string str) {
    resp.resp = FAIL;
    resp.errStr = str;
}

void UpgAppRespReact::InvokeAgentHandler(delphi::objects::UpgAppRespPtr resp) {
    HdlrResp hdlrResp;
    switch (resp->upgapprespval()) {
        case upgrade::PreUpgStatePass:
            this->SetAppRespSuccess(hdlrResp);
            upgAgentHandler_->UpgStatePreUpgCheckComplete(hdlrResp, resp->key());
            break;
        case upgrade::ProcessesQuiescedPass:
            this->SetAppRespSuccess(hdlrResp);
            upgAgentHandler_->UpgStatePreUpgCheckComplete(hdlrResp, resp->key());
            break;
        case upgrade::PostBinRestartPass:
            this->SetAppRespSuccess(hdlrResp);
            upgAgentHandler_->UpgStatePostBinRestartComplete(hdlrResp, resp->key());
            break;
        case upgrade::DataplaneDowntimeStartPass:
            this->SetAppRespSuccess(hdlrResp);
            upgAgentHandler_->UpgStateDataplaceDowntimeComplete(hdlrResp, resp->key());
            break;
        case upgrade::CleanupPass:
            this->SetAppRespSuccess(hdlrResp);
            upgAgentHandler_->UpgStateCleanupComplete(hdlrResp, resp->key());
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
