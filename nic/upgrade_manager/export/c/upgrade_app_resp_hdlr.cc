// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_app_resp_hdlr.hpp"

namespace upgrade {

using namespace std;

delphi::objects::UpgAppRespPtr UpgAppRespHdlr::findUpgAppResp(string name) {
    delphi::objects::UpgAppRespPtr req = make_shared<delphi::objects::UpgAppResp>();
    req->set_key(name);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);

    return static_pointer_cast<delphi::objects::UpgAppResp>(obj);
}

delphi::error UpgAppRespHdlr::CreateUpgAppResp(void) {
    LogInfo("UpgAppRespHdlr::CreateUpgAppResp called");
    auto upgAppResp = findUpgAppResp(appName_);
    if (upgAppResp == NULL) {
        upgAppResp = make_shared<delphi::objects::UpgAppResp>();
        upgAppResp->set_key(appName_);
        if (upgAppResp == NULL)
            return delphi::error("application unable to create response object");
    }
    sdk_->SetObject(upgAppResp);
    return delphi::error::OK();
}

delphi::error UpgAppRespHdlr::DeleteUpgAppResp(void) {
    LogInfo("UpgAppRespHdlr::DeleteUpgAppResp called");
    auto upgAppResp = findUpgAppResp(appName_);
    if (upgAppResp == NULL) {
        return delphi::error("Response object does not exist");
    }
    sdk_->DeleteObject(upgAppResp);
    return delphi::error::OK();
}


string UpgAppRespHdlr::UpgAppRespValToStr(UpgRespStateType type) {
    return ((type%2==0)?StateMachine[type/2].upgAppRespValToStrPass:StateMachine[type/2].upgAppRespValToStrFail);
}

delphi::error UpgAppRespHdlr::UpdateUpgAppResp(UpgRespStateType type, HdlrResp appHdlrResp) {
    auto upgAppResp = findUpgAppResp(appName_);
    if (upgAppResp == NULL) {
        LogInfo("UpgAppRespHdlr::UpdateUpgAppResp returning error for {}", appName_);
        return delphi::error("application unable to find response object");
    }
    if (UpgAppRespValToStr(type) != "")
        LogInfo("{}", UpgAppRespValToStr(type));
    upgAppResp->set_upgapprespval(type);
    if (appHdlrResp.resp == FAIL) {
        upgAppResp->set_upgapprespstr(appHdlrResp.errStr);
    }
    sdk_->SetObject(upgAppResp);
    return delphi::error::OK();
}

bool UpgAppRespHdlr::CanInvokeHandler(UpgReqStateType reqType) {
   auto upgAppResp = findUpgAppResp(appName_);
    if (upgAppResp == NULL) {
        LogInfo("UpgAppRespHdlr::UpdateUpgAppResp not found for {}", appName_);
        return true;
    } 
    if ((GetUpgAppRespNextPass(reqType) == upgAppResp->upgapprespval()) ||
        (GetUpgAppRespNextFail(reqType) == upgAppResp->upgapprespval())) {
        LogInfo("Application {} already responded with {}", appName_, UpgAppRespValToStr(upgAppResp->upgapprespval()));
        return false;
    }
    LogInfo("Application {} was still processing {}", appName_, reqType); 
    return true;
}

UpgRespStateType
UpgAppRespHdlr::GetUpgAppRespNextPass(UpgReqStateType reqType) {
    //LogInfo("UpgAppRespHdlr::GetUpgAppRespNextPass got called for reqType {}", reqType);
    return StateMachine[reqType].statePassResp;
}

UpgRespStateType
UpgAppRespHdlr::GetUpgAppRespNextFail(UpgReqStateType reqType) {
    return StateMachine[reqType].stateFailResp;
}

UpgRespStateType
UpgAppRespHdlr::GetUpgAppRespNext(UpgReqStateType reqType, bool isReqSuccess) {
    if (isReqSuccess) {
        return GetUpgAppRespNextPass(reqType);
    } else {
        return GetUpgAppRespNextFail(reqType);
    }
}

} // namespace upgrade
