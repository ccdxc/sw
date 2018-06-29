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

    virtual void UpgStateCompatCheckCompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateProcessQuiesceCompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStatePostBinRestartCompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimePhase1CompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimePhase2CompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimePhase3CompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateDataplaneDowntimePhase4CompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateCleanupCompletionHandler(HdlrResp &resp, string svcName);
    virtual void UpgStateAbortedCompletionHandler(HdlrResp &resp, string svcName);

    virtual void UpgSuccessful(void);
    virtual void UpgFailed(vector<string> &errStrList);
    virtual void UpgAborted(vector<string> &errStrList);
};
typedef std::shared_ptr<UpgAgentHandler> UpgAgentHandlerPtr;

} // namespace upgrade

#endif // __UPGRAGE_AGENT_HANDLER_H__
