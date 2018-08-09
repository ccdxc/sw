package upggosdk

import (
	"errors"

	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk/nic/upgrade_manager/upgrade"
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
	UpgStateCompatCheckCompletionHandler(resp *HdlrResp, svcName string)
	UpgStateProcessQuiesceCompletionHandler(resp *HdlrResp, svcName string)
	UpgStatePostBinRestartCompletionHandler(resp *HdlrResp, svcName string)
	UpgStateDataplaneDowntimePhase1CompletionHandler(resp *HdlrResp, svcName string)
	UpgStateDataplaneDowntimePhase2CompletionHandler(resp *HdlrResp, svcName string)
	UpgStateDataplaneDowntimePhase3CompletionHandler(resp *HdlrResp, svcName string)
	UpgStateDataplaneDowntimePhase4CompletionHandler(resp *HdlrResp, svcName string)
	UpgStateCleanupCompletionHandler(resp *HdlrResp, svcName string)
	UpgStateAbortCompletionHandler(resp *HdlrResp, svcName string)

	UpgSuccessful()
	UpgFailed(errStrList *[]string)
	UpgAborted(errStrList *[]string)
	UpgNotPossible(errStrList *[]string)
	UpgPossible()
}

//TableMeta is the actual table information
type TableMeta struct {
	Version int
	Name    string
}

//UpgCtx is the wrapper that holds all the information about the current upgrade
type UpgCtx struct {
	PreUpgTables  map[string]TableMeta
	PostUpgTables map[string]TableMeta
	upgType       upgrade.UpgType
}

//UpgAppHandlers all upgrade applications to implement this
type UpgAppHandlers interface {
	HandleUpgStateCompatCheck(upgCtx *UpgCtx) HdlrResp
	HandleUpgStatePostBinRestart(upgCtx *UpgCtx) HdlrResp
	HandleUpgStateProcessQuiesce(upgCtx *UpgCtx) HdlrResp
	HandleUpgStateDataplaneDowntimePhase1(upgCtx *UpgCtx) HdlrResp
	HandleUpgStateDataplaneDowntimePhase2(upgCtx *UpgCtx) HdlrResp
	HandleUpgStateDataplaneDowntimePhase3(upgCtx *UpgCtx) HdlrResp
	HandleUpgStateDataplaneDowntimePhase4(upgCtx *UpgCtx) HdlrResp
	HandleUpgStateCleanup(upgCtx *UpgCtx) HdlrResp
	HandleUpgStateSuccess(upgCtx *UpgCtx)
	HandleUpgStateFailed(upgCtx *UpgCtx)
	HandleUpgStateAbort(upgCtx *UpgCtx)
}

// UpgSdk is the main Upgrade SDK API
type UpgSdk interface {
	StartDisruptiveUpgrade() error
	StartNonDisruptiveUpgrade() error
	AbortUpgrade() error
	GetUpgradeStatus(retStr *[]string) error
	SendAppRespSuccess() error
	SendAppRespFail(str string) error
	IsUpgradeInProgress() bool
	CanPerformNonDisruptiveUpgrade() error
	CanPerformDisruptiveUpgrade() error
}

func (u *upgSdk) IsUpgradeInProgress() bool {
	upgreq := upgrade.GetUpgReq(u.sdkClient)
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
	upgreq := upgrade.GetUpgStateReq(u.sdkClient)
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
	upgreq := upgrade.GetUpgStateReq(u.sdkClient)
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
	upggosdk := &upgSdk{
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
		upgAppRespInit(client, agentHdlrs, name)
	} else {
		if agentHdlrs != nil {
			return nil, errors.New("Non-Agent applications cannot provide AgentHandlers")
		}
		upgAppRespInit(client, nil, name)
	}
	upgStateReqInit(client, appHdlrs, name)
	upgAppRegInit(client, name)
	initStateMachineVector()
	return upggosdk, nil
}

func (u *upgSdk) StartDisruptiveUpgrade() error {
	return u.startUpgrade(upgrade.UpgType_UpgTypeDisruptive)
}

func (u *upgSdk) StartNonDisruptiveUpgrade() error {
	return u.startUpgrade(upgrade.UpgType_UpgTypeNonDisruptive)
}

