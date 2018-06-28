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

func upgRespStateTypeToUpgReqStateType(resp upgrade.UpgRespStateType) upgrade.UpgReqStateType {
	switch resp {
	case upgrade.UpgRespStateType_PreUpgStatePass:
		return upgrade.UpgReqStateType_PreUpgState
	case upgrade.UpgRespStateType_PreUpgStateFail:
		return upgrade.UpgReqStateType_PreUpgState
	case upgrade.UpgRespStateType_ProcessesQuiescedPass:
		return upgrade.UpgReqStateType_ProcessesQuiesced
	case upgrade.UpgRespStateType_ProcessesQuiescedFail:
		return upgrade.UpgReqStateType_ProcessesQuiesced
	case upgrade.UpgRespStateType_PostBinRestartPass:
		return upgrade.UpgReqStateType_PostBinRestart
	case upgrade.UpgRespStateType_PostBinRestartFail:
		return upgrade.UpgReqStateType_PostBinRestart
	case upgrade.UpgRespStateType_DataplaneDowntimePhase1StartPass:
		return upgrade.UpgReqStateType_DataplaneDowntimePhase1Start
	case upgrade.UpgRespStateType_DataplaneDowntimePhase1StartFail:
		return upgrade.UpgReqStateType_DataplaneDowntimePhase1Start
	case upgrade.UpgRespStateType_DataplaneDowntimePhase2StartPass:
		return upgrade.UpgReqStateType_DataplaneDowntimePhase2Start
	case upgrade.UpgRespStateType_DataplaneDowntimePhase2StartFail:
		return upgrade.UpgReqStateType_DataplaneDowntimePhase2Start
	case upgrade.UpgRespStateType_DataplaneDowntimePhase3StartPass:
		return upgrade.UpgReqStateType_DataplaneDowntimePhase3Start
	case upgrade.UpgRespStateType_DataplaneDowntimePhase3StartFail:
		return upgrade.UpgReqStateType_DataplaneDowntimePhase3Start
	case upgrade.UpgRespStateType_DataplaneDowntimePhase4StartPass:
		return upgrade.UpgReqStateType_DataplaneDowntimePhase4Start
	case upgrade.UpgRespStateType_DataplaneDowntimePhase4StartFail:
		return upgrade.UpgReqStateType_DataplaneDowntimePhase4Start
	case upgrade.UpgRespStateType_CleanupPass:
		return upgrade.UpgReqStateType_Cleanup
	case upgrade.UpgRespStateType_CleanupFail:
		return upgrade.UpgReqStateType_Cleanup
	case upgrade.UpgRespStateType_UpgSuccessPass:
		return upgrade.UpgReqStateType_UpgSuccess
	case upgrade.UpgRespStateType_UpgSuccessFail:
		return upgrade.UpgReqStateType_UpgSuccess
	case upgrade.UpgRespStateType_UpgFailedPass:
		return upgrade.UpgReqStateType_UpgFailed
	case upgrade.UpgRespStateType_UpgFailedFail:
		return upgrade.UpgReqStateType_UpgFailed
	case upgrade.UpgRespStateType_UpgAbortedPass:
		return upgrade.UpgReqStateType_UpgAborted
	case upgrade.UpgRespStateType_UpgAbortedFail:
		return upgrade.UpgReqStateType_UpgAborted
	default:
		log.Infof("Should never come here")
		return upgrade.UpgReqStateType_UpgStateTerminal
	}
}

func upgRespValPassStr(req upgrade.UpgReqStateType) string {
	return stateMachine[req].upgAppRespValToStrPass
}

func upgRespValFailStr(req upgrade.UpgReqStateType) string {
	return stateMachine[req].upgAppRespValToStrFail
}

func upgRespStatePassStr(req upgrade.UpgReqStateType) string {
	return stateMachine[req].upgRespStateTypeToStrPass
}

func upgRespStateFailStr(req upgrade.UpgReqStateType) string {
	return stateMachine[req].upgRespStateTypeToStrFail
}

