// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_agent_handler.hpp"

namespace upgrade {

using namespace std;

void UpgAgentHandler::UpgStatePreUpgCheckComplete(HdlrResp &resp, string name) {
    LogInfo("Pre Upgrade Check completed with status {} error {} for service {}", resp.resp, resp.errStr, name);
    return;
}

void UpgAgentHandler::UpgStateProcessQuiesceComplete(HdlrResp &resp, string name) {
    LogInfo("Process quiesce completed with status {} error {} for service {}", resp.resp, resp.errStr, name);
    return;
}

void UpgAgentHandler::UpgStatePostBinRestartComplete(HdlrResp &resp, string name) {
    LogInfo("Post Bin Restart completed with status {} error {} for service {}", resp.resp, resp.errStr, name);
    return;
}

void UpgAgentHandler::UpgStateDataplaceDowntimeComplete(HdlrResp &resp, string name) {
    LogInfo("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, name);
    return;
}

void UpgAgentHandler::UpgStateCleanupComplete(HdlrResp &resp, string name) {
    LogInfo("Cleanup completed with status {} error {} for service {}", resp.resp, resp.errStr, name);
    return;
}

void UpgAgentHandler::UpgSuccessful(void) {
    LogInfo("Upgrade completed with status successfully!!!!");
    return;
}

void UpgAgentHandler::UpgFailed(vector<string> &errStrList) {
    LogInfo("Upgrade failed!!!!");
    for (uint i=0; i<errStrList.size(); i++) {
        LogInfo("Application failed response: {}", errStrList[i]);
    }
    return;
}

} // namespace upgrade
