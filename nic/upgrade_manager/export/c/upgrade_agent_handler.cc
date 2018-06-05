// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_agent_handler.hpp"

namespace upgrade {

using namespace std;

void UpgAgentHandler::UpgStatePreUpgCheckComplete(HdlrResp &resp, string svcName) {
    LogInfo("Pre Upgrade Check completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateProcessQuiesceComplete(HdlrResp &resp, string svcName) {
    LogInfo("Process quiesce completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStatePostBinRestartComplete(HdlrResp &resp, string svcName) {
    LogInfo("Post Bin Restart completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimePhase1Complete(HdlrResp &resp, string svcName) {
    LogInfo("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimeAdminQComplete(HdlrResp &resp, string svcName) {
    LogInfo("Dataplane downtime adminq handling completed with status {} error {} for service {}}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimePhase2Complete(HdlrResp &resp, string svcName) {
    LogInfo("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateCleanupComplete(HdlrResp &resp, string svcName) {
    LogInfo("Cleanup completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateAbortedComplete(HdlrResp &resp, string svcName) {
    LogInfo("Aborted completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
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
