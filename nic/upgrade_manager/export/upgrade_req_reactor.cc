// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_req_reactor.hpp"

namespace upgrade {

using namespace std;

// OnUpgStateReqCreate gets called when UpgStateReq object is created
delphi::error UpgReqReactor::OnUpgStateReqCreate(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgReqReactor UpgStateReq got created for {}/{}/{}", req, req->meta().ShortDebugString(), req->upgreqstate());
    //create the object
    upgAppRespPtr_->CreateUpgAppResp();
    if (this->upgHdlrPtr_) {
        HdlrResp hdlrResp;
        hdlrResp = this->upgHdlrPtr_->HandleStateUpgReqRcvd(req);
        if (hdlrResp.resp != INPROGRESS) {
            this->upgAppRespPtr_->UpdateUpgAppResp(this->upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrResp.resp==SUCCESS)), hdlrResp);
        }
    }
    return delphi::error::OK();
}

// OnUpgStateReqDelete gets called when UpgStateReq object is deleted
delphi::error UpgReqReactor::OnUpgStateReqDelete(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgReqReactor UpgStateReq got deleted");
    //delete the object
    upgAppRespPtr_->DeleteUpgAppResp();
    HdlrResp hdlrResp;
    if (this->upgHdlrPtr_) {
        hdlrResp = this->upgHdlrPtr_->UpgStateReqDelete(req);
        (void)hdlrResp;
        //if (hdlrResp != INPROGRESS) {
          //  this->upgAppRespPtr_->UpdateUpgAppResp(this->upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrResp==SUCCESS)));
        //}
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
    switch (req->upgreqstate()) {
        case UpgReqRcvd:
            LogInfo("Upgrade: Request Received");
            hdlrResp = this->upgHdlrPtr_->HandleStateUpgReqRcvd(req);
            break;
        case PreUpgState:
            LogInfo("Upgrade: Pre-upgrade check");
            hdlrResp = this->upgHdlrPtr_->HandleStatePreUpgState(req);
            break;
        case PostBinRestart:
            LogInfo("Upgrade: Post-binary restart");
            hdlrResp = this->upgHdlrPtr_->HandleStatePostBinRestart(req);
            break;
        case ProcessesQuiesced:
            LogInfo("Upgrade: Processes Quiesced");
            hdlrResp = this->upgHdlrPtr_->HandleStateProcessesQuiesced(req);
            break;
        case DataplaneDowntimeStart:
            LogInfo("Upgrade: Dataplane Downtime Start");
            hdlrResp = this->upgHdlrPtr_->HandleStateDataplaneDowntimeStart(req);
            break;
        case Cleanup:
            LogInfo("Upgrade: Cleanup Request Received");
            hdlrResp = this->upgHdlrPtr_->HandleStateCleanup(req);
            break;
        case UpgSuccess:
            LogInfo("Upgrade: Succeeded");
            hdlrResp = this->upgHdlrPtr_->HandleStateUpgSuccess(req);
            break;
        case UpgFailed:
            LogInfo("Upgrade: Failed");
            hdlrResp = this->upgHdlrPtr_->HandleStateUpgFailed(req);
            break;
        default:
            LogInfo("Upgrade: Default state");
            break; 
    }
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
