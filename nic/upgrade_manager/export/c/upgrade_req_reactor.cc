// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_req_reactor.hpp"

namespace upgrade {

using namespace std;

void UpgReqReactor::InvokeAppHdlr(UpgReqStateType type, HdlrResp &hdlrResp, UpgCtx &ctx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    switch (type) {
        case UpgStateCompatCheck:
            LogInfo("Upgrade: Pre-upgrade check");
            hdlrResp = upgHdlrPtr_->HandleStatePreUpgState(ctx);
            break;
        case UpgStatePostBinRestart:
            LogInfo("Upgrade: Post-binary restart");
            hdlrResp = upgHdlrPtr_->HandleStatePostBinRestart(ctx);
            break;
        case UpgStateProcessQuiesce:
            LogInfo("Upgrade: Processes Quiesced");
            hdlrResp = upgHdlrPtr_->HandleStateProcessesQuiesced(ctx);
            break;
        case UpgStateDataplaneDowntimePhase1:
            LogInfo("Upgrade: Dataplane Downtime Phase1 Start");
            hdlrResp = upgHdlrPtr_->HandleStateDataplaneDowntimePhase1Start(ctx);
            break;
        case UpgStateDataplaneDowntimePhase2:
            LogInfo("Upgrade: Dataplane Downtime Phase2 Start");
            hdlrResp = upgHdlrPtr_->HandleStateDataplaneDowntimePhase2Start(ctx);
            break;
        case UpgStateDataplaneDowntimePhase3:
            LogInfo("Upgrade: Dataplane Downtime Phase3 Start");
            hdlrResp = upgHdlrPtr_->HandleStateDataplaneDowntimePhase3Start(ctx);
            break;
        case UpgStateDataplaneDowntimePhase4:
            LogInfo("Upgrade: Dataplane Downtime Phase4 Start");
            hdlrResp = upgHdlrPtr_->HandleStateDataplaneDowntimePhase4Start(ctx);
            break;
        case UpgStateCleanup:
            LogInfo("Upgrade: Cleanup Request Received");
            hdlrResp = upgHdlrPtr_->HandleStateCleanup(ctx);
            break;
        case UpgStateSuccess:
            LogInfo("Upgrade: Succeeded");
            hdlrResp = resp;
            upgHdlrPtr_->HandleStateUpgSuccess(ctx);
            break;
        case UpgStateFailed:
            LogInfo("Upgrade: Failed");
            hdlrResp = resp;
            upgHdlrPtr_->HandleStateUpgFailed(ctx);
            break;
        case UpgStateAbort:
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
    if (upgHdlrPtr_ && upgAppRespPtr_->CanInvokeHandler(req->upgreqstate())) {
        upgAppRespPtr_->CreateUpgAppResp();
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
    LogInfo("UpgReqReactor UpgStateReq got deleted with {}", req->upgreqstate());
    //delete the object
    upgAppRespPtr_->DeleteUpgAppResp();
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

delphi::objects::UpgAppPtr UpgReqReactor::FindUpgAppPtr(void) {
    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key(svcName_);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(app);

    return static_pointer_cast<delphi::objects::UpgApp>(obj);
}

delphi::objects::UpgAppPtr UpgReqReactor::CreateUpgAppObj(void) {
    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key(svcName_);

    // add it to database
    sdk_->SetObject(app);
    return app;
}

void UpgReqReactor::RegisterUpgApp() {
    LogInfo("UpgReqReactor::RegisterUpgApp");
    delphi::objects::UpgAppPtr app = FindUpgAppPtr();
    if (app == NULL) {
        LogInfo("Creating UpgApp");
        app = CreateUpgAppObj();
    }
    LogInfo("Returning after creating UpgApp");
}

void UpgReqReactor::OnMountComplete(void) {
    RegisterUpgApp();
    LogInfo("UpgReqReactor OnMountComplete called");

    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
    req->set_key(10);

    //find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);

    delphi::objects::UpgStateReqPtr reqStatus = static_pointer_cast<delphi::objects::UpgStateReq>(obj);
    if (reqStatus == NULL) {
        LogInfo("No UpgStateReq object found");
        return;
    }
    OnUpgStateReqCreate(reqStatus);
}

} // namespace upgrade
