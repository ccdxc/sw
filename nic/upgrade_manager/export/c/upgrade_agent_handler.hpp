// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_AGENT_HANDLER_H__
#define __UPGRAGE_AGENT_HANDLER_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "upgrade_handler.hpp"

namespace upgrade {

using namespace std;

class UpgAgentHandler {
public:
    UpgAgentHandler() {}

    virtual void UpgStatePreUpgCheckComplete(HdlrResp &resp, string svcName);
    virtual void UpgStateProcessQuiesceComplete(HdlrResp &resp, string svcName);
    virtual void UpgStatePostBinRestartComplete(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimePhase1Complete(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimeAdminQComplete(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimePhase2Complete(HdlrResp &resp, string svcName);
    virtual void UpgStateCleanupComplete(HdlrResp &resp, string svcName);

    virtual void UpgSuccessful(void);
    virtual void UpgFailed(vector<string> &errStrList);

};
typedef std::shared_ptr<UpgAgentHandler> UpgAgentHandlerPtr;

} // namespace upgrade

#endif // __UPGRAGE_AGENT_HANDLER_H__
