// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_mgr_agent_resp_reactor.hpp"

namespace upgrade {

using namespace std;

string UpgMgrAgentRespReact::GetRespStr(delphi::objects::UpgRespPtr resp) {
    switch (resp->upgrespval()) {
        case upgrade::UpgPass:
            return ("Upgrade Successful");
        case upgrade::UpgFail:
            return ("Upgrade Failed");
        default:
            return ("");
    }
}

delphi::error UpgMgrAgentRespReact::DeleteUpgReqSpec(void) {
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);

    req = static_pointer_cast<delphi::objects::UpgReq>(obj);
 
    sdk_->DeleteObject(req);
    return delphi::error::OK();
}

void UpgMgrAgentRespReact::InvokeAgentHandler(delphi::objects::UpgRespPtr resp) {
    switch (resp->upgrespval()) {
        case upgrade::UpgPass:
            upgAgentHandler_->UpgSuccessful();
            break;
        case upgrade::UpgFail:
            upgAgentHandler_->UpgFailed();
            break;
        default:
            break;
    }
    if (this->DeleteUpgReqSpec() == delphi::error::OK()) {
        LogInfo("Upgrade Req Object deleted for next request");
    }
}

delphi::error UpgMgrAgentRespReact::OnUpgRespCreate(delphi::objects::UpgRespPtr resp) {
    LogInfo("UpgRespHdlr::OnUpgRespCreate called with status {}", this->GetRespStr(resp));
    this->InvokeAgentHandler(resp);
    return delphi::error::OK();
}

delphi::error UpgMgrAgentRespReact::OnUpgRespVal(delphi::objects::UpgRespPtr
resp) {
    if (this->GetRespStr(resp) != "")
        LogInfo("UpgRespHdlr::OnUpgRespVal called with status: {}", this->GetRespStr(resp));
    this->InvokeAgentHandler(resp);
    return delphi::error::OK();
}

};
