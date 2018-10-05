#include <stdio.h>
#include <iostream>

#include "upgrade_state_machine.hpp"
#include "upgrade_pre_state_handlers.hpp"
#include "upgrade_post_state_handlers.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

UpgStateMachine NonDisruptiveUpgradeStateMachine[UpgStateTerminal];
UpgStateMachine DisruptiveUpgradeStateMachine[UpgStateTerminal];
UpgStateMachine CanUpgradeStateMachine[UpgStateTerminal];
UpgPreStateHandler* preStateHandlers;
UpgPostStateHandler* postStateHandlers;

UpgReqStateType UpgStateRespTypeToUpgReqStateType(UpgStateRespType type) {
    switch(type) {
    case UpgStateCompatCheckRespPass:
    case UpgStateCompatCheckRespFail:
        return UpgStateCompatCheck;
    case UpgStateLinkDownRespPass:
    case UpgStateLinkDownRespFail:
        return UpgStateLinkDown;
    case UpgStateLinkUpRespPass:
    case UpgStateLinkUpRespFail:
        return UpgStateLinkUp;
    case UpgStateProcessQuiesceRespPass:
    case UpgStateProcessQuiesceRespFail:
        return UpgStateProcessQuiesce;
    case UpgStatePostBinRestartRespPass:
    case UpgStatePostBinRestartRespFail:
        return UpgStatePostBinRestart;
    case UpgStateDataplaneDowntimeStartRespPass:
    case UpgStateDataplaneDowntimeStartRespFail:
        return UpgStateDataplaneDowntimeStart;
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
    case UpgStateIsSystemReadyRespPass:
    case UpgStateIsSystemReadyRespFail:
        return UpgStateIsSystemReady;
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
    case UpgStateUpgPossibleRespPass:
    case UpgStateUpgPossibleRespFail:
        return UpgStateUpgPossible;
    default:
        UPG_LOG_FATAL("Should never come here");
        return UpgStateTerminal;
    }
}

string UpgRespValPassStr(UpgReqStateType req, UpgType upgType) {
    if (req == UpgStateUpgPossible)
        return CanUpgradeStateMachine[req].upgAppRespValToStrPass;
    if (upgType == UpgTypeNonDisruptive)
        return NonDisruptiveUpgradeStateMachine[req].upgAppRespValToStrPass;
    return DisruptiveUpgradeStateMachine[req].upgAppRespValToStrPass;
}

string UpgRespValFailStr(UpgReqStateType req, UpgType upgType) {
    if (req == UpgStateUpgPossible)
        return CanUpgradeStateMachine[req].upgAppRespValToStrFail;
    if (upgType == UpgTypeNonDisruptive)
        return NonDisruptiveUpgradeStateMachine[req].upgAppRespValToStrFail;
    return DisruptiveUpgradeStateMachine[req].upgAppRespValToStrFail;
}

string UpgRespStatePassStr(UpgReqStateType req, UpgType upgType) {
    if (req == UpgStateUpgPossible)
        return CanUpgradeStateMachine[req].upgRespStateTypeToStrPass;
    if (upgType == UpgTypeNonDisruptive)
        return NonDisruptiveUpgradeStateMachine[req].upgRespStateTypeToStrPass;
    return DisruptiveUpgradeStateMachine[req].upgRespStateTypeToStrPass;
}

string UpgRespStateFailStr(UpgReqStateType req, UpgType upgType) {
    if (req == UpgStateUpgPossible)
        return CanUpgradeStateMachine[req].upgRespStateTypeToStrFail;
    if (upgType == UpgTypeNonDisruptive)
        return NonDisruptiveUpgradeStateMachine[req].upgRespStateTypeToStrFail;
    return DisruptiveUpgradeStateMachine[req].upgRespStateTypeToStrFail;
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
    case UpgStateDataplaneDowntimeStartRespPass:
    case UpgStateCleanupRespPass:
    case UpgStateSuccessRespPass:
    case UpgStateFailedRespPass:
    case UpgStateAbortRespPass:
    case UpgStateUpgPossibleRespPass:
    case UpgStateLinkDownRespPass:
    case UpgStateLinkUpRespPass:
    case UpgStateIsSystemReadyRespPass:
        return true;
    default:
        UPG_LOG_DEBUG("Got failed UpgRespStatePassType {}", type);
        return false;
    }
}

string GetAppRespStrUtil(UpgStateRespType type, UpgType upgType) {
    bool ret = UpgRespStatePassType(type);
    UpgReqStateType req = UpgStateRespTypeToUpgReqStateType(type);
    return (ret?UpgRespStatePassStr(req, upgType):UpgRespStateFailStr(req, upgType));
}

