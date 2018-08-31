// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_HANDLER_H__
#define __UPGRAGE_HANDLER_H__

#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"

namespace upgrade {

using namespace std;

class UpgHandler {
public:
    UpgHandler() {}
    /*
     * Applications are expected to perform compat check 
     *     and let Upgrade manager know if Upgrade is allowed or not
     */
    virtual HdlrResp HandleUpgStateCompatCheck(UpgCtx& upgCtx);
    /*
     * Applications are expected to perform any quiescing needed in control plane.
     * Here, NMD-like agents are expected to stop accepting 
     *     any configurations from their north-bound agents
     */
    virtual HdlrResp HandleUpgStateProcessQuiesce(UpgCtx& upgCtx);
    /*
     * LinkMgr is expected to bring link-down for disruptive upgrade
     */
    virtual HdlrResp HandleUpgStateLinkDown(UpgCtx& upgCtx);
    /*
     * LinkMgr is expected to bring link-up for disruptive upgrade
     */
    virtual HdlrResp HandleUpgStateLinkUp(UpgCtx& upgCtx);
    /*
     * Applications are expected to do any post-binary restart handling here.
     */
    virtual HdlrResp HandleUpgStatePostBinRestart(UpgCtx& upgCtx);
    /*
     * At this stage, for disruptive upgrade,
     *   NicMgr and HAL together are expected to bring the dataplane down
     */
    virtual HdlrResp HandleUpgStateDataplaneDowntimeStart(UpgCtx& upgCtx);
    /*
     * At this stage, for disruptive upgrade,
     *   NicMgr and HAL together are expected to do post-link-up processing
     */
    virtual HdlrResp HandleUpgStateIsSystemReady(UpgCtx& upgCtx);
    /*
     * At this stage, following is expected:
     * 1. FTE is supposed to disable flow-miss
     * 2. Dataplane is quiesced
     */
    virtual HdlrResp HandleUpgStateDataplaneDowntimePhase1(UpgCtx& upgCtx);
    /*
     * At this stage, following is expected:
     * 1. AdminQ comes up after transforming its relevant data to new format
     */
    virtual HdlrResp HandleUpgStateDataplaneDowntimePhase2(UpgCtx& upgCtx);
    /*
     * At this stage, following is expected:
     * 1. All table data transformed to new format
     * 2. All table base addresses fixed up
     * 3. Programs in pipeline are ready to go
     */
    virtual HdlrResp HandleUpgStateDataplaneDowntimePhase3(UpgCtx& upgCtx);
    /*
     * At this stage, following is expected:
     * 1. Dataplane is re-enabled
     * 2. FTE flow-miss re-enabled
     */
    virtual HdlrResp HandleUpgStateDataplaneDowntimePhase4(UpgCtx& upgCtx);
    /*
     * At this stage, following is expected:
     * 1. Cleanup any state needed
     */
    virtual HdlrResp HandleUpgStateCleanup(UpgCtx& upgCtx);
    /*
     * Handle upgrade success
     */
    virtual void HandleUpgStateSuccess(UpgCtx& upgCtx);
    /*
     * Handle upgrade failed
     */
    virtual void HandleUpgStateFailed(UpgCtx& upgCtx);
    /*
     * Handle upgrade aborted
     */
    virtual void HandleUpgStateAbort(UpgCtx& upgCtx);
};
typedef std::shared_ptr<UpgHandler> UpgHandlerPtr;

} // namespace upgrade

#endif // __UPGRAGE_HANDLER_H__
