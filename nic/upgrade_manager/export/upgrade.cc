// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"

namespace upgrade {

using namespace std;

delphi::error UpgHandler::UpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler UpgReqStatusCreate not implemented by service");
    return delphi::error::OK();
}

delphi::error UpgHandler::UpgReqStatusDelete(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler UpgReqStatusDelete not implemented by service");
    return delphi::error::OK();
}

delphi::error UpgHandler::HandleStateUpgReqRcvd(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateUpgReqRcvd not implemented by service");
    return delphi::error::OK();
}

delphi::error UpgHandler::HandleStatePreUpgState(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStatePreUpgState not implemented by service");
    return delphi::error::OK();
}

delphi::error UpgHandler::HandleStatePostBinRestart(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStatePostBinRestart not implemented by service");
    return delphi::error::OK();
}

delphi::error UpgHandler::HandleStateProcessesQuiesced(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateProcessesQuiesced not implemented by service");
    return delphi::error::OK();
}

delphi::error UpgHandler::HandleStateDataplaneDowntimeStart(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateDataplaneDowntimeStart not implemented by service");
    return delphi::error::OK();
}

delphi::error UpgHandler::HandleStateCleanup(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateCleanup not implemented by service");
    return delphi::error::OK();
}

delphi::error UpgHandler::HandleStateUpgSuccess(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateUpgSuccess not implemented by service");
    return delphi::error::OK();
}

delphi::error UpgHandler::HandleStateUpgFailed(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateUpgFailed not implemented by service");
    return delphi::error::OK();
}

delphi::error UpgHandler::HandleStateInvalidUpgState(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgHandler HandleStateInvalidUpgState not implemented by service");
    return delphi::error::OK();
}

// OnUpgReqStatusCreate gets called when UpgReqStatus object is created
delphi::error UpgReqReactor::OnUpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgReqReactor UpgReqStatus got created for {}/{}", req, req->meta().ShortDebugString());
    //create the object
    if (this->upgHdlrPtr_)
        return (this->upgHdlrPtr_->UpgReqStatusCreate(req));
    return delphi::error::OK();
}

// OnUpgReqStatusDelete gets called when UpgReqStatus object is deleted
delphi::error UpgReqReactor::OnUpgReqStatusDelete(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgReqReactor UpgReqStatus got deleted");
    //delete the object
    if (this->upgHdlrPtr_)
        return (this->upgHdlrPtr_->UpgReqStatusDelete(req));
    return delphi::error::OK();
}

// OnUpgReqState gets called when UpgReqState attribute changes
delphi::error UpgReqReactor::OnUpgReqState(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgReqReactor OnUpgReqState called");
    delphi::error resp = delphi::error("Error processing OnUpgReqState");
    if (!this->upgHdlrPtr_) {
        LogInfo("No handlers available");
        return resp;
    }
    switch (req->upgreqstate()) {
        case upgrade::UpgReqRcvd:
            LogInfo("Upgrade: Request Received");
            resp = this->upgHdlrPtr_->HandleStateUpgReqRcvd(req);
            break;
        case upgrade::PreUpgState:
            LogInfo("Upgrade: Pre-upgrade check");
            resp = this->upgHdlrPtr_->HandleStatePreUpgState(req);
            break;
        case upgrade::PostBinRestart:
            LogInfo("Upgrade: Post-binary restart");
            resp = this->upgHdlrPtr_->HandleStatePostBinRestart(req);
            break;
        case upgrade::ProcessesQuiesced:
            LogInfo("Upgrade: Processes Quiesced");
            resp = this->upgHdlrPtr_->HandleStateProcessesQuiesced(req);
            break;
        case upgrade::DataplaneDowntimeStart:
            LogInfo("Upgrade: Dataplane Downtime Start");
            resp = this->upgHdlrPtr_->HandleStateDataplaneDowntimeStart(req);
            break;
        case upgrade::Cleanup:
            LogInfo("Upgrade: Cleanup Request Received");
            resp = this->upgHdlrPtr_->HandleStateCleanup(req);
            break;
        case upgrade::UpgSuccess:
            LogInfo("Upgrade: Succeeded");
            resp = this->upgHdlrPtr_->HandleStateUpgSuccess(req);
            break;
        case upgrade::UpgFailed:
            LogInfo("Upgrade: Failed");
            resp = this->upgHdlrPtr_->HandleStateUpgFailed(req);
            break;
        case upgrade::InvalidUpgState:
            LogInfo("Upgrade: Invalid Upgrade State");
            resp = this->upgHdlrPtr_->HandleStateInvalidUpgState(req);
            break; 
        default:
            LogInfo("Upgrade: Default state");
            break; 
    }
    return resp;
}

void UpgSdk::OnMountComplete(void) {
    LogInfo("UpgReqStatusMgr OnMountComplete called");

    vector<delphi::objects::UpgReqStatusPtr> upgReqStatuslist = delphi::objects::UpgReqStatus::List(sdk_);
    for (vector<delphi::objects::UpgReqStatusPtr>::iterator reqStatus=upgReqStatuslist.begin(); reqStatus != upgReqStatuslist.end(); ++reqStatus) {
        this->upgReqReactPtr_->OnUpgReqStatusCreate(*reqStatus);
    }
}

} // namespace upgrade
