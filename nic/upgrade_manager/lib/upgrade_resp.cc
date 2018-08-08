// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_resp.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

delphi::objects::UpgRespPtr UpgMgrResp::findUpgMgrRespObj () {
    return delphi::objects::UpgResp::FindObject(sdk_);
}

void UpgMgrResp::updateUpgMgrResp(delphi::objects::UpgRespPtr resp, UpgRespType val, vector<string> &str) {
    resp->set_upgrespval(val);
    while (!str.empty()) {
        resp->add_upgrespfailstr(str.back());
        str.pop_back();
    }
    // add it to database
    sdk_->SetObject(resp);
}

delphi::error UpgMgrResp::createUpgMgrResp(UpgRespType val, vector<string> &str) {
    // create an object
    delphi::objects::UpgRespPtr resp = make_shared<delphi::objects::UpgResp>();
    updateUpgMgrResp(resp, val, str);
    UPG_LOG_DEBUG("Created upgrade response object with state {} resp: {}", val, resp);

    return delphi::error::OK();
}

delphi::error UpgMgrResp::DeleteUpgMgrResp(void) {
    auto upgResp = findUpgMgrRespObj();
    if (upgResp != NULL) {
        UPG_LOG_DEBUG("UpgResp object got deleted for agent");
        sdk_->DeleteObject(upgResp);
    }
    return delphi::error::OK();
}

delphi::error UpgMgrResp::UpgradeFinish(UpgRespType respType, vector<string> &str) {
    UPG_LOG_INFO("Returning response {} to agent", respType);
    auto upgResp = findUpgMgrRespObj();
    if (upgResp == NULL) {
        UPG_LOG_DEBUG("Sending Following String to Agent");
        for (uint i=0; i<str.size(); i++) {
            UPG_LOG_DEBUG("{}", str[i]);
        }
        RETURN_IF_FAILED(createUpgMgrResp(respType, str));
    } else {
        //Update the value in the response object to the apps
        updateUpgMgrResp(upgResp, respType, str);
    }
    UPG_LOG_DEBUG("Responded back to the agent");
    return delphi::error::OK();
}

}