func upgRespStatePassType(resp upgrade.UpgRespStateType) bool {
	switch resp {
	case upgrade.UpgRespStateType_PreUpgStatePass:
		return true
	case upgrade.UpgRespStateType_ProcessesQuiescedPass:
		return true
	case upgrade.UpgRespStateType_PostBinRestartPass:
		return true
	case upgrade.UpgRespStateType_DataplaneDowntimePhase1StartPass:
		return true
	case upgrade.UpgRespStateType_DataplaneDowntimePhase2StartPass:
		return true
	case upgrade.UpgRespStateType_DataplaneDowntimePhase3StartPass:
		return true
	case upgrade.UpgRespStateType_DataplaneDowntimePhase4StartPass:
		return true
	case upgrade.UpgRespStateType_CleanupPass:
		return true
	case upgrade.UpgRespStateType_UpgSuccessPass:
		return true
	case upgrade.UpgRespStateType_UpgFailedPass:
		return true
	case upgrade.UpgRespStateType_UpgAbortedPass:
		return true
	}
	log.Infof("Got failed UpgRespStatePassType %d", resp)
	return false
}

func getAppRespStrUtil(resp upgrade.UpgRespStateType) string {
	ret := upgRespStatePassType(resp)
	req := upgRespStateTypeToUpgReqStateType(resp)
	if ret {
		return upgRespStatePassStr(req)
	}
	return upgRespStateFailStr(req)
}

func getUpgAppRespValToStr(resp upgrade.UpgRespStateType) string {
	ret := upgRespStatePassType(resp)
	req := upgRespStateTypeToUpgReqStateType(resp)
	if ret {
		return upgRespValPassStr(req)
	}
	return upgRespValPassStr(req)
}

func initStateMachineVector() {
	log.Infof("initStateMachineVector called!!!")

	stateMachine = []upgStateMachine{
		upgrade.UpgReqStateType_PreUpgState: upgStateMachine{
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
		upgrade.UpgReqStateType_ProcessesQuiesced: upgStateMachine{
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
		upgrade.UpgReqStateType_PostBinRestart: upgStateMachine{
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
		upgrade.UpgReqStateType_DataplaneDowntimePhase1Start: upgStateMachine{
			state:                     upgrade.UpgReqStateType_DataplaneDowntimePhase1Start,
			stateNext:                 upgrade.UpgReqStateType_DataplaneDowntimePhase2Start,
			statePassResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase1StartPass,
			stateFailResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase1StartFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime Phase1 Start message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime Phase1 Start message",
			upgReqStateTypeToStr:      "Dataplane Downtime Phase1 Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime Phase1 Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime Phase1 Fail",
		},
		upgrade.UpgReqStateType_DataplaneDowntimePhase2Start: upgStateMachine{
			state:                     upgrade.UpgReqStateType_DataplaneDowntimePhase2Start,
			stateNext:                 upgrade.UpgReqStateType_DataplaneDowntimePhase3Start,
			statePassResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase2StartPass,
			stateFailResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase2StartFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime Phase2 Start message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime Phase2 Start message",
			upgReqStateTypeToStr:      "Dataplane Downtime Phase2 Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime Phase2 Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime Phase2 Fail",
		},
		upgrade.UpgReqStateType_DataplaneDowntimePhase3Start: upgStateMachine{
			state:                     upgrade.UpgReqStateType_DataplaneDowntimePhase3Start,
			stateNext:                 upgrade.UpgReqStateType_DataplaneDowntimePhase4Start,
			statePassResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase3StartPass,
			stateFailResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase3StartFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime Phase3 Start message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime Phase3 Start message",
			upgReqStateTypeToStr:      "Dataplane Downtime Phase3 Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime Phase3 Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime Phase3 Fail",
		},
		upgrade.UpgReqStateType_DataplaneDowntimePhase4Start: upgStateMachine{
			state:                     upgrade.UpgReqStateType_DataplaneDowntimePhase4Start,
			stateNext:                 upgrade.UpgReqStateType_UpgSuccess,
			statePassResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase4StartPass,
			stateFailResp:             upgrade.UpgRespStateType_DataplaneDowntimePhase4StartFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime Phase4 Start message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime Phase4 Start message",
			upgReqStateTypeToStr:      "Dataplane Downtime Phase4 Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime Phase4 Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime Phase4 Fail",
		},
		upgrade.UpgReqStateType_Cleanup: upgStateMachine{
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
		upgrade.UpgReqStateType_UpgSuccess: upgStateMachine{
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
		upgrade.UpgReqStateType_UpgFailed: upgStateMachine{
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
		upgrade.UpgReqStateType_UpgAborted: upgStateMachine{
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
