// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_app_resp_reactor.hpp"
#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"

namespace upgrade {

using namespace std;

delphi::error UpgAppRespReact::OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp) {
    LogInfo("UpgAppRespHdlr::OnUpgAppRespCreate called for {}", resp->key());
    return delphi::error::OK();
}

string UpgAppRespReact::GetAppRespStr(delphi::objects::UpgAppRespPtr resp) {
    return ((resp->upgapprespval() % 2 == 0)?StateMachine[resp->upgapprespval()/2].upgRespStateTypeToStrPass:StateMachine[resp->upgapprespval()/2].upgRespStateTypeToStrFail);
}

void UpgAppRespReact::SetAppRespSuccess(HdlrResp &resp) {
    resp.resp = SUCCESS;
    resp.errStr = "";
}

void UpgAppRespReact::SetAppRespFail(HdlrResp &resp, string str) {
    resp.resp = FAIL;
    resp.errStr = str;
}

void UpgAppRespReact::GetAppResp(delphi::objects::UpgAppRespPtr resp, HdlrResp &hdlrResp) {
    switch (resp->upgapprespval()) {
        case upgrade::PreUpgStatePass:
        case upgrade::ProcessesQuiescedPass:
        case upgrade::PostBinRestartPass:
        case upgrade::DataplaneDowntimePhase1StartPass:
        case upgrade::DataplaneDowntimePhase2StartPass:
        case upgrade::CleanupPass:
            this->SetAppRespSuccess(hdlrResp);
            break;
        case upgrade::PreUpgStateFail:
        case upgrade::ProcessesQuiescedFail:
        case upgrade::PostBinRestartFail:
        case upgrade::DataplaneDowntimePhase1StartFail:
        case upgrade::DataplaneDowntimePhase2StartFail:
        case upgrade::CleanupFail:
            this->SetAppRespFail(hdlrResp, GetAppRespStr(resp));
            break;
        default:
            break;
    }
}

void UpgAppRespReact::InvokeAgentHandler(delphi::objects::UpgAppRespPtr resp) {
    HdlrResp hdlrResp;
    GetAppResp(resp, hdlrResp);
    switch (resp->upgapprespval()) {
        case upgrade::PreUpgStatePass:
        case upgrade::PreUpgStateFail:
            upgAgentHandler_->UpgStatePreUpgCheckComplete(hdlrResp, resp->key());
            break;
        case upgrade::ProcessesQuiescedPass:
        case upgrade::ProcessesQuiescedFail:
            upgAgentHandler_->UpgStatePreUpgCheckComplete(hdlrResp, resp->key());
            break;
        case upgrade::PostBinRestartPass:
        case upgrade::PostBinRestartFail:
            upgAgentHandler_->UpgStatePostBinRestartComplete(hdlrResp, resp->key());
            break;
        case upgrade::DataplaneDowntimePhase1StartPass:
        case upgrade::DataplaneDowntimePhase1StartFail:
            upgAgentHandler_->UpgStateDataplaneDowntimePhase1Complete(hdlrResp, resp->key());
            break;
        case upgrade::DataplaneDowntimePhase2StartPass:
        case upgrade::DataplaneDowntimePhase2StartFail:
            upgAgentHandler_->UpgStateDataplaneDowntimePhase2Complete(hdlrResp, resp->key());
            break;
        case upgrade::CleanupPass:
        case upgrade::CleanupFail:
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
