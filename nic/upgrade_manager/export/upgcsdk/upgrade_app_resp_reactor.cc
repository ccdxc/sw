// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_app_resp_reactor.hpp"
#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

delphi::error UpgAppRespReact::OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp) {
    UPG_LOG_DEBUG("UpgAppRespHdlr::OnUpgAppRespCreate called for {}", resp->key());
    return delphi::error::OK();
}

string UpgAppRespReact::GetAppRespStr(delphi::objects::UpgAppRespPtr resp, UpgType upgType) {
    return GetAppRespStrUtil(resp->upgapprespval(), upgType);
}

void UpgAppRespReact::SetAppRespSuccess(HdlrResp &resp) {
    resp.resp = SUCCESS;
    resp.errStr = "";
}

void UpgAppRespReact::SetAppRespFail(HdlrResp &resp, string str) {
    resp.resp = FAIL;
    resp.errStr = str;
}

void UpgAppRespReact::GetAppResp(delphi::objects::UpgAppRespPtr resp, HdlrResp &hdlrResp, UpgType upgType) {
    switch (resp->upgapprespval()) {
        case UpgStateCompatCheckRespPass:
        case UpgStateProcessQuiesceRespPass:
        case UpgStateLinkDownRespPass:
        case UpgStatePostBinRestartRespPass:
        case UpgStateDataplaneDowntimePhase1RespPass:
        case UpgStateDataplaneDowntimePhase2RespPass:
        case UpgStateDataplaneDowntimePhase3RespPass:
        case UpgStateDataplaneDowntimePhase4RespPass:
        case UpgStateCleanupRespPass:
        case UpgStateAbortRespPass:
        case UpgStateLinkUpRespPass:
        case UpgStateDataplaneDowntimeStartRespPass:
        case UpgStateIsSystemReadyRespPass:
            SetAppRespSuccess(hdlrResp);
            break;
        case UpgStateCompatCheckRespFail:
        case UpgStateProcessQuiesceRespFail:
        case UpgStateLinkDownRespFail:
        case UpgStatePostBinRestartRespFail:
        case UpgStateDataplaneDowntimePhase1RespFail:
        case UpgStateDataplaneDowntimePhase2RespFail:
        case UpgStateDataplaneDowntimePhase3RespFail:
        case UpgStateDataplaneDowntimePhase4RespFail:
        case UpgStateCleanupRespFail:
        case UpgStateAbortRespFail:
        case UpgStateLinkUpRespFail:
        case UpgStateDataplaneDowntimeStartRespFail:
        case UpgStateIsSystemReadyRespFail:
            SetAppRespFail(hdlrResp, GetAppRespStr(resp, upgType));
            break;
        default:
            break;
    }
}

void UpgAppRespReact::InvokeAgentHandler(delphi::objects::UpgAppRespPtr resp, UpgType upgType) {
    HdlrResp hdlrResp;
    GetAppResp(resp, hdlrResp, upgType);
    switch (resp->upgapprespval()) {
        case UpgStateCompatCheckRespPass:
        case UpgStateCompatCheckRespFail:
            upgAgentHandler_->CompatCheckCompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateProcessQuiesceRespPass:
        case UpgStateProcessQuiesceRespFail:
            upgAgentHandler_->ProcessQuiesceCompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStatePostBinRestartRespPass:
        case UpgStatePostBinRestartRespFail:
            upgAgentHandler_->PostRestartCompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateDataplaneDowntimePhase1RespPass:
        case UpgStateDataplaneDowntimePhase1RespFail:
            upgAgentHandler_->DataplaneDowntimePhase1CompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateDataplaneDowntimePhase2RespPass:
        case UpgStateDataplaneDowntimePhase2RespFail:
            upgAgentHandler_->DataplaneDowntimePhase2CompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateDataplaneDowntimePhase3RespPass:
        case UpgStateDataplaneDowntimePhase3RespFail:
            upgAgentHandler_->DataplaneDowntimePhase3CompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateDataplaneDowntimePhase4RespPass:
        case UpgStateDataplaneDowntimePhase4RespFail:
            upgAgentHandler_->DataplaneDowntimePhase4CompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateCleanupRespPass:
        case UpgStateCleanupRespFail:
            upgAgentHandler_->CleanupCompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateAbortRespPass:
        case UpgStateAbortRespFail:
            upgAgentHandler_->AbortedCompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateLinkDownRespPass:
        case UpgStateLinkDownRespFail:
            upgAgentHandler_->LinkDownCompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateLinkUpRespPass:
        case UpgStateLinkUpRespFail:
            upgAgentHandler_->LinkUpCompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateDataplaneDowntimeStartRespPass:
        case UpgStateDataplaneDowntimeStartRespFail:
            upgAgentHandler_->DataplaneDowntimeStartCompletionHandler(hdlrResp, resp->key());
            break;
        case UpgStateIsSystemReadyRespPass:
        case UpgStateIsSystemReadyRespFail:
            upgAgentHandler_->IsSystemReadyCompletionHandler(hdlrResp, resp->key());
            break;
        default:
            break;
    }
    return;
}

extern UpgCtx ctx;
delphi::error UpgAppRespReact::OnUpgAppRespVal(delphi::objects::UpgAppRespPtr
resp) {
    auto upgType = ctx.upgType;
    if (GetAppRespStr(resp, upgType) != "")
        UPG_LOG_DEBUG("UpgAppRespHdlr::OnUpgAppRespVal called for {} with status: {}", resp->key(), GetAppRespStr(resp, upgType));
    InvokeAgentHandler(resp, upgType);
    return delphi::error::OK();
}


} // namespace upgrade
