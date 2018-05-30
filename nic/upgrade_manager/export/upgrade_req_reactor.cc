// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_req_reactor.hpp"

namespace upgrade {

using namespace std;

void UpgReqReactor::InvokeAppHdlr(UpgReqStateType type, HdlrResp &hdlrResp) {
    UpgCtx ctx;
    switch (type) {
        case UpgReqRcvd:
            LogInfo("Upgrade: Request Received");
            hdlrResp = this->upgHdlrPtr_->HandleStateUpgReqRcvd(ctx);
            break;
        case PreUpgState:
            LogInfo("Upgrade: Pre-upgrade check");
            hdlrResp = this->upgHdlrPtr_->HandleStatePreUpgState(ctx);
            break;
        case PostBinRestart:
            LogInfo("Upgrade: Post-binary restart");
            hdlrResp = this->upgHdlrPtr_->HandleStatePostBinRestart(ctx);
            break;
        case ProcessesQuiesced:
            LogInfo("Upgrade: Processes Quiesced");
            hdlrResp = this->upgHdlrPtr_->HandleStateProcessesQuiesced(ctx);
            break;
        case DataplaneDowntimeStart:
            LogInfo("Upgrade: Dataplane Downtime Start");
            hdlrResp = this->upgHdlrPtr_->HandleStateDataplaneDowntimeStart(ctx);
            break;
        case Cleanup:
            LogInfo("Upgrade: Cleanup Request Received");
            hdlrResp = this->upgHdlrPtr_->HandleStateCleanup(ctx);
            break;
        case UpgSuccess:
            LogInfo("Upgrade: Succeeded");
            hdlrResp = this->upgHdlrPtr_->HandleStateUpgSuccess(ctx);
            break;
        case UpgFailed:
            LogInfo("Upgrade: Failed");
            hdlrResp = this->upgHdlrPtr_->HandleStateUpgFailed(ctx);
            break;
        default:
            LogInfo("Upgrade: Default state");
            break;
    }
}

// OnUpgStateReqCreate gets called when UpgStateReq object is created
delphi::error UpgReqReactor::OnUpgStateReqCreate(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgReqReactor UpgStateReq got created for {}/{}/{}", req, req->meta().ShortDebugString(), req->upgreqstate());
    //create the object
    upgAppRespPtr_->CreateUpgAppResp();
    if (this->upgHdlrPtr_) {
        HdlrResp hdlrResp;
        InvokeAppHdlr(req->upgreqstate(), hdlrResp);
        if (hdlrResp.resp != INPROGRESS) {
            this->upgAppRespPtr_->UpdateUpgAppResp(this->upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrResp.resp==SUCCESS)), hdlrResp);
        }
    }
    return delphi::error::OK();
}

// OnUpgStateReqDelete gets called when UpgStateReq object is deleted
delphi::error UpgReqReactor::OnUpgStateReqDelete(delphi::objects::UpgStateReqPtr req) {
    UpgCtx   ctx;
    LogInfo("UpgReqReactor UpgStateReq got deleted");
    //delete the object
    upgAppRespPtr_->DeleteUpgAppResp();
    if (this->upgHdlrPtr_) {
        this->upgHdlrPtr_->UpgStateReqDelete(ctx);
    }
    return delphi::error::OK();
}

// OnUpgReqState gets called when UpgReqState attribute changes
delphi::error UpgReqReactor::OnUpgReqState(delphi::objects::UpgStateReqPtr req) {
    HdlrResp hdlrResp;
    if (!this->upgHdlrPtr_) {
        LogInfo("No handlers available");
        return delphi::error("Error processing OnUpgReqState");
    }
    if (req->upgreqstate() != UpgStateTerminal)
        LogInfo("\n\n\n===== Incoming Message =====");
    InvokeAppHdlr(req->upgreqstate(), hdlrResp);
    if (hdlrResp.resp != INPROGRESS) {
        if (req->upgreqstate() != UpgStateTerminal)
            LogInfo("Application returned {}", (hdlrResp.resp==SUCCESS)?"success":"fail");
        this->upgAppRespPtr_->UpdateUpgAppResp(this->upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrResp.resp==SUCCESS)), hdlrResp);
    } else {
        LogInfo("Application still processing"); 
    }
    return delphi::error::OK();
}

} // namespace upgrade
