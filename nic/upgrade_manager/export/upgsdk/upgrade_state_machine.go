package upgsdk

import (
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

type upgStateMachine struct {
	state                     upgrade.UpgReqStateType
	stateNext                 upgrade.UpgReqStateType
	statePassResp             upgrade.UpgRespStateType
	stateFailResp             upgrade.UpgRespStateType
	upgAppRespValToStrPass    string
	upgAppRespValToStrFail    string
	upgReqStateTypeToStr      string
	upgRespStateTypeToStrPass string
	upgRespStateTypeToStrFail string
}

var stateMachine []upgStateMachine

func initStateMachineVector() {
	log.Infof("initStateMachineVector called!!!\n")

	stateMachine = []upgStateMachine{
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgReqRcvd,
			stateNext:                 upgrade.UpgReqStateType_PreUpgState,
			statePassResp:             upgrade.UpgRespStateType_UpgReqRcvdPass,
			stateFailResp:             upgrade.UpgRespStateType_UpgReqRcvdFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Upgrade Request Received message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Upgrade Request Received message",
			upgReqStateTypeToStr:      "Upgrade Request Received",
			upgRespStateTypeToStrPass: "Upgrade Request Received Pass",
			upgRespStateTypeToStrFail: "Upgrade Request Received Fail",
		},
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_PreUpgState,
			stateNext:                 upgrade.UpgReqStateType_ProcessesQuiesced,
			statePassResp:             upgrade.UpgRespStateType_PreUpgStatePass,
			stateFailResp:             upgrade.UpgRespStateType_PreUpgStateFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Pre-Upgrade Check message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Pre-Upgrade Check message",
			upgReqStateTypeToStr:      "Perform Compat Check",
			upgRespStateTypeToStrPass: "Compat check passed",
			upgRespStateTypeToStrFail: "Compat check failed",
		},
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_ProcessesQuiesced,
			stateNext:                 upgrade.UpgReqStateType_PostBinRestart,
			statePassResp:             upgrade.UpgRespStateType_ProcessesQuiescedPass,
			stateFailResp:             upgrade.UpgRespStateType_ProcessesQuiescedFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Process Quiesced message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Process Quiesced message",
			upgReqStateTypeToStr:      "Quiesce Processes Pre-Restart",
			upgRespStateTypeToStrPass: "Process Quiesce Pass",
			upgRespStateTypeToStrFail: "Process Quiesce Fail",
		},
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_PostBinRestart,
			stateNext:                 upgrade.UpgReqStateType_DataplaneDowntimePhase1Start,
			statePassResp:             upgrade.UpgRespStateType_PostBinRestartPass,
			stateFailResp:             upgrade.UpgRespStateType_PostBinRestartFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Post-Binary Restart message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Post-Binary Restart message",
			upgReqStateTypeToStr:      "Post Process Restart",
			upgRespStateTypeToStrPass: "Post Process Restart Pass",
			upgRespStateTypeToStrFail: "Post Process Restart Fail",
		},
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_DataplaneDowntimePhase1Start,
			stateNext:                 upgrade.UpgReqStateType_DataplaneDowntimeAdminQHandling,
			statePassResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase1StartPass,
			stateFailResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase1StartFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime Phase1 Start message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime Phase1 Start message",
			upgReqStateTypeToStr:      "Dataplane Downtime Phase1 Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime Phase1 Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime Phase1 Fail",
		},
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_DataplaneDowntimeAdminQHandling,
			stateNext:                 upgrade.UpgReqStateType_DataplaneDowntimePhase2Start,
			statePassResp:             upgrade.UpgRespStateType_DataplaneDowntimeAdminQHandlingPass,
			stateFailResp:             upgrade.UpgRespStateType_DataplaneDowntimeAdminQHandlingFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime AdminQ Handling message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime AdminQ Handling message",
			upgReqStateTypeToStr:      "Dataplane Downtime AdminQ Handling Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime AdminQ Handling Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime AdminQ Handling Fail",
		},
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_DataplaneDowntimePhase2Start,
			stateNext:                 upgrade.UpgReqStateType_UpgSuccess,
			statePassResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase2StartPass,
			stateFailResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase2StartFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime Phase2 Start message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime Phase2 Start message",
			upgReqStateTypeToStr:      "Dataplane Downtime Phase2 Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime Phase2 Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime Phase2 Fail",
		},
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgSuccess,
			stateNext:                 upgrade.UpgReqStateType_Cleanup,
			statePassResp:             upgrade.UpgRespStateType_UpgSuccessPass,
			stateFailResp:             upgrade.UpgRespStateType_UpgSuccessFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr after upgrade success message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr after upgrade fail message",
			upgReqStateTypeToStr:      "Upgrade Success",
			upgRespStateTypeToStrPass: "",
			upgRespStateTypeToStrFail: "",
		},
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgFailed,
			stateNext:                 upgrade.UpgReqStateType_Cleanup,
			statePassResp:             upgrade.UpgRespStateType_UpgFailedPass,
			stateFailResp:             upgrade.UpgRespStateType_UpgFailedFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr after upgrade fail message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr after upgrade fail message",
			upgReqStateTypeToStr:      "Upgrade Fail",
			upgRespStateTypeToStrPass: "",
			upgRespStateTypeToStrFail: "",
		},
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_Cleanup,
			stateNext:                 upgrade.UpgReqStateType_UpgStateTerminal,
			statePassResp:             upgrade.UpgRespStateType_CleanupPass,
			stateFailResp:             upgrade.UpgRespStateType_CleanupFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr after cleaning up stale state",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr after cleaning up stale state",
			upgReqStateTypeToStr:      "Cleanup State",
			upgRespStateTypeToStrPass: "Cleanup Pass",
			upgRespStateTypeToStrFail: "Cleanup Fail",
		},
		upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgAborted,
			stateNext:                 upgrade.UpgReqStateType_UpgStateTerminal,
			statePassResp:             upgrade.UpgRespStateType_UpgAbortedPass,
			stateFailResp:             upgrade.UpgRespStateType_UpgAbortedFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr after handling upgrade aborted",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr after handling upgrade aborted",
			upgReqStateTypeToStr:      "Upgrade Aborted State",
			upgRespStateTypeToStrPass: "Upgrade Aborted Pass",
			upgRespStateTypeToStrFail: "Upgrade Aborted Fail",
		},
	}
}
