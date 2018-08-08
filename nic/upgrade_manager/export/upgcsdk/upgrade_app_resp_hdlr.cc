// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_app_resp_hdlr.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

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
    UPG_LOG_DEBUG("UpgAppRespHdlr::CreateUpgAppResp called");
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
    UPG_LOG_DEBUG("UpgAppRespHdlr::DeleteUpgAppResp called");
    auto upgAppResp = findUpgAppResp(appName_);
    if (upgAppResp == NULL) {
        return delphi::error("Response object does not exist");
    }
    sdk_->DeleteObject(upgAppResp);
    return delphi::error::OK();
}


string UpgAppRespHdlr::UpgAppRespValToStr(UpgStateRespType type) {
    return GetUpgAppRespValToStr(type);
}

delphi::error UpgAppRespHdlr::UpdateUpgAppResp(UpgStateRespType type, HdlrResp appHdlrResp) {
    auto upgAppResp = findUpgAppResp(appName_);
    if (upgAppResp == NULL) {
        UPG_LOG_DEBUG("UpgAppRespHdlr::UpdateUpgAppResp returning error for {}", appName_);
        return delphi::error("application unable to find response object");
    }
    if (UpgAppRespValToStr(type) != "")
        UPG_LOG_DEBUG("{}", UpgAppRespValToStr(type));
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
        UPG_LOG_DEBUG("UpgAppRespHdlr::CanInvokeHandler not found for {}", appName_);
        return true;
    } 
    if ((GetUpgAppRespNextPass(reqType) == upgAppResp->upgapprespval()) ||
        (GetUpgAppRespNextFail(reqType) == upgAppResp->upgapprespval())) {
        UPG_LOG_DEBUG("Application {} already responded with {}", appName_, UpgAppRespValToStr(upgAppResp->upgapprespval()));
        return false;
    }
    UPG_LOG_DEBUG("Application {} was still processing {}", appName_, reqType); 
    return true;
}

UpgStateRespType
UpgAppRespHdlr::GetUpgAppRespNextPass(UpgReqStateType reqType) {
    UPG_LOG_DEBUG("UpgAppRespHdlr::GetUpgAppRespNextPass got called for reqType {}", reqType);
    if (reqType == UpgStateUpgPossible) {
        UPG_LOG_DEBUG("CanUpgradeStateMachine returning {}", CanUpgradeStateMachine[reqType].statePassResp);
        return CanUpgradeStateMachine[reqType].statePassResp;
    }
    return UpgradeStateMachine[reqType].statePassResp;
}

UpgStateRespType
UpgAppRespHdlr::GetUpgAppRespNextFail(UpgReqStateType reqType) {
    UPG_LOG_DEBUG("UpgAppRespHdlr::GetUpgAppRespNextFail got called for reqType {}", reqType);
    if (reqType == UpgStateUpgPossible) { 
        UPG_LOG_DEBUG("CanUpgradeStateMachine returning {}", CanUpgradeStateMachine[reqType].statePassResp);
        return CanUpgradeStateMachine[reqType].stateFailResp;
    }
    return UpgradeStateMachine[reqType].stateFailResp;
}

UpgStateRespType
UpgAppRespHdlr::GetUpgAppRespNext(UpgReqStateType reqType, bool isReqSuccess) {
    if (isReqSuccess) {
        return GetUpgAppRespNextPass(reqType);
    } else {
        return GetUpgAppRespNextFail(reqType);
    }
}

} // namespace upgrade