func (u *upgSdk) startUpgrade(upgType upgrade.UpgType) error {
	if u.svcRole != AgentRole {
		return errors.New("Svc not of role Agent")
	}
	upgreq := upgrade.GetUpgReq(u.sdkClient)
	if upgreq == nil {
		upgreq = upgrade.NewUpgReq(u.sdkClient)
	}
	upgreq.SetUpgReqCmd(upgrade.UpgReqType_UpgStart)
	upgreq.SetUpgReqType(upgType)
	return nil
}

func (u *upgSdk) CanPerformDisruptiveUpgrade() error {
	return u.canPerformUpgrade(upgrade.UpgType_UpgTypeDisruptive)
}

func (u *upgSdk) CanPerformNonDisruptiveUpgrade() error {
	return u.canPerformUpgrade(upgrade.UpgType_UpgTypeNonDisruptive)
}

func (u *upgSdk) canPerformUpgrade(upgType upgrade.UpgType) error {
	if u.svcRole != AgentRole {
		return errors.New("Svc not of role Agent")
	}
	upgreq := upgrade.GetUpgReq(u.sdkClient)
	if upgreq == nil {
		upgreq = upgrade.NewUpgReq(u.sdkClient)
	}
	upgreq.SetUpgReqCmd(upgrade.UpgReqType_IsUpgPossible)
	upgreq.SetUpgReqType(upgType)
	return nil
}

func (u *upgSdk) AbortUpgrade() error {
	if u.svcRole != AgentRole {
		return errors.New("Svc not of role Agent")
	}
	upgreq := upgrade.GetUpgReq(u.sdkClient)
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
	upgreq := upgrade.GetUpgReq(u.sdkClient)
	if upgreq == nil {
		*retStr = append(*retStr, "No active upgrade detected from agent side.")
	} else if upgreq.GetUpgReqCmd() == upgrade.UpgReqType_UpgStart {
		*retStr = append(*retStr, "Agent initiated upgrade.")
	} else if upgreq.GetUpgReqCmd() == upgrade.UpgReqType_UpgAbort {
		*retStr = append(*retStr, "Agent aborted upgrade.")
	}

	//Get list of all applications registered with upgrade manager
	*retStr = append(*retStr, "======= List of applications registered with Upgrade Manager =======")
	upgAppList := upgrade.UpgAppList(u.sdkClient)
	for obj := upgAppList.Next(); obj != nil; obj = upgAppList.Next() {
		str := "Application " + obj.GetKey() + " registered with Upgrade Manager"
		*retStr = append(*retStr, str)
	}

	//Check if Upgrade Manager is running the state machine
	*retStr = append(*retStr, "======= Checking if Upgrade Manager State Machine is running =======")
	upgstatereq := upgrade.GetUpgStateReq(u.sdkClient)
	if upgstatereq == nil {
		*retStr = append(*retStr, "Upgrade Manager not running state machine")
	} else {
		*retStr = append(*retStr, "Upgrade Manager running state machine. State is:")
		if upgstatereq.GetUpgReqState() == upgrade.UpgReqStateType_UpgStateUpgPossible {
			*retStr = append(*retStr, canUpgradeStateMachine[upgstatereq.GetUpgReqState()].upgReqStateTypeToStr)
		} else {
			*retStr = append(*retStr, upgradeStateMachine[upgstatereq.GetUpgReqState()].upgReqStateTypeToStr)
		}
	}

	//Check the status of individual applications
	*retStr = append(*retStr, "======= Checking status of all applications =======")
	upgAppRespList := upgrade.UpgAppRespList(u.sdkClient)
	for obj := upgAppRespList.Next(); obj != nil; obj = upgAppRespList.Next() {
		str := "Application " + obj.GetKey() + " has created response object"
		*retStr = append(*retStr, str)

		val := obj.GetUpgAppRespVal()
		*retStr = append(*retStr, getUpgAppRespValToStr(val))
		if !upgRespStatePassType(val) {
			*retStr = append(*retStr, obj.GetUpgAppRespStr())
		}
	}

	//Check if upgrade manager replied back to the agent
	*retStr = append(*retStr, "======= Checking status upgrade manager reply to agent =======")
	upgresp := upgrade.GetUpgResp(u.sdkClient)
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
