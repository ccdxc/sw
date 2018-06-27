#include <stdio.h>
#include <iostream>

#include "upgrade_state_machine.hpp"
#include "upgrade_pre_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

UpgStateMachine StateMachine[UpgStateTerminal];
UpgPreStateHandler* preStateHandlers;

UpgReqStateType UpgRespStateTypeToUpgReqStateType(UpgRespStateType type) {
    switch(type) {
    case UpgReqRcvdPass:
    case UpgReqRcvdFail: 
        return UpgReqRcvd;
    case PreUpgStatePass:
    case PreUpgStateFail:
        return PreUpgState;
    case ProcessesQuiescedPass:
    case ProcessesQuiescedFail:
        return ProcessesQuiesced;
    case PostBinRestartPass:
    case PostBinRestartFail:
        return PostBinRestart;
    case DataplaneDowntimePhase1StartPass:
    case DataplaneDowntimePhase1StartFail:
        return DataplaneDowntimePhase1Start;
    case DataplaneDowntimeAdminQHandlingPass:
    case DataplaneDowntimeAdminQHandlingFail:
        return DataplaneDowntimeAdminQHandling;
    case DataplaneDowntimePhase2StartPass:
    case DataplaneDowntimePhase2StartFail:
        return DataplaneDowntimePhase2Start;
    case CleanupPass:
    case CleanupFail:
        return Cleanup;
    case UpgSuccessPass:
    case UpgSuccessFail:
        return UpgSuccess;
    case UpgFailedPass:
    case UpgFailedFail:
        return UpgFailed;
    case UpgAbortedPass:
    case UpgAbortedFail:
        return UpgAborted;
    default:
        LogInfo("Should never come here");
        return UpgStateTerminal;
    }
}

string UpgRespValPassStr(UpgReqStateType req) {
    return StateMachine[req].upgAppRespValToStrPass;
}

string UpgRespValFailStr(UpgReqStateType req) {
    return StateMachine[req].upgAppRespValToStrFail;
}

string UpgRespStatePassStr(UpgReqStateType req) {
    return StateMachine[req].upgRespStateTypeToStrPass;
}

string UpgRespStateFailStr(UpgReqStateType req) {
    return StateMachine[req].upgRespStateTypeToStrFail;
}

bool UpgRespStatePassType(UpgRespStateType type) {
    switch (type) {
    case UpgReqRcvdPass:
    case PreUpgStatePass:
    case ProcessesQuiescedPass:
    case PostBinRestartPass:
    case DataplaneDowntimePhase1StartPass:
    case DataplaneDowntimeAdminQHandlingPass:
    case DataplaneDowntimePhase2StartPass:
    case CleanupPass:
    case UpgSuccessPass:
    case UpgFailedPass:
    case UpgAbortedPass:
        return true;
    default:
        LogInfo("Got failed UpgRespStatePassType {}", type);
        return false;
    }
}

string GetAppRespStrUtil(UpgRespStateType type) {
    bool ret = UpgRespStatePassType(type);
    UpgReqStateType req = UpgRespStateTypeToUpgReqStateType(type);
    return (ret?UpgRespStatePassStr(req):UpgRespStateFailStr(req));
}

string GetUpgAppRespValToStr(UpgRespStateType type) {
    bool ret = UpgRespStatePassType(type);
    UpgReqStateType req = UpgRespStateTypeToUpgReqStateType(type);
    return (ret?UpgRespValPassStr(req):UpgRespValPassStr(req));
}

