package upgsdk

import (
	"errors"

	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

//SvcRole upgrade service application role
type SvcRole int

const (
	//AgentRole role agent
	AgentRole SvcRole = 0
	//NonAgentRole role non-agent
	NonAgentRole SvcRole = 1
)

type upgSdk struct {
	svcName   string
	svcRole   SvcRole
	sdkClient gosdk.Client
}

//HdlrRespCode application response code
type HdlrRespCode int

const (
	//Success response
	Success HdlrRespCode = 0
	//Fail response
	Fail HdlrRespCode = 1
	//InProgress response
	InProgress HdlrRespCode = 2
)

//HdlrResp application response
type HdlrResp struct {
	Resp   HdlrRespCode
	ErrStr string
}

//AgentHandlers agents to implement this
type AgentHandlers interface {
	UpgStatePreUpgCheckComplete(resp *HdlrResp, svcName string)
	UpgStateProcessQuiesceComplete(resp *HdlrResp, svcName string)
	UpgStatePostBinRestartComplete(resp *HdlrResp, svcName string)
	UpgStateDataplaneDowntimePhase1Complete(resp *HdlrResp, svcName string)
	UpgStateDataplaneDowntimePhase2Complete(resp *HdlrResp, svcName string)
	UpgStateDataplaneDowntimePhase3Complete(resp *HdlrResp, svcName string)
	UpgStateDataplaneDowntimePhase4Complete(resp *HdlrResp, svcName string)
	UpgStateCleanupComplete(resp *HdlrResp, svcName string)
	UpgStateAbortedComplete(resp *HdlrResp, svcName string)

	UpgSuccessful()
	UpgFailed(errStrList *[]string)
	UpgAborted(errStrList *[]string)
}

//UpgCtx is the wrapper that holds all the information about the current upgrade
type UpgCtx struct {
	fromVer string
	toVer   string
	upgType upgrade.UpgType
}

//UpgAppHandlers all upgrade applications to implement this
type UpgAppHandlers interface {
	HandleStatePreUpgState(upgCtx *UpgCtx) HdlrResp
	HandleStatePostBinRestart(upgCtx *UpgCtx) HdlrResp
	HandleStateProcessesQuiesced(upgCtx *UpgCtx) HdlrResp
	HandleStateDataplaneDowntimePhase1Start(upgCtx *UpgCtx) HdlrResp
	HandleStateDataplaneDowntimePhase2Start(upgCtx *UpgCtx) HdlrResp
	HandleStateDataplaneDowntimePhase3Start(upgCtx *UpgCtx) HdlrResp
	HandleStateDataplaneDowntimePhase4Start(upgCtx *UpgCtx) HdlrResp
	HandleStateCleanup(upgCtx *UpgCtx) HdlrResp
	HandleStateUpgSuccess(upgCtx *UpgCtx)
	HandleStateUpgFailed(upgCtx *UpgCtx)
	HandleStateUpgAborted(upgCtx *UpgCtx)
}

// UpgSdk is the main Upgrade SDK API
type UpgSdk interface {
	StartUpgrade() error
	AbortUpgrade() error
	GetUpgradeStatus(retStr *[]string) error
	SendAppRespSuccess() error
	SendAppRespFail(str string) error
	IsUpgradeInProgress() bool
}

func (u *upgSdk) IsUpgradeInProgress() bool {
	upgreq := upgrade.GetUpgReq(u.sdkClient, 10)
	if upgreq != nil &&
		upgreq.GetUpgReqCmd() == upgrade.UpgReqType_UpgStart {
		log.Infof("Upgrade in progress")
		return true
	}
	log.Infof("Upgrade not in progress")
	return false
}

//SendAppRespFail is used to reply with Fail
func (u *upgSdk) SendAppRespFail(str string) error {
	var resp HdlrResp
	resp.Resp = Fail
	resp.ErrStr = str
	upgreq := upgrade.GetUpgStateReq(u.sdkClient, 10)
	if upgreq == nil {
		return errors.New("No active upgrade in progress")
	}
	reqType := upgreq.GetUpgReqState()
	respType := getUpgAppRespNextFail(reqType)
	updateUpgAppResp(respType, &resp, u.svcName, u.sdkClient)
	return nil
}

//SendAppRespSuccess is used to reply with Success
func (u *upgSdk) SendAppRespSuccess() error {
	var resp HdlrResp
	resp.Resp = Success
	resp.ErrStr = ""
	upgreq := upgrade.GetUpgStateReq(u.sdkClient, 10)
	if upgreq == nil {
		return errors.New("No active upgrade in progress")
	}
	reqType := upgreq.GetUpgReqState()
	respType := getUpgAppRespNextPass(reqType)
	updateUpgAppResp(respType, &resp, u.svcName, u.sdkClient)
	return nil
}

//NewUpgSdk API is used to init upgrade sdk
func NewUpgSdk(name string, client gosdk.Client, role SvcRole, agentHdlrs AgentHandlers, appHdlrs UpgAppHandlers) (UpgSdk, error) {
	log.Infof("NewUpgSdk called for %s", name)
	upgsdk := &upgSdk{
		svcName:   name,
		sdkClient: client,
		svcRole:   role,
	}
	if appHdlrs == nil {
		return nil, errors.New("Application handlers need to be provided")
	}
	if role == AgentRole {
		if agentHdlrs == nil {
			return nil, errors.New("AgentHandlers need to be provided for Agent application")
		}
		upgrade.UpgReqMount(client, delphi.MountMode_ReadWriteMode)
		upgRespInit(client, agentHdlrs)
		upgAppRespInit(client, agentHdlrs)
	} else {
		if agentHdlrs != nil {
			return nil, errors.New("Non-Agent applications cannot provide AgentHandlers")
		}
		upgAppRespInit(client, nil)
	}
	upgStateReqInit(client, appHdlrs, name)
	upgAppRegInit(client)
	initStateMachineVector()
	return upgsdk, nil
}

func (u *upgSdk) StartUpgrade() error {
	if u.svcRole != AgentRole {
		return errors.New("Svc not of role Agent")
	}
	upgreq := upgrade.GetUpgReq(u.sdkClient, 10)
	if upgreq == nil {
		upgreq = upgrade.NewUpgReqWithKey(u.sdkClient, 10)
	}
	upgreq.SetUpgReqCmd(upgrade.UpgReqType_UpgStart)
	return nil
}

func (u *upgSdk) AbortUpgrade() error {
	if u.svcRole != AgentRole {
		return errors.New("Svc not of role Agent")
	}
	upgreq := upgrade.GetUpgReq(u.sdkClient, 10)
	if upgreq == nil {
		return errors.New("No upgrade in progress")
	}
	upgreq.SetUpgReqCmd(upgrade.UpgReqType_UpgAbort)
	return nil
}

func (u *upgSdk) GetUpgradeStatus(retStr *[]string) error {
	if u.svcRole != AgentRole {
		return errors.New("Svc not of role Agent")
	}

	//Check if upgrade is initiated
	*retStr = append(*retStr, "======= Checking if Upgrade is initiated =======")
	upgreq := upgrade.GetUpgReq(u.sdkClient, 10)
	if upgreq == nil {
		*retStr = append(*retStr, "No active upgrade detected from agent side.")
	} else if upgreq.GetUpgReqCmd() == upgrade.UpgReqType_UpgStart {
		*retStr = append(*retStr, "Agent initiated upgrade.")
	} else if upgreq.GetUpgReqCmd() == upgrade.UpgReqType_UpgAbort {
		*retStr = append(*retStr, "Agent aborted upgrade.")
	}

	//Check if Upgrade Manager is running the state machine
	*retStr = append(*retStr, "======= Checking if Upgrade Manager State Machine is running =======")
	upgstatereq := upgrade.GetUpgStateReq(u.sdkClient, 10)
	if upgstatereq == nil {
		*retStr = append(*retStr, "Upgrade Manager not running state machine")
	} else {
		*retStr = append(*retStr, "Upgrade Manager running state machine. State is:")
		*retStr = append(*retStr, stateMachine[upgstatereq.GetUpgReqState()].upgReqStateTypeToStr)
	}

	//Check the status of individual applications
	*retStr = append(*retStr, "======= Checking status of all applications =======")
	//TODO

	//Check if upgrade manager replied back to the agent
	*retStr = append(*retStr, "======= Checking status upgrade manager reply to agent =======")
	upgresp := upgrade.GetUpgResp(u.sdkClient, 10)
	if upgresp == nil {
		*retStr = append(*retStr, "Upgrade Manager has not replied back to agent yet.")
	} else if upgresp.GetUpgRespVal() == upgrade.UpgRespType_UpgRespPass {
		*retStr = append(*retStr, "Upgrade completed successfully.")
	} else if upgresp.GetUpgRespVal() == upgrade.UpgRespType_UpgRespFail {
		*retStr = append(*retStr, "Upgrade completed with failure.")
		for idx := 0; idx < upgresp.GetUpgRespFailStr().Length(); idx++ {
			*retStr = append(*retStr, upgresp.GetUpgRespFailStr().Get(idx))
		}
	} else if upgresp.GetUpgRespVal() == upgrade.UpgRespType_UpgRespAbort {
		*retStr = append(*retStr, "Upgrade aborted.")
		for idx := 0; idx < upgresp.GetUpgRespFailStr().Length(); idx++ {
			*retStr = append(*retStr, upgresp.GetUpgRespFailStr().Get(idx))
		}
	}

	return nil
}
