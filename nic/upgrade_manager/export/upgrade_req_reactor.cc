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
        HdlrRespCode hdlrRespCode;
        hdlrRespCode = this->upgHdlrPtr_->HandleStateUpgReqRcvd(req);
        if (hdlrRespCode != INPROGRESS) {
            this->upgAppRespPtr_->UpdateUpgAppResp(this->upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrRespCode==SUCCESS)));
        }
    }
    return delphi::error::OK();
}

// OnUpgStateReqDelete gets called when UpgStateReq object is deleted
delphi::error UpgReqReactor::OnUpgStateReqDelete(delphi::objects::UpgStateReqPtr req) {
    LogInfo("UpgReqReactor UpgStateReq got deleted");
    //delete the object
    upgAppRespPtr_->DeleteUpgAppResp();
    HdlrRespCode hdlrRespCode;
    if (this->upgHdlrPtr_) {
        hdlrRespCode = this->upgHdlrPtr_->UpgStateReqDelete(req);
        (void)hdlrRespCode;
        //if (hdlrRespCode != INPROGRESS) {
          //  this->upgAppRespPtr_->UpdateUpgAppResp(this->upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrRespCode==SUCCESS)));
        //}
    }
    return delphi::error::OK();
}

// OnUpgReqState gets called when UpgReqState attribute changes
delphi::error UpgReqReactor::OnUpgReqState(delphi::objects::UpgStateReqPtr req) {
    HdlrRespCode hdlrRespCode;
    if (!this->upgHdlrPtr_) {
        LogInfo("No handlers available");
        return delphi::error("Error processing OnUpgReqState");
    }
    if (req->upgreqstate() != UpgStateTerminal)
        LogInfo("\n\n\n===== Incoming Message =====");
    switch (req->upgreqstate()) {
        case UpgReqRcvd:
            LogInfo("Upgrade: Request Received");
            hdlrRespCode = this->upgHdlrPtr_->HandleStateUpgReqRcvd(req);
            break;
        case PreUpgState:
            LogInfo("Upgrade: Pre-upgrade check");
            hdlrRespCode = this->upgHdlrPtr_->HandleStatePreUpgState(req);
            break;
        case PostBinRestart:
            LogInfo("Upgrade: Post-binary restart");
            hdlrRespCode = this->upgHdlrPtr_->HandleStatePostBinRestart(req);
            break;
        case ProcessesQuiesced:
            LogInfo("Upgrade: Processes Quiesced");
            hdlrRespCode = this->upgHdlrPtr_->HandleStateProcessesQuiesced(req);
            break;
        case DataplaneDowntimeStart:
            LogInfo("Upgrade: Dataplane Downtime Start");
            hdlrRespCode = this->upgHdlrPtr_->HandleStateDataplaneDowntimeStart(req);
            break;
        case Cleanup:
            LogInfo("Upgrade: Cleanup Request Received");
            hdlrRespCode = this->upgHdlrPtr_->HandleStateCleanup(req);
            break;
        case UpgSuccess:
            LogInfo("Upgrade: Succeeded");
            hdlrRespCode = this->upgHdlrPtr_->HandleStateUpgSuccess(req);
            break;
        case UpgFailed:
            LogInfo("Upgrade: Failed");
            hdlrRespCode = this->upgHdlrPtr_->HandleStateUpgFailed(req);
            break;
        default:
            LogInfo("Upgrade: Default state");
            break; 
    }
    if (hdlrRespCode != INPROGRESS) {
        if (req->upgreqstate() != UpgStateTerminal)
            LogInfo("Application returned {}", (hdlrRespCode==SUCCESS)?"success":"fail");
        this->upgAppRespPtr_->UpdateUpgAppResp(this->upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), (hdlrRespCode==SUCCESS)));
    } else {
        LogInfo("Application still processing"); 
    }
    return delphi::error::OK();
}

} // namespace upgrade
