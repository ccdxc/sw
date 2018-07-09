// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_agent_handler.hpp"

namespace upgrade {

using namespace std;

void UpgAgentHandler::UpgStateCompatCheckCompletionHandler(HdlrResp &resp, string svcName) {
    LogInfo("Pre Upgrade Check completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateProcessQuiesceCompletionHandler(HdlrResp &resp, string svcName) {
    LogInfo("Process quiesce completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStatePostBinRestartCompletionHandler(HdlrResp &resp, string svcName) {
    LogInfo("Post Bin Restart completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimePhase1CompletionHandler(HdlrResp &resp, string svcName) {
    LogInfo("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimePhase2CompletionHandler(HdlrResp &resp, string svcName) {
    LogInfo("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimePhase3CompletionHandler(HdlrResp &resp, string svcName) {
    LogInfo("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimePhase4CompletionHandler(HdlrResp &resp, string svcName) {
    LogInfo("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateCleanupCompletionHandler(HdlrResp &resp, string svcName) {
    LogInfo("Cleanup completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateAbortedCompletionHandler(HdlrResp &resp, string svcName) {
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

void UpgAgentHandler::UpgAborted(vector<string> &errStrList) {
    LogInfo("Upgrade aborted!!!!");
    for (uint i=0; i<errStrList.size(); i++) {
        LogInfo("Application failed response: {}", errStrList[i]);
    }
    return;
}

} // namespace upgrade
