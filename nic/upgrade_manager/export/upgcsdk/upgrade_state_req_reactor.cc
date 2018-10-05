// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_state_req_reactor.hpp"
#include "nic/upgrade_manager/include/c/upgrade_metadata.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

UpgCtx ctx;

void UpgStateReqReact::InvokeAppHdlr(UpgReqStateType type, HdlrResp &hdlrResp) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    switch (type) {
        case UpgStateUpgPossible:
        case UpgStateCompatCheck:
            UPG_LOG_DEBUG("Upgrade: Pre-upgrade check");
            hdlrResp = upgHdlrPtr_->CompatCheckHandler(ctx);
            break;
        case UpgStatePostBinRestart:
            UPG_LOG_DEBUG("Upgrade: Post-binary restart");
            hdlrResp = upgHdlrPtr_->PostRestartHandler(ctx);
            break;
        case UpgStateLinkUp:
            UPG_LOG_DEBUG("Upgrade: Link Up");
            hdlrResp = upgHdlrPtr_->LinkUpHandler(ctx);
            break;
        case UpgStateDataplaneDowntimeStart:
            UPG_LOG_DEBUG("Upgrade: Dataplane Downtime Start");
            hdlrResp = upgHdlrPtr_->DataplaneDowntimeStartHandler(ctx);
            break;
        case UpgStateIsSystemReady:
            UPG_LOG_DEBUG("Upgrade: Is System Ready");
            hdlrResp = upgHdlrPtr_->IsSystemReadyHandler(ctx);
            break;
        case UpgStateProcessQuiesce:
            UPG_LOG_DEBUG("Upgrade: Processes Quiesced");
            hdlrResp = upgHdlrPtr_->ProcessQuiesceHandler(ctx);
            break;
        case UpgStateLinkDown:
            UPG_LOG_DEBUG("Upgrade: Link Down");
            hdlrResp = upgHdlrPtr_->LinkDownHandler(ctx);
            break;
        case UpgStateDataplaneDowntimePhase1:
            UPG_LOG_DEBUG("Upgrade: Dataplane Downtime Phase1 ");
            hdlrResp = upgHdlrPtr_->DataplaneDowntimePhase1Handler(ctx);
            break;
        case UpgStateDataplaneDowntimePhase2:
            UPG_LOG_DEBUG("Upgrade: Dataplane Downtime Phase2 ");
            hdlrResp = upgHdlrPtr_->DataplaneDowntimePhase2Handler(ctx);
            break;
        case UpgStateDataplaneDowntimePhase3:
            UPG_LOG_DEBUG("Upgrade: Dataplane Downtime Phase3 ");
            hdlrResp = upgHdlrPtr_->DataplaneDowntimePhase3Handler(ctx);
            break;
        case UpgStateDataplaneDowntimePhase4:
            UPG_LOG_DEBUG("Upgrade: Dataplane Downtime Phase4 ");
            hdlrResp = upgHdlrPtr_->DataplaneDowntimePhase4Handler(ctx);
            break;
        case UpgStateCleanup:
            UPG_LOG_DEBUG("Upgrade: Cleanup Request Received");
            hdlrResp = upgHdlrPtr_->CleanupHandler(ctx);
            break;
        case UpgStateSuccess:
            UPG_LOG_DEBUG("Upgrade: Succeeded");
            hdlrResp = resp;
            upgHdlrPtr_->SuccessHandler(ctx);
            break;
        case UpgStateFailed:
            UPG_LOG_DEBUG("Upgrade: Failed");
            hdlrResp = resp;
            upgHdlrPtr_->FailedHandler(ctx);
            break;
        case UpgStateAbort:
            UPG_LOG_DEBUG("Upgrade: Abort");
            hdlrResp = resp;
            upgHdlrPtr_->AbortHandler(ctx);
            break;
        default:
            UPG_LOG_DEBUG("Upgrade: Default state {}", type);
            break;
    }
}

bool UpgStateReqReact::GetUpgCtx(delphi::objects::UpgStateReqPtr req) {
    ctx.upgType = req->upgreqtype(); 
    return GetUpgCtxFromMeta(ctx);
}

