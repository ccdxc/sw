// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_agent_handler.hpp"

namespace upgrade {

using namespace std;

void UpgAgentHandler::UpgStatePreUpgCheckComplete(string status, string name) {
    LogInfo("Pre Upgrade Check completed {} for service {}", status, name);
    return;
}

void UpgAgentHandler::UpgStateProcessQuiesceComplete(string status, string name) {
    LogInfo("Process quiesce completed {} for service {}", status, name);
    return;
}

void UpgAgentHandler::UpgStatePostBinRestartComplete(string status, string name) {
    LogInfo("Post Bin Restart completed {} for service {}", status, name);
    return;
}

void UpgAgentHandler::UpgStateDataplaceDowntimeComplete(string status, string name) {
    LogInfo("Dataplane downtime completed {} for service {}", status, name);
    return;
}

void UpgAgentHandler::UpgStateCleanupComplete(string status, string name) {
    LogInfo("Cleanup completed {} for service {}", status, name);
    return;
}

} // namespace upgrade
