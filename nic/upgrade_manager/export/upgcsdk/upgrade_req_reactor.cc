// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_req_reactor.hpp"
#include "nic/upgrade_manager/include/c/upgrade_metadata.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

UpgCtx ctx;

void UpgReqReactor::InvokeAppHdlr(UpgReqStateType type, HdlrResp &hdlrResp) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    switch (type) {
        case UpgStateCompatCheck:
            UPG_LOG_DEBUG("Upgrade: Pre-upgrade check");
            hdlrResp = upgHdlrPtr_->HandleUpgStateCompatCheck(ctx);
            break;
        case UpgStatePostBinRestart:
            UPG_LOG_DEBUG("Upgrade: Post-binary restart");
            hdlrResp = upgHdlrPtr_->HandleUpgStatePostBinRestart(ctx);
            break;
        case UpgStateProcessQuiesce:
            UPG_LOG_DEBUG("Upgrade: Processes Quiesced");
            hdlrResp = upgHdlrPtr_->HandleUpgStateProcessQuiesce(ctx);
            break;
        case UpgStateDataplaneDowntimePhase1:
            UPG_LOG_DEBUG("Upgrade: Dataplane Downtime Phase1 ");
            hdlrResp = upgHdlrPtr_->HandleUpgStateDataplaneDowntimePhase1(ctx);
            break;
        case UpgStateDataplaneDowntimePhase2:
            UPG_LOG_DEBUG("Upgrade: Dataplane Downtime Phase2 ");
            hdlrResp = upgHdlrPtr_->HandleUpgStateDataplaneDowntimePhase2(ctx);
            break;
        case UpgStateDataplaneDowntimePhase3:
            UPG_LOG_DEBUG("Upgrade: Dataplane Downtime Phase3 ");
            hdlrResp = upgHdlrPtr_->HandleUpgStateDataplaneDowntimePhase3(ctx);
            break;
        case UpgStateDataplaneDowntimePhase4:
            UPG_LOG_DEBUG("Upgrade: Dataplane Downtime Phase4 ");
            hdlrResp = upgHdlrPtr_->HandleUpgStateDataplaneDowntimePhase4(ctx);
            break;
        case UpgStateCleanup:
            UPG_LOG_DEBUG("Upgrade: Cleanup Request Received");
            hdlrResp = upgHdlrPtr_->HandleUpgStateCleanup(ctx);
            break;
        case UpgStateSuccess:
            UPG_LOG_DEBUG("Upgrade: Succeeded");
            hdlrResp = resp;
            upgHdlrPtr_->HandleUpgStateSuccess(ctx);
            break;
        case UpgStateFailed:
            UPG_LOG_DEBUG("Upgrade: Failed");
            hdlrResp = resp;
            upgHdlrPtr_->HandleUpgStateFailed(ctx);
            break;
        case UpgStateAbort:
            UPG_LOG_DEBUG("Upgrade: Abort");
            hdlrResp = resp;
            upgHdlrPtr_->HandleUpgStateAbort(ctx);
            break;
        default:
            UPG_LOG_DEBUG("Upgrade: Default state");
            break;
    }
}

void UpgReqReactor::GetUpgCtx(delphi::objects::UpgStateReqPtr req) {
    ctx.upgType = req->upgreqtype(); 
    GetUpgCtxFromMeta(ctx);
}

// OnUpgStateReqCreate gets called when UpgStateReq object is created
delphi::error UpgReqReactor::OnUpgStateReqCreate(delphi::objects::UpgStateReqPtr req) {
    UPG_LOG_DEBUG("UpgReqReactor UpgStateReq got created for {}/{}/{}", req, req->meta().ShortDebugString(), req->upgreqstate());
    //create the object
    if (upgHdlrPtr_ && upgAppRespPtr_->CanInvokeHandler(req->upgreqstate())) {
        upgAppRespPtr_->CreateUpgAppResp();
        HdlrResp hdlrResp;
        UpgReqReactor::GetUpgCtx(req);
        InvokeAppHdlr(req->upgreqstate(), hdlrResp);
        if (hdlrResp.resp != INPROGRESS) {
            upgAppRespPtr_->UpdateUpgAppResp(upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrResp.resp==SUCCESS)), hdlrResp);
        } else {
            UPG_LOG_DEBUG("Application still processing");
        }
    }
    return delphi::error::OK();
}

// OnUpgStateReqDelete gets called when UpgStateReq object is deleted
delphi::error UpgReqReactor::OnUpgStateReqDelete(delphi::objects::UpgStateReqPtr req) {
    UPG_LOG_DEBUG("UpgReqReactor UpgStateReq got deleted with {}", req->upgreqstate());
    //delete the object
    upgAppRespPtr_->DeleteUpgAppResp();
    return delphi::error::OK();
}

// OnUpgReqState gets called when UpgReqState attribute changes
delphi::error UpgReqReactor::OnUpgReqState(delphi::objects::UpgStateReqPtr req) {
    HdlrResp hdlrResp;
    if (!upgHdlrPtr_) {
        UPG_LOG_ERROR("No handlers available");
        return delphi::error("Error processing OnUpgReqState");
    }
    if (req->upgreqstate() != UpgStateTerminal)
        UPG_LOG_DEBUG("\n\n\n===== Incoming Message =====");

    InvokeAppHdlr(req->upgreqstate(), hdlrResp);
    if (hdlrResp.resp != INPROGRESS) {
        if (req->upgreqstate() != UpgStateTerminal)
            UPG_LOG_DEBUG("Application returned {}", (hdlrResp.resp==SUCCESS)?"success":"fail");
        upgAppRespPtr_->UpdateUpgAppResp(upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrResp.resp==SUCCESS)), hdlrResp);
    } else {
        UPG_LOG_DEBUG("Application still processing"); 
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
    UPG_LOG_DEBUG("UpgReqReactor::RegisterUpgApp");
    delphi::objects::UpgAppPtr app = FindUpgAppPtr();
    if (app == NULL) {
        UPG_LOG_DEBUG("Creating UpgApp");
        app = CreateUpgAppObj();
    }
    UPG_LOG_DEBUG("Returning after creating UpgApp");
}

void UpgReqReactor::OnMountComplete(void) {
    RegisterUpgApp();
    UPG_LOG_DEBUG("UpgReqReactor OnMountComplete called");

    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
    req->set_key(10);

    //find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);

    delphi::objects::UpgStateReqPtr reqStatus = static_pointer_cast<delphi::objects::UpgStateReq>(obj);
    if (reqStatus == NULL) {
        UPG_LOG_DEBUG("No UpgStateReq object found");
        return;
    }
    OnUpgStateReqCreate(reqStatus);
}

} // namespace upgrade
