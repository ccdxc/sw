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

delphi::objects::UpgAppRespPtr UpgAppRespHdlr::findUpgAppResp(string name) {
    delphi::objects::UpgAppRespPtr req = make_shared<delphi::objects::UpgAppResp>();
    req->set_key(name);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);

    return static_pointer_cast<delphi::objects::UpgAppResp>(obj);
}

delphi::error UpgAppRespHdlr::CreateUpgAppResp(delphi::objects::UpgReqStatusPtr ptr) {
    LogInfo("UpgAppRespHdlr::CreateUpgAppResp called");
    auto upgAppResp = this->findUpgAppResp(this->appName_);
    if (upgAppResp == NULL) {
        upgAppResp = make_shared<delphi::objects::UpgAppResp>();
        upgAppResp->set_key(this->appName_);
        if (upgAppResp == NULL)
            return delphi::error("application unable to create response object");
    }
    this->upgReqStatus_ = ptr;
    this->sdk_->SetObject(upgAppResp);
    return delphi::error::OK();
}

delphi::error UpgAppRespHdlr::UpdateUpgAppResp(UpgRespStateType type) {
    LogInfo("UpgAppRespHdlr::UpdateUpgAppResp called for {} with type {}", this->appName_, type);
    auto upgAppResp = this->findUpgAppResp(this->appName_);
    if (upgAppResp == NULL) {
        LogInfo("UpgAppRespHdlr::UpdateUpgAppResp returning error for {}", this->appName_);
        return delphi::error("application unable to find response object");
    }
    upgAppResp->set_upgapprespval(type);
    this->sdk_->SetObject(upgAppResp);
    return delphi::error::OK();
}

// OnUpgReqStatusCreate gets called when UpgReqStatus object is created
delphi::error UpgReqReactor::OnUpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgReqReactor UpgReqStatus got created for {}/{}", req, req->meta().ShortDebugString());
    //create the object
    upgAppRespPtr_->CreateUpgAppResp(req);
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

UpgRespStateType
UpgAppRespHdlr::GetUpgAppRespNextPass(UpgReqStateType reqType) {
    LogInfo("UpgAppRespHdlr::GetUpgAppRespNextPass got called");
    switch (reqType) {
        case UpgReqRcvd:
            return UpgReqRcvdPass;
        case PreUpgState:
            return PreUpgStatePass;
        case PostBinRestart:
            return PostBinRestartPass;
        case ProcessesQuiesced:
            return ProcessesQuiescedPass;
        case DataplaneDowntimeStart:
            return DataplaneDowntimeStartPass;
        case Cleanup:
            return CleanupPass;
        case UpgSuccess:
            return UpgSuccessPass;
        case UpgFailed:
            return UpgFailedPass;
        case InvalidUpgState:
            return InvalidUpgStatePass;
        default:
            return InvalidUpgStatePass;
     }
}

UpgRespStateType
UpgAppRespHdlr::GetUpgAppRespNextFail(UpgReqStateType reqType) {
    switch (reqType) {
        case UpgReqRcvd:
            return UpgReqRcvdFail;
        case PreUpgState:
            return PreUpgStateFail;
        case PostBinRestart:
            return PostBinRestartFail;
        case ProcessesQuiesced:
            return ProcessesQuiescedFail;
        case DataplaneDowntimeStart:
            return DataplaneDowntimeStartFail;
        case Cleanup:
            return CleanupFail;
        case UpgSuccess:
            return UpgSuccessFail;
        case UpgFailed:
            return UpgFailedFail;
        case InvalidUpgState:
            return InvalidUpgStateFail;
        default:
            return InvalidUpgStateFail;
     }
}

UpgRespStateType
UpgAppRespHdlr::GetUpgAppRespNext(UpgReqStateType reqType, bool isReqSuccess) {
    if (isReqSuccess) {
        return GetUpgAppRespNextPass(reqType);
    } else {
        return GetUpgAppRespNextFail(reqType);
    }
}

// OnUpgReqState gets called when UpgReqState attribute changes
delphi::error UpgReqReactor::OnUpgReqState(delphi::objects::UpgReqStatusPtr req) {
    LogInfo("UpgReqReactor OnUpgReqState called");
    delphi::error errResp = delphi::error("Error processing OnUpgReqState");
    if (!this->upgHdlrPtr_) {
        LogInfo("No handlers available");
        return errResp;
    }
    switch (req->upgreqstate()) {
        case UpgReqRcvd:
            LogInfo("Upgrade: Request Received");
            errResp = this->upgHdlrPtr_->HandleStateUpgReqRcvd(req);
            break;
        case PreUpgState:
            LogInfo("Upgrade: Pre-upgrade check");
            errResp = this->upgHdlrPtr_->HandleStatePreUpgState(req);
            break;
        case PostBinRestart:
            LogInfo("Upgrade: Post-binary restart");
            errResp = this->upgHdlrPtr_->HandleStatePostBinRestart(req);
            break;
        case ProcessesQuiesced:
            LogInfo("Upgrade: Processes Quiesced");
            errResp = this->upgHdlrPtr_->HandleStateProcessesQuiesced(req);
            break;
        case DataplaneDowntimeStart:
            LogInfo("Upgrade: Dataplane Downtime Start");
            errResp = this->upgHdlrPtr_->HandleStateDataplaneDowntimeStart(req);
            break;
        case Cleanup:
            LogInfo("Upgrade: Cleanup Request Received");
            errResp = this->upgHdlrPtr_->HandleStateCleanup(req);
            break;
        case UpgSuccess:
            LogInfo("Upgrade: Succeeded");
            errResp = this->upgHdlrPtr_->HandleStateUpgSuccess(req);
            break;
        case UpgFailed:
            LogInfo("Upgrade: Failed");
            errResp = this->upgHdlrPtr_->HandleStateUpgFailed(req);
            break;
        case InvalidUpgState:
            LogInfo("Upgrade: Invalid Upgrade State");
            errResp = this->upgHdlrPtr_->HandleStateInvalidUpgState(req);
            break; 
        default:
            LogInfo("Upgrade: Default state");
            break; 
    }
    this->upgAppRespPtr_->UpdateUpgAppResp(this->upgAppRespPtr_->GetUpgAppRespNext(req->upgreqstate(), errResp.IsOK()));
    return errResp;
}

void UpgSdk::OnMountComplete(void) {
    LogInfo("UpgReqStatusMgr OnMountComplete called");

    vector<delphi::objects::UpgReqStatusPtr> upgReqStatuslist = delphi::objects::UpgReqStatus::List(sdk_);
    for (vector<delphi::objects::UpgReqStatusPtr>::iterator reqStatus=upgReqStatuslist.begin(); reqStatus != upgReqStatuslist.end(); ++reqStatus) {
        this->upgReqReactPtr_->OnUpgReqStatusCreate(*reqStatus);
    }
}

void UpgSdk::SendAppRespSuccess(void) {
    LogInfo("UpgSdk::SendAppRespSuccess");
    this->upgAppRespPtr_->UpdateUpgAppResp(
          this->upgAppRespPtr_->GetUpgAppRespNextPass(
                this->upgAppRespPtr_->GetUpgReqStatusPtr()->upgreqstate()));
}

void UpgSdk::SendAppRespFail(void) {
    LogInfo("UpgSdk::SendAppRespFail");
    this->upgAppRespPtr_->UpdateUpgAppResp(
          this->upgAppRespPtr_->GetUpgAppRespNextFail(
                this->upgAppRespPtr_->GetUpgReqStatusPtr()->upgreqstate()));
}

} // namespace upgrade
