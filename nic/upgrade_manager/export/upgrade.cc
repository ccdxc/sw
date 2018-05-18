// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade.hpp"

namespace upgrade {

using namespace std;

void UpgSdk::OnMountComplete(void) {
    LogInfo("UpgStateReqMgr OnMountComplete called");

    vector<delphi::objects::UpgStateReqPtr> upgReqStatuslist = delphi::objects::UpgStateReq::List(sdk_);
    for (vector<delphi::objects::UpgStateReqPtr>::iterator reqStatus=upgReqStatuslist.begin(); reqStatus != upgReqStatuslist.end(); ++reqStatus) {
        this->upgReqReactPtr_->OnUpgStateReqCreate(*reqStatus);
    }
}

void UpgSdk::SendAppRespSuccess(void) {
    LogInfo("UpgSdk::SendAppRespSuccess");
    this->upgAppRespPtr_->UpdateUpgAppResp(
          this->upgAppRespPtr_->GetUpgAppRespNextPass(
                this->upgAppRespPtr_->GetUpgStateReqPtr()->upgreqstate()));
}

void UpgSdk::SendAppRespFail(void) {
    LogInfo("UpgSdk::SendAppRespFail");
    this->upgAppRespPtr_->UpdateUpgAppResp(
          this->upgAppRespPtr_->GetUpgAppRespNextFail(
                this->upgAppRespPtr_->GetUpgStateReqPtr()->upgreqstate()));
}

} // namespace upgrade
