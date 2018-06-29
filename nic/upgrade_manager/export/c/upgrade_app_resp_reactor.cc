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
    return GetAppRespStrUtil(resp->upgapprespval());
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
        case UpgStateCompatCheckRespPass:
        case UpgStateProcessQuiesceRespPass:
        case UpgStatePostBinRestartRespPass:
        case UpgStateDataplaneDowntimePhase1RespPass:
        case UpgStateDataplaneDowntimePhase2RespPass:
        case UpgStateDataplaneDowntimePhase3RespPass:
        case UpgStateDataplaneDowntimePhase4RespPass:
        case UpgStateCleanupRespPass:
        case UpgStateAbortRespPass:
            SetAppRespSuccess(hdlrResp);
            break;
        case UpgStateCompatCheckRespFail:
        case UpgStateProcessQuiesceRespFail:
        case UpgStatePostBinRestartRespFail:
        case UpgStateDataplaneDowntimePhase1RespFail:
        case UpgStateDataplaneDowntimePhase2RespFail:
        case UpgStateDataplaneDowntimePhase3RespFail:
        case UpgStateDataplaneDowntimePhase4RespFail:
        case UpgStateCleanupRespFail:
        case UpgStateAbortRespFail:
            SetAppRespFail(hdlrResp, GetAppRespStr(resp));
            break;
        default:
            break;
    }
}

void UpgAppRespReact::InvokeAgentHandler(delphi::objects::UpgAppRespPtr resp) {
    HdlrResp hdlrResp;
    GetAppResp(resp, hdlrResp);
    switch (resp->upgapprespval()) {
        case UpgStateCompatCheckRespPass:
        case UpgStateCompatCheckRespFail:
            upgAgentHandler_->UpgStatePreUpgCheckComplete(hdlrResp, resp->key());
            break;
        case UpgStateProcessQuiesceRespPass:
        case UpgStateProcessQuiesceRespFail:
            upgAgentHandler_->UpgStatePreUpgCheckComplete(hdlrResp, resp->key());
            break;
        case UpgStatePostBinRestartRespPass:
        case UpgStatePostBinRestartRespFail:
            upgAgentHandler_->UpgStatePostBinRestartComplete(hdlrResp, resp->key());
            break;
        case UpgStateDataplaneDowntimePhase1RespPass:
        case UpgStateDataplaneDowntimePhase1RespFail:
            upgAgentHandler_->UpgStateDataplaneDowntimePhase1Complete(hdlrResp, resp->key());
            break;
        case UpgStateDataplaneDowntimePhase2RespPass:
        case UpgStateDataplaneDowntimePhase2RespFail:
            upgAgentHandler_->UpgStateDataplaneDowntimePhase2Complete(hdlrResp, resp->key());
            break;
        case UpgStateDataplaneDowntimePhase3RespPass:
        case UpgStateDataplaneDowntimePhase3RespFail:
            upgAgentHandler_->UpgStateDataplaneDowntimePhase3Complete(hdlrResp, resp->key());
            break;
        case UpgStateDataplaneDowntimePhase4RespPass:
        case UpgStateDataplaneDowntimePhase4RespFail:
            upgAgentHandler_->UpgStateDataplaneDowntimePhase4Complete(hdlrResp, resp->key());
            break;
        case UpgStateCleanupRespPass:
        case UpgStateCleanupRespFail:
            upgAgentHandler_->UpgStateCleanupComplete(hdlrResp, resp->key());
            break;
        case UpgStateAbortRespPass:
        case UpgStateAbortRespFail:
            upgAgentHandler_->UpgStateAbortedComplete(hdlrResp, resp->key());
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
