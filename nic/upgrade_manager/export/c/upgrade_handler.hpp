// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_HANDLER_H__
#define __UPGRAGE_HANDLER_H__

#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"

namespace upgrade {

using namespace std;

class UpgHandler {
public:
    UpgHandler() {}
    virtual HdlrResp HandleUpgStateCompatCheck(UpgCtx& upgCtx);
    virtual HdlrResp HandleUpgStatePostBinRestart(UpgCtx& upgCtx);
    virtual HdlrResp HandleUpgStateProcessQuiesce(UpgCtx& upgCtx);
    virtual HdlrResp HandleUpgStateDataplaneDowntimePhase1(UpgCtx& upgCtx);
    virtual HdlrResp HandleUpgStateDataplaneDowntimePhase2(UpgCtx& upgCtx);
    virtual HdlrResp HandleUpgStateDataplaneDowntimePhase3(UpgCtx& upgCtx);
    virtual HdlrResp HandleUpgStateDataplaneDowntimePhase4(UpgCtx& upgCtx);
    virtual HdlrResp HandleUpgStateCleanup(UpgCtx& upgCtx);
    virtual void HandleUpgStateSuccess(UpgCtx& upgCtx);
    virtual void HandleUpgStateFailed(UpgCtx& upgCtx);
    virtual void HandleUpgStateAbort(UpgCtx& upgCtx);
};
typedef std::shared_ptr<UpgHandler> UpgHandlerPtr;

} // namespace upgrade

#endif // __UPGRAGE_HANDLER_H__
