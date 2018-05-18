// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"

namespace upgrade {

using namespace std;

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
