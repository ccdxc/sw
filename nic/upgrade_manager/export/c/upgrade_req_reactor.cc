// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_req_reactor.hpp"

namespace upgrade {

using namespace std;

void UpgReqReactor::InvokeAppHdlr(UpgReqStateType type, HdlrResp &hdlrResp, UpgCtx &ctx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    switch (type) {
        case UpgReqRcvd:
            LogInfo("Upgrade: Request Received");
            hdlrResp = upgHdlrPtr_->HandleStateUpgReqRcvd(ctx);
            break;
        case PreUpgState:
            LogInfo("Upgrade: Pre-upgrade check");
            hdlrResp = upgHdlrPtr_->HandleStatePreUpgState(ctx);
            break;
        case PostBinRestart:
            LogInfo("Upgrade: Post-binary restart");
            hdlrResp = upgHdlrPtr_->HandleStatePostBinRestart(ctx);
            break;
        case ProcessesQuiesced:
            LogInfo("Upgrade: Processes Quiesced");
            hdlrResp = upgHdlrPtr_->HandleStateProcessesQuiesced(ctx);
            break;
        case DataplaneDowntimePhase1Start:
            LogInfo("Upgrade: Dataplane Downtime Phase1 Start");
            hdlrResp = upgHdlrPtr_->HandleStateDataplaneDowntimePhase1Start(ctx);
            break;
        case DataplaneDowntimeAdminQHandling:
            LogInfo("Upgrade: Dataplane Downtime AdminQ Handling Start");
            hdlrResp = upgHdlrPtr_->HandleDataplaneDowntimeAdminQ(ctx);
            break;
        case DataplaneDowntimePhase2Start:
            LogInfo("Upgrade: Dataplane Downtime Phase2 Start");
            hdlrResp = upgHdlrPtr_->HandleStateDataplaneDowntimePhase2Start(ctx);
            break;
        case Cleanup:
            LogInfo("Upgrade: Cleanup Request Received");
            hdlrResp = upgHdlrPtr_->HandleStateCleanup(ctx);
            break;
        case UpgSuccess:
            LogInfo("Upgrade: Succeeded");
            hdlrResp = upgHdlrPtr_->HandleStateUpgSuccess(ctx);
            break;
        case UpgFailed:
            LogInfo("Upgrade: Failed");
            hdlrResp = upgHdlrPtr_->HandleStateUpgFailed(ctx);
            break;
        case UpgAborted:
            LogInfo("Upgrade: Aborted");
            hdlrResp = resp;
            upgHdlrPtr_->HandleStateUpgAborted(ctx);
            break;
        default:
            LogInfo("Upgrade: Default state");
            break;
    }
}

void UpgReqReactor::GetUpgCtx(UpgCtx &ctx, delphi::objects::UpgStateReqPtr req) {
    ctx.upgType = req->upgreqtype(); 
}

// OnUpgStateReqCreate gets called when UpgStateReq object is created
delphi::error UpgReqReactor::OnUpgStateReqCreate(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgReqReactor UpgStateReq got created for {}/{}/{}", req, req->meta().ShortDebugString(), req->upgreqstate());
    //create the object
    upgAppRespPtr_->CreateUpgAppResp();
    if (upgHdlrPtr_) {
        HdlrResp hdlrResp;
        UpgCtx ctx;
        UpgReqReactor::GetUpgCtx(ctx, req);
        InvokeAppHdlr(req->upgreqstate(), hdlrResp, ctx);
        if (hdlrResp.resp != INPROGRESS) {
            upgAppRespPtr_->UpdateUpgAppResp(upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrResp.resp==SUCCESS)), hdlrResp);
        } else {
            LogInfo("Application still processing");
        }
    }
    return delphi::error::OK();
}

// OnUpgStateReqDelete gets called when UpgStateReq object is deleted
delphi::error UpgReqReactor::OnUpgStateReqDelete(delphi::objects::UpgStateReqPtr req) {
    UpgCtx   ctx;
    LogInfo("UpgReqReactor UpgStateReq got deleted with {}", req->upgreqstate());
    //delete the object
    UpgReqReactor::GetUpgCtx(ctx, req);
    upgAppRespPtr_->DeleteUpgAppResp();
    if (upgHdlrPtr_) {
        upgHdlrPtr_->UpgStateReqDelete(ctx);
    }
    return delphi::error::OK();
}

// OnUpgReqState gets called when UpgReqState attribute changes
delphi::error UpgReqReactor::OnUpgReqState(delphi::objects::UpgStateReqPtr req) {
    HdlrResp hdlrResp;
    UpgCtx ctx;
    if (!upgHdlrPtr_) {
        LogInfo("No handlers available");
        return delphi::error("Error processing OnUpgReqState");
    }
    if (req->upgreqstate() != UpgStateTerminal)
        LogInfo("\n\n\n===== Incoming Message =====");

    UpgReqReactor::GetUpgCtx(ctx, req);
    InvokeAppHdlr(req->upgreqstate(), hdlrResp, ctx);
    if (hdlrResp.resp != INPROGRESS) {
        if (req->upgreqstate() != UpgStateTerminal)
            LogInfo("Application returned {}", (hdlrResp.resp==SUCCESS)?"success":"fail");
        upgAppRespPtr_->UpdateUpgAppResp(upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrResp.resp==SUCCESS)), hdlrResp);
    } else {
        LogInfo("Application still processing"); 
    }
    return delphi::error::OK();
}

} // namespace upgrade
