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

var nonDisruptiveUpgradeStateMachine []upgStateMachine
var disruptiveUpgradeStateMachine []upgStateMachine
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
	case upgrade.UpgStateRespType_UpgStatePostRestartRespPass:
		return upgrade.UpgReqStateType_UpgStatePostRestart
	case upgrade.UpgStateRespType_UpgStatePostRestartRespFail:
		return upgrade.UpgReqStateType_UpgStatePostRestart
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
	case upgrade.UpgStateRespType_UpgStateUpgPossibleRespPass:
		return upgrade.UpgReqStateType_UpgStateUpgPossible
	case upgrade.UpgStateRespType_UpgStateUpgPossibleRespFail:
		return upgrade.UpgReqStateType_UpgStateUpgPossible
	case upgrade.UpgStateRespType_UpgStateLinkDownRespPass:
		return upgrade.UpgReqStateType_UpgStateLinkDown
	case upgrade.UpgStateRespType_UpgStateLinkDownRespFail:
		return upgrade.UpgReqStateType_UpgStateLinkDown
	case upgrade.UpgStateRespType_UpgStateLinkUpRespPass:
		return upgrade.UpgReqStateType_UpgStateLinkUp
	case upgrade.UpgStateRespType_UpgStateLinkUpRespFail:
		return upgrade.UpgReqStateType_UpgStateLinkUp
	case upgrade.UpgStateRespType_UpgStateHostDownRespPass:
		return upgrade.UpgReqStateType_UpgStateHostDown
	case upgrade.UpgStateRespType_UpgStateHostDownRespFail:
		return upgrade.UpgReqStateType_UpgStateHostDown
	case upgrade.UpgStateRespType_UpgStateHostUpRespPass:
		return upgrade.UpgReqStateType_UpgStateHostUp
	case upgrade.UpgStateRespType_UpgStateHostUpRespFail:
		return upgrade.UpgReqStateType_UpgStateHostUp
	case upgrade.UpgStateRespType_UpgStatePostHostDownRespPass:
		return upgrade.UpgReqStateType_UpgStatePostHostDown
	case upgrade.UpgStateRespType_UpgStatePostHostDownRespFail:
		return upgrade.UpgReqStateType_UpgStatePostHostDown
	case upgrade.UpgStateRespType_UpgStatePostLinkUpRespPass:
		return upgrade.UpgReqStateType_UpgStatePostLinkUp
	case upgrade.UpgStateRespType_UpgStatePostLinkUpRespFail:
		return upgrade.UpgReqStateType_UpgStatePostLinkUp
	case upgrade.UpgStateRespType_UpgStateSaveStateRespPass:
		return upgrade.UpgReqStateType_UpgStateSaveState
	case upgrade.UpgStateRespType_UpgStateSaveStateRespFail:
		return upgrade.UpgReqStateType_UpgStateSaveState
	default:
		log.Infof("Should never come here")
		return upgrade.UpgReqStateType_UpgStateTerminal
	}
}

func upgRespValPassStr(req upgrade.UpgReqStateType) string {
	if req == upgrade.UpgReqStateType_UpgStateUpgPossible {
		return canUpgradeStateMachine[req].upgAppRespValToStrPass
	}
	if upgCtx.upgType == upgrade.UpgType_UpgTypeNonDisruptive {
		return nonDisruptiveUpgradeStateMachine[req].upgAppRespValToStrPass
	}
	return disruptiveUpgradeStateMachine[req].upgAppRespValToStrPass
}

func upgRespValFailStr(req upgrade.UpgReqStateType) string {
	if req == upgrade.UpgReqStateType_UpgStateUpgPossible {
		return canUpgradeStateMachine[req].upgAppRespValToStrFail
	}
	if upgCtx.upgType == upgrade.UpgType_UpgTypeNonDisruptive {
		return nonDisruptiveUpgradeStateMachine[req].upgAppRespValToStrFail
	}
	return disruptiveUpgradeStateMachine[req].upgAppRespValToStrFail
}

func upgRespStatePassStr(req upgrade.UpgReqStateType) string {
	if req == upgrade.UpgReqStateType_UpgStateUpgPossible {
		return canUpgradeStateMachine[req].upgRespStateTypeToStrPass
	}
	if upgCtx.upgType == upgrade.UpgType_UpgTypeNonDisruptive {
		return nonDisruptiveUpgradeStateMachine[req].upgRespStateTypeToStrPass
	}
	return disruptiveUpgradeStateMachine[req].upgRespStateTypeToStrPass
}