void InitStateMachineVector(void) {
    LogInfo("InitStateMachineVector called!!!");
    preStateHandlers = new UpgPreStateHandler();
    StateMachine[UpgReqRcvd] = 
               {
                UpgReqRcvd, 
                PreUpgState, 
                UpgReqRcvdPass, 
                UpgReqRcvdFail, 
                "Sending pass to upg-mgr for Upgrade Request Received message", 
                "Sending fail to upg-mgr for Upgrade Request Received message",
                "Upgrade Request Received", 
                "Upgrade Request Received Pass", 
                "Upgrade Request Received Fail",
                &UpgPreStateHandler::PreUpgReqRcvd
               };
    StateMachine[PreUpgState] = 
               {
                PreUpgState, 
                ProcessesQuiesced, 
                PreUpgStatePass, 
                PreUpgStateFail, 
                "Sending pass to upg-mgr for Pre-Upgrade Check message", 
                "Sending fail to upg-mgr for Pre-Upgrade Check message", 
                "Perform Compat Check", 
                "Compat check passed", 
                "Compat check failed",
                &UpgPreStateHandler::PrePreUpgState
               };
    StateMachine[ProcessesQuiesced] = 
               {
                ProcessesQuiesced, 
                PostBinRestart, 
                ProcessesQuiescedPass, 
                ProcessesQuiescedFail, 
                "Sending pass to upg-mgr for Process Quiesced message", 
                "Sending fail to upg-mgr for Process Quiesced message", 
                "Quiesce Processes Pre-Restart", 
                "Process Quiesce Pass", 
                "Process Quiesce Fail",
                &UpgPreStateHandler::PreProcessesQuiesced
               };
    StateMachine[PostBinRestart] = 
               {
                PostBinRestart, 
                DataplaneDowntimePhase1Start, 
                PostBinRestartPass, 
                PostBinRestartFail, 
                "Sending pass to upg-mgr for Post-Binary Restart message", 
                "Sending fail to upg-mgr for Post-Binary Restart message", 
                "Post Process Restart", 
                "Post Process Restart Pass", 
                "Post Process Restart Fail",
                &UpgPreStateHandler::PrePostBinRestart
               };
    StateMachine[DataplaneDowntimePhase1Start] =
               {
                DataplaneDowntimePhase1Start,
                DataplaneDowntimeAdminQHandling,
                DataplaneDowntimePhase1StartPass,
                DataplaneDowntimePhase1StartFail,
                "Sending pass to upg-mgr for Dataplane Downtime Phase1 Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Phase1 Start message",
                "Dataplane Downtime Phase1 Start",
                "Dataplane Downtime Phase1 Success",
                "Dataplane Downtime Phase1 Fail",
                &UpgPreStateHandler::PreDataplaneDowntimePhase1Start
               };
    StateMachine[DataplaneDowntimeAdminQHandling] = 
               {
                DataplaneDowntimeAdminQHandling,
                DataplaneDowntimePhase2Start, 
                DataplaneDowntimeAdminQHandlingPass, 
                DataplaneDowntimeAdminQHandlingFail, 
                "Sending pass to upg-mgr for Dataplane Downtime AdminQ Handling message", 
                "Sending fail to upg-mgr for Dataplane Downtime AdminQ Handling message", 
                "Dataplane Downtime AdminQ Handling Start", 
                "Dataplane Downtime AdminQ Handling Success", 
                "Dataplane Downtime AdminQ Handling Fail",
                &UpgPreStateHandler::PreDataplaneDowntimeAdminQ
               };
    StateMachine[DataplaneDowntimePhase2Start] =
               {
                DataplaneDowntimePhase2Start,
                UpgSuccess,
                DataplaneDowntimePhase2StartPass,
                DataplaneDowntimePhase2StartFail,
                "Sending pass to upg-mgr for Dataplane Downtime Phase2 Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Phase2 Start message",
                "Dataplane Downtime Phase2 Start",
                "Dataplane Downtime Phase2 Success",
                "Dataplane Downtime Phase2 Fail",
                &UpgPreStateHandler::PreDataplaneDowntimePhase2Start
               };
    StateMachine[UpgSuccess] = 
               {
                UpgSuccess, 
                Cleanup, 
                UpgSuccessPass, 
                UpgSuccessFail, 
                "Sending pass to upg-mgr after upgrade success message", 
                "Sending fail to upg-mgr after upgrade fail message", 
                "Upgrade Success", 
                "", 
                "",
                &UpgPreStateHandler::PreUpgSuccess
               };
    StateMachine[UpgFailed] = 
               {
                UpgFailed, 
                Cleanup, 
                UpgFailedPass, 
                UpgFailedFail, 
                "Sending pass to upg-mgr after upgrade fail message", 
                "Sending fail to upg-mgr after upgrade fail message", 
                "Upgrade Fail", 
                "", 
                "",
                &UpgPreStateHandler::PreUpgFailed
                };
    StateMachine[Cleanup] =
               {
                Cleanup,
                UpgStateTerminal,
                CleanupPass,
                CleanupFail,
                "Sending pass to upg-mgr after cleaning up stale state",
                "Sending fail to upg-mgr after cleaning up stale state",
                "Cleanup State",
                "Cleanup Pass",
                "Cleanup Fail",
                &UpgPreStateHandler::PreCleanup
               };
    StateMachine[UpgAborted] = 
               {
                UpgAborted, 
                UpgStateTerminal, 
                UpgAbortedPass, 
                UpgAbortedFail, 
                "Sending pass to upg-mgr after handling upgrade aborted",
                "Sending fail to upg-mgr after handling upgrade aborted", 
                "Upgrade Aborted State", 
                "Upgrade Aborted Pass", 
                "Upgrade Aborted Fail",
                &UpgPreStateHandler::PreUpgAborted
               };
}
}
