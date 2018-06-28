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

UpgReqStateType UpgRespStateTypeToUpgReqStateType(UpgRespStateType type) {
    switch(type) {
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
    case DataplaneDowntimePhase2StartPass:
    case DataplaneDowntimePhase2StartFail:
        return DataplaneDowntimePhase2Start;
    case DataplaneDowntimePhase3StartPass:
    case DataplaneDowntimePhase3StartFail:
        return DataplaneDowntimePhase3Start;
    case DataplaneDowntimePhase4StartPass:
    case DataplaneDowntimePhase4StartFail:
        return DataplaneDowntimePhase4Start;
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
    case PreUpgStatePass:
    case ProcessesQuiescedPass:
    case PostBinRestartPass:
    case DataplaneDowntimePhase1StartPass:
    case DataplaneDowntimePhase2StartPass:
    case DataplaneDowntimePhase3StartPass:
    case DataplaneDowntimePhase4StartPass:
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
    postStateHandlers = new UpgPostStateHandler();
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
                &UpgPreStateHandler::PrePreUpgState,
                &UpgPostStateHandler::PostPreUpgState
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
                &UpgPreStateHandler::PreProcessesQuiesced,
                &UpgPostStateHandler::PostProcessesQuiesced
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
                &UpgPreStateHandler::PrePostBinRestart,
                &UpgPostStateHandler::PostPostBinRestart
               };
    StateMachine[DataplaneDowntimePhase1Start] =
               {
                DataplaneDowntimePhase1Start,
                DataplaneDowntimePhase2Start,
                DataplaneDowntimePhase1StartPass,
                DataplaneDowntimePhase1StartFail,
                "Sending pass to upg-mgr for Dataplane Downtime Phase1 Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Phase1 Start message",
                "Dataplane Downtime Phase1 Start",
                "Dataplane Downtime Phase1 Success",
                "Dataplane Downtime Phase1 Fail",
                &UpgPreStateHandler::PreDataplaneDowntimePhase1Start,
                &UpgPostStateHandler::PostDataplaneDowntimePhase1Start
               };
    StateMachine[DataplaneDowntimePhase2Start] =
               {
                DataplaneDowntimePhase2Start,
                DataplaneDowntimePhase3Start,
                DataplaneDowntimePhase2StartPass,
                DataplaneDowntimePhase2StartFail,
                "Sending pass to upg-mgr for Dataplane Downtime Phase2 Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Phase2 Start message",
                "Dataplane Downtime Phase2 Start",
                "Dataplane Downtime Phase2 Success",
                "Dataplane Downtime Phase2 Fail",
                &UpgPreStateHandler::PreDataplaneDowntimePhase2Start,
                &UpgPostStateHandler::PostDataplaneDowntimePhase2Start
               };
    StateMachine[DataplaneDowntimePhase3Start] =
               {
                DataplaneDowntimePhase3Start,
                DataplaneDowntimePhase4Start,
                DataplaneDowntimePhase3StartPass,
                DataplaneDowntimePhase3StartFail,
                "Sending pass to upg-mgr for Dataplane Downtime Phase3 Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Phase3 Start message",
                "Dataplane Downtime Phase3 Start",
                "Dataplane Downtime Phase3 Success",
                "Dataplane Downtime Phase3 Fail",
                &UpgPreStateHandler::PreDataplaneDowntimePhase3Start,
                &UpgPostStateHandler::PostDataplaneDowntimePhase3Start
               };
    StateMachine[DataplaneDowntimePhase4Start] =
               {
                DataplaneDowntimePhase4Start,
                UpgSuccess,
                DataplaneDowntimePhase4StartPass,
                DataplaneDowntimePhase4StartFail,
                "Sending pass to upg-mgr for Dataplane Downtime Phase4 Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Phase4 Start message",
                "Dataplane Downtime Phase4 Start",
                "Dataplane Downtime Phase4 Success",
                "Dataplane Downtime Phase4 Fail",
                &UpgPreStateHandler::PreDataplaneDowntimePhase4Start,
                &UpgPostStateHandler::PostDataplaneDowntimePhase4Start
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
                &UpgPreStateHandler::PreUpgSuccess,
                &UpgPostStateHandler::PostUpgSuccess
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
                &UpgPreStateHandler::PreUpgFailed,
                &UpgPostStateHandler::PostUpgFailed
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
                &UpgPreStateHandler::PreCleanup,
                &UpgPostStateHandler::PostCleanup
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
                &UpgPreStateHandler::PreUpgAborted,
                &UpgPostStateHandler::PostUpgAborted
               };
}
}
