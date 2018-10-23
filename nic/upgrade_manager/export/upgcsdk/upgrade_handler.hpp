// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_HANDLER_H__
#define __UPGRAGE_HANDLER_H__

#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"

namespace upgrade {

using namespace std;

typedef enum {
    SUCCESS,
    FAIL,
    INPROGRESS
} HdlrRespCode;

typedef struct HdlrResp_ {
    HdlrRespCode     resp;
    string           errStr;
    HdlrResp_() {}
    HdlrResp_(HdlrRespCode code, string error) {
        resp = code;
        errStr = error;
    }
} HdlrResp;

class UpgHandler {
public:
    UpgHandler() {}
    /*
     * Applications are expected to perform compat check 
     *     and let Upgrade manager know if Upgrade is allowed or not
     * (aka. PreUpgradeHandler()
     */
    virtual HdlrResp CompatCheckHandler(UpgCtx& upgCtx);
    /*
     * Applications are expected to perform any quiescing needed in control plane.
     * Here, NMD-like agents are expected to stop accepting 
     *     any configurations from their north-bound agents
     */
    virtual HdlrResp ProcessQuiesceHandler(UpgCtx& upgCtx);
    /*
     * LinkMgr is expected to bring link-down for disruptive upgrade
     */
    virtual HdlrResp LinkDownHandler(UpgCtx& upgCtx);
    /*
     * Applications are expected to do any post-binary restart handling here.
     */
    virtual HdlrResp PostRestartHandler(UpgCtx& upgCtx);
    /*
     * At this stage, following is expected:
     * 1. FTE is supposed to disable flow-miss
     * 2. Dataplane is quiesced
     */
    virtual HdlrResp DataplaneDowntimePhase1Handler(UpgCtx& upgCtx);
    /*
     * At this stage, following is expected:
     * 1. AdminQ comes up after transforming its relevant data to new format
     */
    virtual HdlrResp DataplaneDowntimePhase2Handler(UpgCtx& upgCtx);
    /*
     * At this stage, following is expected:
     * 1. All table data transformed to new format
     * 2. All table base addresses fixed up
     * 3. Programs in pipeline are ready to go
     */
    virtual HdlrResp DataplaneDowntimePhase3Handler(UpgCtx& upgCtx);
    /*
     * At this stage, following is expected:
     * 1. Dataplane is re-enabled
     * 2. FTE flow-miss re-enabled
     */
    virtual HdlrResp DataplaneDowntimePhase4Handler(UpgCtx& upgCtx);
    /*
     * During this phase
     * 1. link down is propagated to devices
     * 2. inidicate down to pipeline by disabling queue state
     * 3. await for pipeline until it is complete
     * 4. HOST_DOWN event is raised for rest of the system
     */
    virtual HdlrResp HostDownHandler(UpgCtx& upgCtx);
    /*
     * During this phase scheduler should be shut down
     */
    virtual HdlrResp PostHostDownHandler(UpgCtx& upgCtx);
    /*
     * During this phase apps are supposed to save their state, if any
     */
    virtual HdlrResp SaveStateHandler(UpgCtx& upgCtx);
    /*
     * During this phase
     * 1. device queues need to be enabled
     * 2. HOST_UP event is raised
     */
    virtual HdlrResp HostUpHandler(UpgCtx& upgCtx);
    /*
     * At this stage, links are brought up including:
     * 1. enabling scheduler
     * 2. bringing up links
     * 3. sending link up notifications
     */
    virtual HdlrResp LinkUpHandler(UpgCtx& upgCtx);
    /*
     * During this phase DEVICE_UP is notified to the host side (driver)
     */
    virtual HdlrResp PostLinkUpHandler(UpgCtx& upgCtx);
    /*
     * Handle upgrade success
     */
    virtual void SuccessHandler(UpgCtx& upgCtx);
     /*
      * Handle upgrade failed
      */
    virtual void FailedHandler(UpgCtx& upgCtx);
    /*
     * Handle upgrade aborted
     */
    virtual void AbortHandler(UpgCtx& upgCtx);
};
typedef std::shared_ptr<UpgHandler> UpgHandlerPtr;

} // namespace upgrade

#endif // __UPGRAGE_HANDLER_H__
