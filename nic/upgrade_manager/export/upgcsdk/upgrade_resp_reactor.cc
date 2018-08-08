// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_resp_reactor.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

string UpgRespReact::GetRespStr(delphi::objects::UpgRespPtr resp) {
    switch (resp->upgrespval()) {
        case UpgRespPass:
            return ("Upgrade Successful");
        case UpgRespFail:
            return ("Upgrade Failed");
        case UpgRespAbort:
            return ("Upgrade Aborted");
        default:
            return ("");
    }
}

delphi::error UpgRespReact::DeleteUpgReqSpec(void) {
    delphi::objects::UpgReqPtr req = delphi::objects::UpgReq::FindObject(sdk_);
    sdk_->DeleteObject(req);
    return delphi::error::OK();
}

void UpgRespReact::InvokeAgentHandler(delphi::objects::UpgRespPtr resp) {
    vector<string> errStrList;
    switch (resp->upgrespval()) {
        case UpgRespPass:
            upgAgentHandler_->UpgSuccessful();
            break;
        case UpgRespFail:
            for (int i=0; i<resp->upgrespfailstr_size(); i++) {
                errStrList.push_back(resp->upgrespfailstr(i));
            }
            upgAgentHandler_->UpgFailed(errStrList);
            break;
        case UpgRespAbort:
            for (int i=0; i<resp->upgrespfailstr_size(); i++) {
                errStrList.push_back(resp->upgrespfailstr(i));
            }
            upgAgentHandler_->UpgAborted(errStrList);
            break;
        case UpgRespUpgPossible:
            if (!resp->upgrespfailstr().empty()) {
                for (int i=0; i<resp->upgrespfailstr_size(); i++) {
                    errStrList.push_back(resp->upgrespfailstr(i));
                }
                upgAgentHandler_->UpgNotPossible(errStrList);
            } else {
                upgAgentHandler_->UpgPossible();
            }
            break;
        default:
            break;
    }
}

delphi::error UpgRespReact::OnUpgRespCreate(delphi::objects::UpgRespPtr resp) {
    UPG_LOG_DEBUG("UpgRespHdlr::OnUpgRespCreate called with status {}", GetRespStr(resp));
    if (DeleteUpgReqSpec() == delphi::error::OK()) {
        UPG_LOG_DEBUG("Upgrade Req Object deleted for next request");
    }
    InvokeAgentHandler(resp);
    return delphi::error::OK();
}

delphi::error UpgRespReact::OnUpgRespVal(delphi::objects::UpgRespPtr
resp) {
    if (GetRespStr(resp) != "")
        UPG_LOG_DEBUG("UpgRespHdlr::OnUpgRespVal called with status: {}", GetRespStr(resp));
    if (DeleteUpgReqSpec() == delphi::error::OK()) {
        UPG_LOG_DEBUG("Upgrade Req Object deleted for next request");
    }
    InvokeAgentHandler(resp);
    return delphi::error::OK();
}

delphi::objects::UpgRespPtr UpgRespReact::FindUpgRespSpec(void) {
    return delphi::objects::UpgResp::FindObject(sdk_);
}

};
