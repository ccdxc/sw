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

    /*
     * All the following APIs are invoked for agents.
     * The APIs are invoked on a per-application basis after the application has
     *     returned back to upgrade-manager with a success/fail
     */
    virtual void UpgStateCompatCheckCompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateProcessQuiesceCompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStatePostBinRestartCompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimePhase1CompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimePhase2CompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimePhase3CompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimePhase4CompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateCleanupCompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateAbortedCompletionHandler(HdlrResp &resp, string svcName);

    /*
     * All the following APIs are invoked for agents.
     * These are APIs called at the end of a successful/failed/aborted upgrade processing
     */
    virtual void UpgSuccessful(void);
    virtual void UpgFailed(vector<string> &errStrList);
    virtual void UpgNotPossible(vector<string> &errStrList);
    virtual void UpgPossible(void);
    virtual void UpgAborted(vector<string> &errStrList);
};
typedef std::shared_ptr<UpgAgentHandler> UpgAgentHandlerPtr;

} // namespace upgrade

#endif // __UPGRAGE_AGENT_HANDLER_H__