func upgRespStateFailStr(req upgrade.UpgReqStateType) string {
	if req == upgrade.UpgReqStateType_UpgStateUpgPossible {
		return canUpgradeStateMachine[req].upgRespStateTypeToStrFail
	}
	if upgCtx.upgType == upgrade.UpgType_UpgTypeNonDisruptive {
		return nonDisruptiveUpgradeStateMachine[req].upgRespStateTypeToStrFail
	}
	return disruptiveUpgradeStateMachine[req].upgRespStateTypeToStrFail
}

func upgRespStatePassType(resp upgrade.UpgStateRespType) bool {
	switch resp {
	case upgrade.UpgStateRespType_UpgStateCompatCheckRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateProcessQuiesceRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateLinkDownRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateLinkUpRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStatePostRestartRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase1RespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase2RespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase3RespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase4RespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateSuccessRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateHostDownRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateHostUpRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStatePostHostDownRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStatePostLinkUpRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateSaveStateRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateFailedRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateAbortRespPass:
		return true
	case upgrade.UpgStateRespType_UpgStateUpgPossibleRespPass:
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
	return upgRespValFailStr(req)
}

func initStateMachineVector() {
	log.Infof("initStateMachineVector called!!!")

	canUpgradeStateMachine = []upgStateMachine{
		upgrade.UpgReqStateType_UpgStateUpgPossible: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateUpgPossible,
			stateNext:                 upgrade.UpgReqStateType_UpgStateTerminal,
			statePassResp:             upgrade.UpgStateRespType_UpgStateUpgPossibleRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateUpgPossibleRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Pre-Upgrade Check message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Pre-Upgrade Check message",
			upgReqStateTypeToStr:      "Perform Compat Check to see if upgrade is possible",
			upgRespStateTypeToStrPass: "Compat check passed. Upgrade is possible",
			upgRespStateTypeToStrFail: "Compat check failed. Upgrade is not possible",
		},
	}

	nonDisruptiveUpgradeStateMachine = []upgStateMachine{
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
			stateNext:                 upgrade.UpgReqStateType_UpgStatePostRestart,
			statePassResp:             upgrade.UpgStateRespType_UpgStateProcessQuiesceRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateProcessQuiesceRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Process Quiesced message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Process Quiesced message",
			upgReqStateTypeToStr:      "Quiesce Processes Pre-Restart",
			upgRespStateTypeToStrPass: "Process Quiesce Pass",
			upgRespStateTypeToStrFail: "Process Quiesce Fail",
		},
		upgrade.UpgReqStateType_UpgStatePostRestart: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStatePostRestart,
			stateNext:                 upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase1,
			statePassResp:             upgrade.UpgStateRespType_UpgStatePostRestartRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStatePostRestartRespFail,
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
		upgrade.UpgReqStateType_UpgStateSuccess: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateSuccess,
			stateNext:                 upgrade.UpgReqStateType_UpgStateTerminal,
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
			stateNext:                 upgrade.UpgReqStateType_UpgStateTerminal,
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

	disruptiveUpgradeStateMachine = []upgStateMachine{
		upgrade.UpgReqStateType_UpgStateCompatCheck: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateCompatCheck,
			stateNext:                 upgrade.UpgReqStateType_UpgStateLinkDown,
			statePassResp:             upgrade.UpgStateRespType_UpgStateCompatCheckRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateCompatCheckRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Pre-Upgrade Check message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Pre-Upgrade Check message",
			upgReqStateTypeToStr:      "Perform Compat Check",
			upgRespStateTypeToStrPass: "Compat check passed",
			upgRespStateTypeToStrFail: "Compat check failed",
		},
		upgrade.UpgReqStateType_UpgStateLinkDown: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateLinkDown,
			stateNext:                 upgrade.UpgReqStateType_UpgStateHostDown,
			statePassResp:             upgrade.UpgStateRespType_UpgStateLinkDownRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateLinkDownRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Link Down message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Link Down message",
			upgReqStateTypeToStr:      "Link Down",
			upgRespStateTypeToStrPass: "Link Down Pass",
			upgRespStateTypeToStrFail: "Link Down Fail",
		},
		upgrade.UpgReqStateType_UpgStateHostDown: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateHostDown,
			stateNext:                 upgrade.UpgReqStateType_UpgStatePostHostDown,
			statePassResp:             upgrade.UpgStateRespType_UpgStateHostDownRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateHostDownRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Host Down message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Host Down message",
			upgReqStateTypeToStr:      "Host Down",
			upgRespStateTypeToStrPass: "Host Down Success",
			upgRespStateTypeToStrFail: "Host Down Fail",
		},
		upgrade.UpgReqStateType_UpgStatePostHostDown: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStatePostHostDown,
			stateNext:                 upgrade.UpgReqStateType_UpgStateSaveState,
			statePassResp:             upgrade.UpgStateRespType_UpgStatePostHostDownRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStatePostHostDownRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Post Host Down message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Post Host Down message",
			upgReqStateTypeToStr:      "Post Host Down",
			upgRespStateTypeToStrPass: "Post Host Down Success",
			upgRespStateTypeToStrFail: "Post Host Down Fail",
		},
		upgrade.UpgReqStateType_UpgStateSaveState: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateSaveState,
			stateNext:                 upgrade.UpgReqStateType_UpgStatePostRestart,
			statePassResp:             upgrade.UpgStateRespType_UpgStateSaveStateRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateSaveStateRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Save State message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Save State message",
			upgReqStateTypeToStr:      "Save State",
			upgRespStateTypeToStrPass: "Save State Pass",
			upgRespStateTypeToStrFail: "Save State Fail",
		},
		upgrade.UpgReqStateType_UpgStatePostRestart: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStatePostRestart,
			stateNext:                 upgrade.UpgReqStateType_UpgStateHostUp,
			statePassResp:             upgrade.UpgStateRespType_UpgStatePostRestartRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStatePostRestartRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Post-Binary Restart message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Post-Binary Restart message",
			upgReqStateTypeToStr:      "Post Process Restart",
			upgRespStateTypeToStrPass: "Post Process Restart Pass",
			upgRespStateTypeToStrFail: "Post Process Restart Fail",
		},
		upgrade.UpgReqStateType_UpgStateHostUp: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateHostUp,
			stateNext:                 upgrade.UpgReqStateType_UpgStateLinkUp,
			statePassResp:             upgrade.UpgStateRespType_UpgStateHostUpRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateHostUpRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Host Up message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Host Up message",
			upgReqStateTypeToStr:      "Host Up",
			upgRespStateTypeToStrPass: "Host Up Pass",
			upgRespStateTypeToStrFail: "Host Up Fail",
		},
		upgrade.UpgReqStateType_UpgStateLinkUp: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateLinkUp,
			stateNext:                 upgrade.UpgReqStateType_UpgStatePostLinkUp,
			statePassResp:             upgrade.UpgStateRespType_UpgStateLinkUpRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStateLinkUpRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Link Up message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Link Up message",
			upgReqStateTypeToStr:      "Link Up",
			upgRespStateTypeToStrPass: "Link Up Pass",
			upgRespStateTypeToStrFail: "Link Up Fail",
		},
		upgrade.UpgReqStateType_UpgStatePostLinkUp: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStatePostLinkUp,
			stateNext:                 upgrade.UpgReqStateType_UpgStateSuccess,
			statePassResp:             upgrade.UpgStateRespType_UpgStatePostLinkUpRespPass,
			stateFailResp:             upgrade.UpgStateRespType_UpgStatePostLinkUpRespFail,
			upgAppRespValToStrPass:    "Sending pass to upg-mgr for Post Link Up message",
			upgAppRespValToStrFail:    "Sending fail to upg-mgr for Post Link Up message",
			upgReqStateTypeToStr:      "Post Link Up",
			upgRespStateTypeToStrPass: "Post Link Up Pass",
			upgRespStateTypeToStrFail: "Post Link Up Fail",
		},
		upgrade.UpgReqStateType_UpgStateSuccess: upgStateMachine{
			state:                     upgrade.UpgReqStateType_UpgStateSuccess,
			stateNext:                 upgrade.UpgReqStateType_UpgStateTerminal,
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
			stateNext:                 upgrade.UpgReqStateType_UpgStateTerminal,
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
