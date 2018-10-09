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
    virtual void CompatCheckCompletionHandler(HdlrResp &resp, string svcName);
    virtual void ProcessQuiesceCompletionHandler(HdlrResp &resp, string svcName);
    virtual void LinkDownCompletionHandler(HdlrResp &resp, string svcName);
    virtual void LinkUpCompletionHandler(HdlrResp &resp, string svcName);
    virtual void PostRestartCompletionHandler(HdlrResp &resp, string svcName);
    virtual void DataplaneDowntimePhase1CompletionHandler(HdlrResp &resp, string svcName);
    virtual void DataplaneDowntimePhase2CompletionHandler(HdlrResp &resp, string svcName);
    virtual void DataplaneDowntimePhase3CompletionHandler(HdlrResp &resp, string svcName);
    virtual void DataplaneDowntimePhase4CompletionHandler(HdlrResp &resp, string svcName);
    virtual void AbortedCompletionHandler(HdlrResp &resp, string svcName);
    virtual void HostDownCompletionHandler(HdlrResp &resp, string svcName);
    virtual void HostUpCompletionHandler(HdlrResp &resp, string svcName);
    virtual void PostHostDownCompletionHandler(HdlrResp &resp, string svcName);
    virtual void PostLinkUpCompletionHandler(HdlrResp &resp, string svcName);
    virtual void SaveStateCompletionHandler(HdlrResp &resp, string svcName);

    /*
     * All the following APIs are invoked for agents.
     * These are APIs called at the end of a successful/failed/aborted upgrade processing
     */
    virtual void UpgSuccessful(void);
    virtual void UpgFailed(vector<string> &errStrList);
    virtual void UpgNotPossible(UpgCtx &ctx, vector<string> &errStrList);
    virtual void UpgPossible(UpgCtx &ctx);
    virtual void UpgAborted(vector<string> &errStrList);
};
typedef std::shared_ptr<UpgAgentHandler> UpgAgentHandlerPtr;

} // namespace upgrade

#endif // __UPGRAGE_AGENT_HANDLER_H__
