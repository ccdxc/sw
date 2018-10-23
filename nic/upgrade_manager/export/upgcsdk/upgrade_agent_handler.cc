// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_agent_handler.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

void UpgAgentHandler::CompatCheckCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Pre Upgrade Check completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::ProcessQuiesceCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Process quiesce completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::LinkDownCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Link down completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::LinkUpCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Link up completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::PostRestartCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Post Restart completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::DataplaneDowntimePhase1CompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::DataplaneDowntimePhase2CompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::DataplaneDowntimePhase3CompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::DataplaneDowntimePhase4CompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Dataplane downtime completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::AbortedCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Aborted completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::HostDownCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Host Down completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::HostUpCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Host Up completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::PostHostDownCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Post-Host Down completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::PostLinkUpCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Post-Link Up completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::SaveStateCompletionHandler(HdlrResp &resp, string svcName) {
    //UPG_LOG_DEBUG("Save State completed with status {} error {} for service {}", resp.resp, resp.errStr, svcName);
    return;
}

void UpgAgentHandler::UpgSuccessful(void) {
    //UPG_LOG_DEBUG("Upgrade completed with status successfully!!!!");
    return;
}

void UpgAgentHandler::UpgNotPossible(UpgCtx &ctx, vector<string> &errStrList) {
    //UPG_LOG_DEBUG("Upgrade Not Possible!!!!");
    for (uint i=0; i<errStrList.size(); i++) {
        //UPG_LOG_DEBUG("Application failed response: {}", errStrList[i]);
    }
    return;
}

void UpgAgentHandler::UpgPossible(UpgCtx &ctx) {
    //UPG_LOG_DEBUG("Upgrade Possible!!!!");
    return;
}

void UpgAgentHandler::UpgFailed(vector<string> &errStrList) {
    //UPG_LOG_DEBUG("Upgrade failed!!!!");
    for (uint i=0; i<errStrList.size(); i++) {
        //UPG_LOG_DEBUG("Application failed response: {}", errStrList[i]);
    }
}

void UpgAgentHandler::UpgAborted(vector<string> &errStrList) {
    //UPG_LOG_DEBUG("Upgrade aborted!!!!");
    for (uint i=0; i<errStrList.size(); i++) {
        //UPG_LOG_DEBUG("Application failed response: {}", errStrList[i]);
    }
    return;
}

} // namespace upgrade
