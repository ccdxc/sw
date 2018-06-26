// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_HANDLER_H__
#define __UPGRAGE_HANDLER_H__

#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"

namespace upgrade {

using namespace std;

class UpgHandler {
public:
    UpgHandler() {}
    virtual HdlrResp HandleStateUpgReqRcvd(UpgCtx& upgCtx);
    virtual HdlrResp HandleStatePreUpgState(UpgCtx& upgCtx);
    virtual HdlrResp HandleStatePostBinRestart(UpgCtx& upgCtx);
    virtual HdlrResp HandleStateProcessesQuiesced(UpgCtx& upgCtx);
    virtual HdlrResp HandleStateDataplaneDowntimePhase1Start(UpgCtx& upgCtx);
    virtual HdlrResp HandleDataplaneDowntimeAdminQ(UpgCtx& upgCtx);
    virtual HdlrResp HandleStateDataplaneDowntimePhase2Start(UpgCtx& upgCtx);
    virtual HdlrResp HandleStateCleanup(UpgCtx& upgCtx);
    virtual void HandleStateUpgSuccess(UpgCtx& upgCtx);
    virtual void HandleStateUpgFailed(UpgCtx& upgCtx);
    virtual void HandleStateUpgAborted(UpgCtx& upgCtx);
};
typedef std::shared_ptr<UpgHandler> UpgHandlerPtr;

} // namespace upgrade

#endif // __UPGRAGE_HANDLER_H__