string GetUpgAppRespValToStr(UpgStateRespType type, UpgType upgType) {
    bool ret = UpgRespStatePassType(type);
    UpgReqStateType req = UpgStateRespTypeToUpgReqStateType(type);
    return (ret?UpgRespValPassStr(req, upgType):UpgRespValPassStr(req, upgType));
}

void InitStateMachineVector(void) {
    UPG_LOG_DEBUG("InitNonDisruptiveUpgradeStateMachineVector called!!!");
    preStateHandlers = new UpgPreStateHandler();
    postStateHandlers = new UpgPostStateHandler();

    CanUpgradeStateMachine[UpgStateUpgPossible] = 
               {
                UpgStateUpgPossible, 
                UpgStateTerminal, 
                UpgStateUpgPossibleRespPass, 
                UpgStateUpgPossibleRespFail, 
                "Sending pass to upg-mgr for Pre-Upgrade Check message", 
                "Sending fail to upg-mgr for Pre-Upgrade Check message", 
                "Perform Compat Check to see if upgrade is possible", 
                "Compat check passed. Upgrade is possible.", 
                "Compat check failed. Upgrade is not possible.",
                &UpgPreStateHandler::PreCompatCheckHandler,
                &UpgPostStateHandler::PostCompatCheckHandler
               };

    NonDisruptiveUpgradeStateMachine[UpgStateCompatCheck] = 
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
                &UpgPreStateHandler::PreCompatCheckHandler,
                &UpgPostStateHandler::PostCompatCheckHandler
               };
    NonDisruptiveUpgradeStateMachine[UpgStateProcessQuiesce] = 
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
                &UpgPreStateHandler::PreProcessQuiesceHandler,
                &UpgPostStateHandler::PostProcessQuiesceHandler
               };
    NonDisruptiveUpgradeStateMachine[UpgStatePostBinRestart] = 
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
                &UpgPreStateHandler::PrePostBinRestartHandler,
                &UpgPostStateHandler::PostPostBinRestartHandler
               };
    NonDisruptiveUpgradeStateMachine[UpgStateDataplaneDowntimePhase1] =
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
                &UpgPreStateHandler::PreDataplaneDowntimePhase1Handler,
                &UpgPostStateHandler::PostDataplaneDowntimePhase1Handler
               };
    NonDisruptiveUpgradeStateMachine[UpgStateDataplaneDowntimePhase2] =
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
                &UpgPreStateHandler::PreDataplaneDowntimePhase2Handler,
                &UpgPostStateHandler::PostDataplaneDowntimePhase2Handler
               };
    NonDisruptiveUpgradeStateMachine[UpgStateDataplaneDowntimePhase3] =
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
                &UpgPreStateHandler::PreDataplaneDowntimePhase3Handler,
                &UpgPostStateHandler::PostDataplaneDowntimePhase3Handler
               };
    NonDisruptiveUpgradeStateMachine[UpgStateDataplaneDowntimePhase4] =
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
                &UpgPreStateHandler::PreDataplaneDowntimePhase4Handler,
                &UpgPostStateHandler::PostDataplaneDowntimePhase4Handler
               };
    NonDisruptiveUpgradeStateMachine[UpgStateSuccess] = 
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
                &UpgPreStateHandler::PreSuccessHandler,
                &UpgPostStateHandler::PostSuccessHandler
               };
    NonDisruptiveUpgradeStateMachine[UpgStateFailed] = 
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
                &UpgPreStateHandler::PreFailedHandler,
                &UpgPostStateHandler::PostFailedHandler
                };
    NonDisruptiveUpgradeStateMachine[UpgStateCleanup] =
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
                &UpgPreStateHandler::PreCleanupHandler,
                &UpgPostStateHandler::PostCleanupHandler
               };
    NonDisruptiveUpgradeStateMachine[UpgStateAbort] = 
               {
                UpgStateAbort, 
                UpgStateTerminal, 
                UpgStateAbortRespPass, 
                UpgStateAbortRespFail, 
                "Sending pass to upg-mgr after handling upgrade aborted",
                "Sending fail to upg-mgr after handling upgrade aborted", 
                "Upgrade Aborted State", 
                "Upgrade Aborted Pass", 
                "Upgrade Aborted Fail",
                &UpgPreStateHandler::PreAbortHandler,
                &UpgPostStateHandler::PostAbortHandler
               };

    DisruptiveUpgradeStateMachine[UpgStateCompatCheck] = 
               {
                UpgStateCompatCheck, 
                UpgStateLinkDown, 
                UpgStateCompatCheckRespPass, 
                UpgStateCompatCheckRespFail, 
                "Sending pass to upg-mgr for Pre-Upgrade Check message", 
                "Sending fail to upg-mgr for Pre-Upgrade Check message", 
                "Perform Compat Check", 
                "Compat check passed", 
                "Compat check failed",
                &UpgPreStateHandler::PreCompatCheckHandler,
                &UpgPostStateHandler::PostCompatCheckHandler
               };
    DisruptiveUpgradeStateMachine[UpgStateLinkDown] =
               {
                UpgStateLinkDown,
                UpgStateDataplaneDowntimeStart,
                UpgStateLinkDownRespPass,
                UpgStateLinkDownRespFail,
                "Sending pass to upg-mgr for Link Down message",
                "Sending fail to upg-mgr for Link Down message",
                "Link Down",
                "Link Down Pass",
                "Link Down Fail",
                &UpgPreStateHandler::PreLinkDownHandler,
                &UpgPostStateHandler::PostLinkDownHandler
               };
    DisruptiveUpgradeStateMachine[UpgStateDataplaneDowntimeStart] =
               {
                UpgStateDataplaneDowntimeStart,
                UpgStatePostBinRestart,
                UpgStateDataplaneDowntimeStartRespPass,
                UpgStateDataplaneDowntimeStartRespFail,
                "Sending pass to upg-mgr for Dataplane Downtime Start message",
                "Sending fail to upg-mgr for Dataplane Downtime Start message",
                "Dataplane Downtime Start",
                "Dataplane Downtime Success",
                "Dataplane Downtime Fail",
                &UpgPreStateHandler::PreDataplaneDowntimeStartHandler,
                &UpgPostStateHandler::PostDataplaneDowntimeStartHandler
               };
    DisruptiveUpgradeStateMachine[UpgStatePostBinRestart] =
               {
                UpgStatePostBinRestart,
                UpgStateLinkUp,
                UpgStatePostBinRestartRespPass,
                UpgStatePostBinRestartRespFail,
                "Sending pass to upg-mgr for Post-Binary Restart message",
                "Sending fail to upg-mgr for Post-Binary Restart message",
                "Post Process Restart",
                "Post Process Restart Pass",
                "Post Process Restart Fail",
                &UpgPreStateHandler::PrePostBinRestartHandler,
                &UpgPostStateHandler::PostPostBinRestartHandler
               };
    DisruptiveUpgradeStateMachine[UpgStateLinkUp] =
               {
                UpgStateLinkUp,
                UpgStateIsSystemReady,
                UpgStateLinkUpRespPass,
                UpgStateLinkUpRespFail,
                "Sending pass to upg-mgr for Link Up message",
                "Sending fail to upg-mgr for Link Up message",
                "Link Up",
                "Link Up Pass",
                "Link Up Fail",
                &UpgPreStateHandler::PreLinkUpHandler,
                &UpgPostStateHandler::PostLinkUpHandler
               };
    DisruptiveUpgradeStateMachine[UpgStateIsSystemReady] = 
               {
                UpgStateIsSystemReady, 
                UpgStateSuccess,
                UpgStateIsSystemReadyRespPass, 
                UpgStateIsSystemReadyRespFail, 
                "Sending pass to upg-mgr for Is System Ready message", 
                "Sending fail to upg-mgr for Is System Ready message", 
                "Is System Ready", 
                "Is System Ready Pass", 
                "Is System Ready Fail",
                &UpgPreStateHandler::PreIsSystemReadyHandler,
                &UpgPostStateHandler::PostIsSystemReadyHandler
               };
    DisruptiveUpgradeStateMachine[UpgStateSuccess] = 
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
                &UpgPreStateHandler::PreSuccessHandler,
                &UpgPostStateHandler::PostSuccessHandler
               };
    DisruptiveUpgradeStateMachine[UpgStateFailed] = 
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
                &UpgPreStateHandler::PreFailedHandler,
                &UpgPostStateHandler::PostFailedHandler
                };
    DisruptiveUpgradeStateMachine[UpgStateCleanup] =
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
                &UpgPreStateHandler::PreCleanupHandler,
                &UpgPostStateHandler::PostCleanupHandler
               };
    DisruptiveUpgradeStateMachine[UpgStateAbort] = 
               {
                UpgStateAbort, 
                UpgStateTerminal, 
                UpgStateAbortRespPass, 
                UpgStateAbortRespFail, 
                "Sending pass to upg-mgr after handling upgrade aborted",
                "Sending fail to upg-mgr after handling upgrade aborted", 
                "Upgrade Aborted State", 
                "Upgrade Aborted Pass", 
                "Upgrade Aborted Fail",
                &UpgPreStateHandler::PreAbortHandler,
                &UpgPostStateHandler::PostAbortHandler
               };
}
}
