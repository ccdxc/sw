#include <stdio.h>
#include <iostream>

#include "upgrade_state_machine.hpp"

namespace upgrade {

using namespace std;

UpgStateMachine StateMachine[UpgStateTerminal];

void InitStateMachineVector(void) {
    LogInfo("InitStateMachineVector called!!!");
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
                "Upgrade Request Received Fail"
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
                "Compat check failed"
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
                "Process Quiesce Fail"
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
                "Post Process Restart Fail"
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
                "Dataplane Downtime Phase1 Fail"
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
                "Dataplane Downtime AdminQ Handling Fail"
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
                "Dataplane Downtime Phase2 Fail"
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
                ""
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
                ""
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
                "Cleanup Fail"
               };
}
}