// OnUpgStateReqCreate gets called when UpgStateReq object is created
delphi::error UpgStateReqReact::OnUpgStateReqCreate(delphi::objects::UpgStateReqPtr req) {
    UPG_LOG_DEBUG("UpgStateReqReact UpgStateReq got created for {}/{}/{}", req, req->meta().ShortDebugString(), req->upgreqstate());
    //create the object
    if (!GetUpgCtx(req)) {
        return delphi::error("GetUpgCtxFromMeta failed");
    }
    UPG_LOG_DEBUG("OnUpgStateReqCreate upgType {}", ctx.upgType);
    if (upgHdlrPtr_ && upgAppRespPtr_->CanInvokeHandler(req->upgreqstate(), ctx.upgType)) {
        upgAppRespPtr_->CreateUpgAppResp();
        HdlrResp hdlrResp;
        InvokeAppHdlr(req->upgreqstate(), hdlrResp);
        if (hdlrResp.resp != INPROGRESS) {
            upgAppRespPtr_->UpdateUpgAppResp(upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrResp.resp==SUCCESS), ctx.upgType), hdlrResp, ctx.upgType);
        } else {
            UPG_LOG_DEBUG("Application still processing");
        }
    }
    return delphi::error::OK();
}

// OnUpgStateReqDelete gets called when UpgStateReq object is deleted
delphi::error UpgStateReqReact::OnUpgStateReqDelete(delphi::objects::UpgStateReqPtr req) {
    UPG_LOG_DEBUG("UpgStateReqReact UpgStateReq got deleted with {}", req->upgreqstate());
    //delete the object
    upgAppRespPtr_->DeleteUpgAppResp();
    return delphi::error::OK();
}

// OnUpgReqState gets called when UpgReqState attribute changes
delphi::error UpgStateReqReact::OnUpgReqState(delphi::objects::UpgStateReqPtr req) {
    UPG_LOG_DEBUG("UpgStateReqReact UpgStateReq got modified with {}", req->upgreqstate());
    HdlrResp hdlrResp;
    if (!GetUpgCtx(req)) {
        return delphi::error("GetUpgCtxFromMeta failed");
    }
    UPG_LOG_DEBUG("OnUpgReqState upgType {}", ctx.upgType);
    if (!upgHdlrPtr_) {
        UPG_LOG_ERROR("No handlers available");
        return delphi::error("Error processing OnUpgReqState");
    }
    if (req->upgreqstate() != UpgStateTerminal) {
        UPG_LOG_DEBUG("\n\n\n===== Incoming Message =====");

        InvokeAppHdlr(req->upgreqstate(), hdlrResp);
        if (hdlrResp.resp != INPROGRESS) {
            UPG_LOG_DEBUG("Application returned {}", (hdlrResp.resp==SUCCESS)?"success":"fail");
            upgAppRespPtr_->UpdateUpgAppResp(upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrResp.resp==SUCCESS), ctx.upgType), hdlrResp, ctx.upgType);
        } else {
            UPG_LOG_DEBUG("Application still processing"); 
        }
    }
    return delphi::error::OK();
}

delphi::objects::UpgAppPtr UpgStateReqReact::FindUpgAppPtr(void) {
    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key(svcName_);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(app);

    return static_pointer_cast<delphi::objects::UpgApp>(obj);
}

delphi::objects::UpgAppPtr UpgStateReqReact::CreateUpgAppObj(void) {
    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key(svcName_);

    // add it to database
    sdk_->SetObject(app);
    return app;
}

void UpgStateReqReact::RegisterUpgApp() {
    UPG_LOG_DEBUG("UpgStateReqReact::RegisterUpgApp");
    delphi::objects::UpgAppPtr app = FindUpgAppPtr();
    if (app == NULL) {
        UPG_LOG_DEBUG("Creating UpgApp");
        app = CreateUpgAppObj();
    }
    UPG_LOG_DEBUG("Returning after creating UpgApp");
}

void UpgStateReqReact::OnMountComplete(void) {
    RegisterUpgApp();
    UPG_LOG_DEBUG("UpgStateReqReact OnMountComplete called");

    delphi::objects::UpgStateReqPtr reqStatus = delphi::objects::UpgStateReq::FindObject(sdk_);
    if (reqStatus == NULL) {
        UPG_LOG_DEBUG("No UpgStateReq object found");
        return;
    }
    OnUpgStateReqCreate(reqStatus);
}

} // namespace upgrade
