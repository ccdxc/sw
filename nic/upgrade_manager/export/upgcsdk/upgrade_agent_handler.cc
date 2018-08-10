// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_agent_handler.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

void UpgAgentHandler::UpgStateCompatCheckCompletionHandler(HdlrResp &resp, string svcName) {
    UPG_LOG_DEBUG("Pre Upgrade Check completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateProcessQuiesceCompletionHandler(HdlrResp &resp, string svcName) {
    UPG_LOG_DEBUG("Process quiesce completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateLinkDownCompletionHandler(HdlrResp &resp, string svcName) {
    UPG_LOG_DEBUG("Link down completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStatePostBinRestartCompletionHandler(HdlrResp &resp, string svcName) {
    UPG_LOG_DEBUG("Post Bin Restart completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimePhase1CompletionHandler(HdlrResp &resp, string svcName) {
    UPG_LOG_DEBUG("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimePhase2CompletionHandler(HdlrResp &resp, string svcName) {
    UPG_LOG_DEBUG("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimePhase3CompletionHandler(HdlrResp &resp, string svcName) {
    UPG_LOG_DEBUG("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateDataplaneDowntimePhase4CompletionHandler(HdlrResp &resp, string svcName) {
    UPG_LOG_DEBUG("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateCleanupCompletionHandler(HdlrResp &resp, string svcName) {
    UPG_LOG_DEBUG("Cleanup completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgStateAbortedCompletionHandler(HdlrResp &resp, string svcName) {
    UPG_LOG_DEBUG("Aborted completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgSuccessful(void) {
    UPG_LOG_DEBUG("Upgrade completed with status successfully!!!!");
    return;
}

void UpgAgentHandler::UpgFailed(vector<string> &errStrList) {
    UPG_LOG_DEBUG("Upgrade failed!!!!");
    for (uint i=0; i<errStrList.size(); i++) {
        UPG_LOG_DEBUG("Application failed response: {}", errStrList[i]);
    }
    return;
}

void UpgAgentHandler::UpgNotPossible(vector<string> &errStrList) {
    UPG_LOG_DEBUG("Upgrade Not Possible!!!!");
    for (uint i=0; i<errStrList.size(); i++) {
        UPG_LOG_DEBUG("Application failed response: {}", errStrList[i]);
    }
    return;
}

void UpgAgentHandler::UpgPossible(void) {
    UPG_LOG_DEBUG("Upgrade Possible!!!!");
    return;
}

void UpgAgentHandler::UpgAborted(vector<string> &errStrList) {
    UPG_LOG_DEBUG("Upgrade aborted!!!!");
    for (uint i=0; i<errStrList.size(); i++) {
        UPG_LOG_DEBUG("Application failed response: {}", errStrList[i]);
    }
    return;
}

} // namespace upgrade
