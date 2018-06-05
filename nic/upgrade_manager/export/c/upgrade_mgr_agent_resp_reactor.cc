// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_mgr_agent_resp_reactor.hpp"

namespace upgrade {

using namespace std;

string UpgMgrAgentRespReact::GetRespStr(delphi::objects::UpgRespPtr resp) {
    switch (resp->upgrespval()) {
        case UpgPass:
            return ("Upgrade Successful");
        case UpgFail:
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
    vector<string> errStrList;
    switch (resp->upgrespval()) {
        case UpgPass:
            upgAgentHandler_->UpgSuccessful();
            break;
        case UpgFail:
            for (int i=0; i<resp->upgrespfailstr_size(); i++) {
                errStrList.push_back(resp->upgrespfailstr(i));
            }
            upgAgentHandler_->UpgFailed(errStrList);
            break;
        default:
            break;
    }
    if (DeleteUpgReqSpec() == delphi::error::OK()) {
        LogInfo("Upgrade Req Object deleted for next request");
    }
}

delphi::error UpgMgrAgentRespReact::OnUpgRespCreate(delphi::objects::UpgRespPtr resp) {
    LogInfo("UpgRespHdlr::OnUpgRespCreate called with status {}", GetRespStr(resp));
    InvokeAgentHandler(resp);
    return delphi::error::OK();
}

delphi::error UpgMgrAgentRespReact::OnUpgRespVal(delphi::objects::UpgRespPtr
resp) {
    if (GetRespStr(resp) != "")
        LogInfo("UpgRespHdlr::OnUpgRespVal called with status: {}", GetRespStr(resp));
    InvokeAgentHandler(resp);
    return delphi::error::OK();
}

delphi::objects::UpgRespPtr UpgMgrAgentRespReact::FindUpgRespSpec(void) {
    delphi::objects::UpgRespPtr req = make_shared<delphi::objects::UpgResp>();
    req->set_key(10);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(req);

    return static_pointer_cast<delphi::objects::UpgResp>(obj);
}

};
