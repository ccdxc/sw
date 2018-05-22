// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_AGENT_HANDLER_H__
#define __UPGRAGE_AGENT_HANDLER_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

class UpgAgentHandler {
public:
    UpgAgentHandler() {}

    virtual void UpgStatePreUpgCheckComplete(string status, string name);
    virtual void UpgStateProcessQuiesceComplete(string status, string name);
    virtual void UpgStatePostBinRestartComplete(string status, string name);
    virtual void UpgStateDataplaceDowntimeComplete(string status, string name);
    virtual void UpgStateCleanupComplete(string status, string name);

};
typedef std::shared_ptr<UpgAgentHandler> UpgAgentHandlerPtr;

} // namespace upgrade

#endif // __UPGRAGE_AGENT_HANDLER_H__
