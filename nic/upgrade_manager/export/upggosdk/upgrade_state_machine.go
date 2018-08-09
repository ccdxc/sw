package upggosdk

import (
	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

type upgStateMachine struct {
	state                     upgrade.UpgReqStateType
	stateNext                 upgrade.UpgReqStateType
	statePassResp             upgrade.UpgStateRespType
	stateFailResp             upgrade.UpgStateRespType
	upgAppRespValToStrPass    string
	upgAppRespValToStrFail    string
	upgReqStateTypeToStr      string
	upgRespStateTypeToStrPass string
	upgRespStateTypeToStrFail string
}

var upgradeStateMachine []upgStateMachine
var canUpgradeStateMachine []upgStateMachine

func upgRespStateTypeToUpgReqStateType(resp upgrade.UpgStateRespType) upgrade.UpgReqStateType {
	switch resp {
	case upgrade.UpgStateRespType_UpgStateCompatCheckRespPass:
		return upgrade.UpgReqStateType_UpgStateCompatCheck
	case upgrade.UpgStateRespType_UpgStateCompatCheckRespFail:
		return upgrade.UpgReqStateType_UpgStateCompatCheck
	case upgrade.UpgStateRespType_UpgStateProcessQuiesceRespPass:
		return upgrade.UpgReqStateType_UpgStateProcessQuiesce
	case upgrade.UpgStateRespType_UpgStateProcessQuiesceRespFail:
		return upgrade.UpgReqStateType_UpgStateProcessQuiesce
	case upgrade.UpgStateRespType_UpgStatePostBinRestartRespPass:
		return upgrade.UpgReqStateType_UpgStatePostBinRestart
	case upgrade.UpgStateRespType_UpgStatePostBinRestartRespFail:
		return upgrade.UpgReqStateType_UpgStatePostBinRestart
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase1RespPass:
		return upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase1
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase1RespFail:
		return upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase1
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase2RespPass:
		return upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase2
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase2RespFail:
		return upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase2
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase3RespPass:
		return upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase3
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase3RespFail:
		return upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase3
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase4RespPass:
		return upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase4
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase4RespFail:
		return upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase4
	case upgrade.UpgStateRespType_UpgStateCleanupRespPass:
		return upgrade.UpgReqStateType_UpgStateCleanup
	case upgrade.UpgStateRespType_UpgStateCleanupRespFail:
		return upgrade.UpgReqStateType_UpgStateCleanup
	case upgrade.UpgStateRespType_UpgStateSuccessRespPass:
		return upgrade.UpgReqStateType_UpgStateSuccess
	case upgrade.UpgStateRespType_UpgStateSuccessRespFail:
		return upgrade.UpgReqStateType_UpgStateSuccess
	case upgrade.UpgStateRespType_UpgStateFailedRespPass:
		return upgrade.UpgReqStateType_UpgStateFailed
	case upgrade.UpgStateRespType_UpgStateFailedRespFail:
		return upgrade.UpgReqStateType_UpgStateFailed
	case upgrade.UpgStateRespType_UpgStateAbortRespPass:
		return upgrade.UpgReqStateType_UpgStateAbort
	case upgrade.UpgStateRespType_UpgStateAbortRespFail:
		return upgrade.UpgReqStateType_UpgStateAbort
	case upgrade.UpgStateRespType_UpgStateUpgPossiblePass:
		return upgrade.UpgReqStateType_UpgStateUpgPossible
	case upgrade.UpgStateRespType_UpgStateUpgPossibleFail:
		return upgrade.UpgReqStateType_UpgStateUpgPossible
	default:
		log.Infof("Should never come here")
		return upgrade.UpgReqStateType_UpgStateTerminal
	}
}

func upgRespValPassStr(req upgrade.UpgReqStateType) string {
	if req == upgrade.UpgReqStateType_UpgStateUpgPossible {
		return canUpgradeStateMachine[req].upgAppRespValToStrPass
	}
	return upgradeStateMachine[req].upgAppRespValToStrPass
}

func upgRespValFailStr(req upgrade.UpgReqStateType) string {
	if req == upgrade.UpgReqStateType_UpgStateUpgPossible {
		return canUpgradeStateMachine[req].upgAppRespValToStrFail
	}
	return upgradeStateMachine[req].upgAppRespValToStrFail
}

func upgRespStatePassStr(req upgrade.UpgReqStateType) string {
	if req == upgrade.UpgReqStateType_UpgStateUpgPossible {
		return canUpgradeStateMachine[req].upgRespStateTypeToStrPass
	}
	return upgradeStateMachine[req].upgRespStateTypeToStrPass
}

func upgRespStateFailStr(req upgrade.UpgReqStateType) string {
	if req == upgrade.UpgReqStateType_UpgStateUpgPossible {
		return canUpgradeStateMachine[req].upgRespStateTypeToStrFail
	}
	return upgradeStateMachine[req].upgRespStateTypeToStrFail
}

func upgRespStatePassType(resp upgrade.UpgStateRespType) bool {
	switch resp {
	case upgrade.UpgStateRespType_UpgStateCompatCheckRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateProcessQuiesceRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStatePostBinRestartRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase1RespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase2RespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase3RespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase4RespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateCleanupRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateSuccessRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateFailedRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateAbortRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateUpgPossiblePass:
		return true
	}
	log.Infof("Got failed UpgRespStatePassType %d", resp)
	return false
}

func getAppRespStrUtil(resp upgrade.UpgStateRespType) string {
	ret := upgRespStatePassType(resp)
	req := upgRespStateTypeToUpgReqStateType(resp)
	if ret {
		return upgRespStatePassStr(req)
	}
	return upgRespStateFailStr(req)
}

func getUpgAppRespValToStr(resp upgrade.UpgStateRespType) string {
	ret := upgRespStatePassType(resp)
	req := upgRespStateTypeToUpgReqStateType(resp)
	if ret {
		return upgRespValPassStr(req)
	}
	return upgRespValPassStr(req)
}

func initStateMachineVector() {
	log.Infof("initStateMachineVector called!!!")

	canUpgradeStateMachine = []upgStateMachine{
		upgrade.UpgReqStateType_UpgStateUpgPossible: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateUpgPossible,
			stateNext:                 upgrade.UpgReqStateType_UpgStateTerminal,
			statePassResp:             upgrade.UpgStateRespType_UpgStateUpgPossiblePass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateUpgPossibleFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Pre-Upgrade Check message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Pre-Upgrade Check message",
			upgReqStateTypeToStr:      "Perform Compat Check to see if upgrade is possible",
			upgRespStateTypeToStrPass: "Compat check passed. Upgrade is possible",
			upgRespStateTypeToStrFail: "Compat check failed. Upgrade is not possible",
		},
	}

	upgradeStateMachine = []upgStateMachine{
		upgrade.UpgReqStateType_UpgStateCompatCheck: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateCompatCheck,
			stateNext:                 upgrade.UpgReqStateType_UpgStateProcessQuiesce,
			statePassResp:             upgrade.UpgStateRespType_UpgStateCompatCheckRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateCompatCheckRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Pre-Upgrade Check message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Pre-Upgrade Check message",
			upgReqStateTypeToStr:      "Perform Compat Check",
			upgRespStateTypeToStrPass: "Compat check passed",
			upgRespStateTypeToStrFail: "Compat check failed",
		},
		upgrade.UpgReqStateType_UpgStateProcessQuiesce: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateProcessQuiesce,
			stateNext:                 upgrade.UpgReqStateType_UpgStatePostBinRestart,
			statePassResp:             upgrade.UpgStateRespType_UpgStateProcessQuiesceRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateProcessQuiesceRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Process Quiesced message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Process Quiesced message",
			upgReqStateTypeToStr:      "Quiesce Processes Pre-Restart",
			upgRespStateTypeToStrPass: "Process Quiesce Pass",
			upgRespStateTypeToStrFail: "Process Quiesce Fail",
		},
		upgrade.UpgReqStateType_UpgStatePostBinRestart: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStatePostBinRestart,
			stateNext:                 upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase1,
			statePassResp:             upgrade.UpgStateRespType_UpgStatePostBinRestartRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStatePostBinRestartRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Post-Binary Restart message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Post-Binary Restart message",
			upgReqStateTypeToStr:      "Post Process Restart",
			upgRespStateTypeToStrPass: "Post Process Restart Pass",
			upgRespStateTypeToStrFail: "Post Process Restart Fail",
		},
		upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase1: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase1,
			stateNext:                 upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase2,
			statePassResp:             upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase1RespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase1RespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime Phase1 Start message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime Phase1 Start message",
			upgReqStateTypeToStr:      "Dataplane Downtime Phase1 Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime Phase1 Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime Phase1 Fail",
		},
		upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase2: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase2,
			stateNext:                 upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase3,
			statePassResp:             upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase2RespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase2RespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime Phase2 Start message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime Phase2 Start message",
			upgReqStateTypeToStr:      "Dataplane Downtime Phase2 Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime Phase2 Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime Phase2 Fail",
		},
		upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase3: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase3,
			stateNext:                 upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase4,
			statePassResp:             upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase3RespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase3RespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime Phase3 Start message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime Phase3 Start message",
			upgReqStateTypeToStr:      "Dataplane Downtime Phase3 Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime Phase3 Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime Phase3 Fail",
		},
		upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase4: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase4,
			stateNext:                 upgrade.UpgReqStateType_UpgStateSuccess,
			statePassResp:             upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase4RespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase4RespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Dataplane Downtime Phase4 Start message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Dataplane Downtime Phase4 Start message",
			upgReqStateTypeToStr:      "Dataplane Downtime Phase4 Start",
			upgRespStateTypeToStrPass: "Dataplane Downtime Phase4 Success",
			upgRespStateTypeToStrFail: "Dataplane Downtime Phase4 Fail",
		},
		upgrade.UpgReqStateType_UpgStateCleanup: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateCleanup,
			stateNext:                 upgrade.UpgReqStateType_UpgStateTerminal,
			statePassResp:             upgrade.UpgStateRespType_UpgStateCleanupRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateCleanupRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr after cleaning up stale state",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr after cleaning up stale state",
			upgReqStateTypeToStr:      "Cleanup State",
			upgRespStateTypeToStrPass: "Cleanup Pass",
			upgRespStateTypeToStrFail: "Cleanup Fail",
		},
		upgrade.UpgReqStateType_UpgStateSuccess: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateSuccess,
			stateNext:                 upgrade.UpgReqStateType_UpgStateCleanup,
			statePassResp:             upgrade.UpgStateRespType_UpgStateSuccessRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateSuccessRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr after upgrade success message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr after upgrade fail message",
			upgReqStateTypeToStr:      "Upgrade Success",
			upgRespStateTypeToStrPass: "",
			upgRespStateTypeToStrFail: "",
		},
		upgrade.UpgReqStateType_UpgStateFailed: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateFailed,
			stateNext:                 upgrade.UpgReqStateType_UpgStateCleanup,
			statePassResp:             upgrade.UpgStateRespType_UpgStateFailedRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateFailedRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr after upgrade fail message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr after upgrade fail message",
			upgReqStateTypeToStr:      "Upgrade Fail",
			upgRespStateTypeToStrPass: "",
			upgRespStateTypeToStrFail: "",
		},
		upgrade.UpgReqStateType_UpgStateAbort: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateAbort,
			stateNext:                 upgrade.UpgReqStateType_UpgStateTerminal,
			statePassResp:             upgrade.UpgStateRespType_UpgStateAbortRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateAbortRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr after handling upgrade aborted",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr after handling upgrade aborted",
			upgReqStateTypeToStr:      "Upgrade Aborted State",
			upgRespStateTypeToStrPass: "Upgrade Aborted Pass",
			upgRespStateTypeToStrFail: "Upgrade Aborted Fail",
		},
	}
}
