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
    LogInfo("Application returning success via UpgSdk");
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

delphi::error UpgSdk::IsRoleAgent (SvcRole role) {
    if (role != AGENT) {
        return delphi::error("Upgrade not initiated. Service is not of role AGENT.");
    }
    return delphi::error::OK(); 
}

delphi::objects::UpgReqPtr UpgSdk::FindUpgReqSpec(void) {
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);

    return static_pointer_cast<delphi::objects::UpgReq>(obj);
}

delphi::objects::UpgReqPtr UpgSdk::CreateUpgReqSpec(void) {
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(upgrade::InvalidCmd);

    // add it to database
    sdk_->SetObject(req);
    return req;
}

delphi::error UpgSdk::UpdateUpgReqSpec(delphi::objects::UpgReqPtr req, UpgReqType type) {
    req->set_upgreqcmd(type);

    // add it to database
    sdk_->SetObject(req);
    return delphi::error::OK();
}

delphi::error UpgSdk::StartUpgrade(void) {
    delphi::error err = delphi::error::OK();
    LogInfo("UpgSdk::StartUpgrade");
    RETURN_IF_FAILED(this->IsRoleAgent(this->svcRole_));

    delphi::objects::UpgReqPtr req = this->FindUpgReqSpec();
    if (req == NULL) {
        req = this->CreateUpgReqSpec();
    }
    this->UpdateUpgReqSpec(req, upgrade::UpgStart);
    
    return err; 
}

} // namespace upgrade
