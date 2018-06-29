#include <stdio.h>
#include <iostream>

#include "upgrade_state_machine.hpp"
#include "upgrade_pre_state_handlers.hpp"
#include "upgrade_post_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace upgrade {

using namespace std;

UpgStateMachine StateMachine[UpgStateTerminal];
UpgPreStateHandler* preStateHandlers;
UpgPostStateHandler* postStateHandlers;

UpgReqStateType UpgStateRespTypeToUpgReqStateType(UpgStateRespType type) {
    switch(type) {
    case UpgStateCompatCheckRespPass:
    case UpgStateCompatCheckRespFail:
        return UpgStateCompatCheck;
    case UpgStateProcessQuiesceRespPass:
    case UpgStateProcessQuiesceRespFail:
        return UpgStateProcessQuiesce;
    case UpgStatePostBinRestartRespPass:
    case UpgStatePostBinRestartRespFail:
        return UpgStatePostBinRestart;
    case UpgStateDataplaneDowntimePhase1RespPass:
    case UpgStateDataplaneDowntimePhase1RespFail:
        return UpgStateDataplaneDowntimePhase1;
    case UpgStateDataplaneDowntimePhase2RespPass:
    case UpgStateDataplaneDowntimePhase2RespFail:
        return UpgStateDataplaneDowntimePhase2;
    case UpgStateDataplaneDowntimePhase3RespPass:
    case UpgStateDataplaneDowntimePhase3RespFail:
        return UpgStateDataplaneDowntimePhase3;
    case UpgStateDataplaneDowntimePhase4RespPass:
    case UpgStateDataplaneDowntimePhase4RespFail:
        return UpgStateDataplaneDowntimePhase4;
    case UpgStateCleanupRespPass:
    case UpgStateCleanupRespFail:
        return UpgStateCleanup;
    case UpgStateSuccessRespPass:
    case UpgStateSuccessRespFail:
        return UpgStateSuccess;
    case UpgStateFailedRespPass:
    case UpgStateFailedRespFail:
        return UpgStateFailed;
    case UpgStateAbortRespPass:
    case UpgStateAbortRespFail:
        return UpgStateAbort;
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

bool UpgRespStatePassType(UpgStateRespType type) {
    switch (type) {
    case UpgStateCompatCheckRespPass:
    case UpgStateProcessQuiesceRespPass:
    case UpgStatePostBinRestartRespPass:
    case UpgStateDataplaneDowntimePhase1RespPass:
    case UpgStateDataplaneDowntimePhase2RespPass:
    case UpgStateDataplaneDowntimePhase3RespPass:
    case UpgStateDataplaneDowntimePhase4RespPass:
    case UpgStateCleanupRespPass:
    case UpgStateSuccessRespPass:
    case UpgStateFailedRespPass:
    case UpgStateAbortRespPass:
        return true;
    default:
        LogInfo("Got failed UpgRespStatePassType {}", type);
        return false;
    }
}

string GetAppRespStrUtil(UpgStateRespType type) {
    bool ret = UpgRespStatePassType(type);
    UpgReqStateType req = UpgStateRespTypeToUpgReqStateType(type);
    return (ret?UpgRespStatePassStr(req):UpgRespStateFailStr(req));
}

string GetUpgAppRespValToStr(UpgStateRespType type) {
    bool ret = UpgRespStatePassType(type);
    UpgReqStateType req = UpgStateRespTypeToUpgReqStateType(type);
    return (ret?UpgRespValPassStr(req):UpgRespValPassStr(req));
}

void InitStateMachineVector(void) {
    LogInfo("InitStateMachineVector called!!!");
    preStateHandlers = new UpgPreStateHandler();
    postStateHandlers = new UpgPostStateHandler();
    StateMachine[UpgStateCompatCheck] = 
               {
                UpgStateCompatCheck, 
                UpgStateProcessQuiesce, 
                UpgStateCompatCheckRespPass, 
                UpgStateCompatCheckRespFail, 
                "Sending pass to upg-mgr for Pre-Upgrade Check message", 
                "Sending fail to upg-mgr for Pre-Upgrade Check message", 
                "Perform Compat Check", 
                "Compat check passed", 
                "Compat check failed",
                &UpgPreStateHandler::PrePreUpgState,
                &UpgPostStateHandler::PostPreUpgState
               };
    StateMachine[UpgStateProcessQuiesce] = 
               {
                UpgStateProcessQuiesce, 
                UpgStatePostBinRestart,
                UpgStateProcessQuiesceRespPass, 
                UpgStateProcessQuiesceRespFail, 
                "Sending pass to upg-mgr for Process Quiesced message", 
                "Sending fail to upg-mgr for Process Quiesced message", 
                "Quiesce Processes Pre-Restart", 
                "Process Quiesce Pass", 
                "Process Quiesce Fail",
                &UpgPreStateHandler::PreProcessesQuiesced,
                &UpgPostStateHandler::PostProcessesQuiesced
               };
    StateMachine[UpgStatePostBinRestart] = 
               {
                UpgStatePostBinRestart, 
                UpgStateDataplaneDowntimePhase1, 
                UpgStatePostBinRestartRespPass, 
                UpgStatePostBinRestartRespFail, 
                "Sending pass to upg-mgr for Post-Binary Restart message", 
                "Sending fail to upg-mgr for Post-Binary Restart message", 
                "Post Process Restart", 
                "Post Process Restart Pass", 
                "Post Process Restart Fail",
                &UpgPreStateHandler::PrePostBinRestart,
                &UpgPostStateHandler::PostPostBinRestart
               };
    StateMachine[UpgStateDataplaneDowntimePhase1] =
               {
                UpgStateDataplaneDowntimePhase1,
                UpgStateDataplaneDowntimePhase2,
                UpgStateDataplaneDowntimePhase1RespPass,
                UpgStateDataplaneDowntimePhase1RespFail,
                "Sending pass to upg-mgr for Dataplane Downtime Phase1 Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Phase1 Start message",
                "Dataplane Downtime Phase1 Start",
                "Dataplane Downtime Phase1 Success",
                "Dataplane Downtime Phase1 Fail",
                &UpgPreStateHandler::PreDataplaneDowntimePhase1Start,
                &UpgPostStateHandler::PostDataplaneDowntimePhase1Start
               };
    StateMachine[UpgStateDataplaneDowntimePhase2] =
               {
                UpgStateDataplaneDowntimePhase2,
                UpgStateDataplaneDowntimePhase3,
                UpgStateDataplaneDowntimePhase2RespPass,
                UpgStateDataplaneDowntimePhase2RespFail,
                "Sending pass to upg-mgr for Dataplane Downtime Phase2 Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Phase2 Start message",
                "Dataplane Downtime Phase2 Start",
                "Dataplane Downtime Phase2 Success",
                "Dataplane Downtime Phase2 Fail",
                &UpgPreStateHandler::PreDataplaneDowntimePhase2Start,
                &UpgPostStateHandler::PostDataplaneDowntimePhase2Start
               };
    StateMachine[UpgStateDataplaneDowntimePhase3] =
               {
                UpgStateDataplaneDowntimePhase3,
                UpgStateDataplaneDowntimePhase4,
                UpgStateDataplaneDowntimePhase3RespPass,
                UpgStateDataplaneDowntimePhase3RespFail,
                "Sending pass to upg-mgr for Dataplane Downtime Phase3 Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Phase3 Start message",
                "Dataplane Downtime Phase3 Start",
                "Dataplane Downtime Phase3 Success",
                "Dataplane Downtime Phase3 Fail",
                &UpgPreStateHandler::PreDataplaneDowntimePhase3Start,
                &UpgPostStateHandler::PostDataplaneDowntimePhase3Start
               };
    StateMachine[UpgStateDataplaneDowntimePhase4] =
               {
                UpgStateDataplaneDowntimePhase4,
                UpgStateSuccess,
                UpgStateDataplaneDowntimePhase4RespPass,
                UpgStateDataplaneDowntimePhase4RespFail,
                "Sending pass to upg-mgr for Dataplane Downtime Phase4 Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Phase4 Start message",
                "Dataplane Downtime Phase4 Start",
                "Dataplane Downtime Phase4 Success",
                "Dataplane Downtime Phase4 Fail",
                &UpgPreStateHandler::PreDataplaneDowntimePhase4Start,
                &UpgPostStateHandler::PostDataplaneDowntimePhase4Start
               };
    StateMachine[UpgStateSuccess] = 
               {
                UpgStateSuccess, 
                UpgStateCleanup, 
                UpgStateSuccessRespPass, 
                UpgStateSuccessRespFail, 
                "Sending pass to upg-mgr after upgrade success message", 
                "Sending fail to upg-mgr after upgrade fail message", 
                "Upgrade Success", 
                "", 
                "",
                &UpgPreStateHandler::PreUpgSuccess,
                &UpgPostStateHandler::PostUpgSuccess
               };
    StateMachine[UpgStateFailed] = 
               {
                UpgStateFailed, 
                UpgStateCleanup, 
                UpgStateFailedRespPass, 
                UpgStateFailedRespFail, 
                "Sending pass to upg-mgr after upgrade fail message", 
                "Sending fail to upg-mgr after upgrade fail message", 
                "Upgrade Fail", 
                "", 
                "",
                &UpgPreStateHandler::PreUpgFailed,
                &UpgPostStateHandler::PostUpgFailed
                };
    StateMachine[UpgStateCleanup] =
               {
                UpgStateCleanup,
                UpgStateTerminal,
                UpgStateCleanupRespPass,
                UpgStateCleanupRespPass,
                "Sending pass to upg-mgr after cleaning up stale state",
                "Sending fail to upg-mgr after cleaning up stale state",
                "Cleanup State",
                "Cleanup Pass",
                "Cleanup Fail",
                &UpgPreStateHandler::PreCleanup,
                &UpgPostStateHandler::PostCleanup
               };
    StateMachine[UpgStateAbort] = 
               {
                UpgStateAbort, 
                UpgStateTerminal, 
                UpgStateAbortRespPass, 
                UpgStateAbortRespPass, 
                "Sending pass to upg-mgr after handling upgrade aborted",
                "Sending fail to upg-mgr after handling upgrade aborted", 
                "Upgrade Aborted State", 
                "Upgrade Aborted Pass", 
                "Upgrade Aborted Fail",
                &UpgPreStateHandler::PreUpgAborted,
                &UpgPostStateHandler::PostUpgAborted
               };
}
}
