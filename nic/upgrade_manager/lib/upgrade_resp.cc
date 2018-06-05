// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_resp.hpp"

namespace upgrade {

using namespace std;

delphi::objects::UpgRespPtr UpgMgrResp::findUpgMgrRespObj (uint32_t id) {
    delphi::objects::UpgRespPtr resp = make_shared<delphi::objects::UpgResp>();
    resp->set_key(id);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(resp);

    return static_pointer_cast<delphi::objects::UpgResp>(obj);
}

delphi::error UpgMgrResp::createUpgMgrResp(uint32_t id, UpgRespType val, vector<string> &str) {
    // create an object
    delphi::objects::UpgRespPtr resp = make_shared<delphi::objects::UpgResp>();
    resp->set_key(id);
    resp->set_upgrespval(val);
    while (!str.empty()) {
        resp->add_upgrespfailstr(str.back());
        str.pop_back();
    }
    // add it to database
    sdk_->SetObject(resp);

    LogInfo("Created upgrade response object for id {} state {} req: {}", id, val, resp);

    return delphi::error::OK();
}

delphi::error UpgMgrResp::DeleteUpgMgrResp(void) {
    auto upgResp = findUpgMgrRespObj(10);
    if (upgResp != NULL) {
        LogInfo("UpgResp object got deleted for agent");
        sdk_->DeleteObject(upgResp);
    }
    return delphi::error::OK();
}

delphi::error UpgMgrResp::UpgradeFinish(bool success, vector<string> &str) {
    UpgRespType respType = success ? UpgPass : UpgFail; 
    LogInfo("Returning response {} to agent", (respType==UpgPass)?"Upgrade successful":"Upgrade Fail");
    auto upgResp = findUpgMgrRespObj(10);
    if (upgResp == NULL) {
        LogInfo("Sending Following String to Agent");
        for (uint i=0; i<str.size(); i++) {
            LogInfo("{}", str[i]);
        }
        RETURN_IF_FAILED(createUpgMgrResp(10, respType, str));
    }
    LogInfo("Responded back to the agent");
    return delphi::error::OK();
}

}
