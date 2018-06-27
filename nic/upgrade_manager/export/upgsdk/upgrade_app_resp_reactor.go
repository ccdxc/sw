package upgsdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

type upgapprespctx struct {
	agentHdlrs AgentHandlers
}

func getAppRespStr(obj *upgrade.UpgAppResp) string {
	return getAppRespStrUtil(obj.GetUpgAppRespVal())
}

func getAppResp(obj *upgrade.UpgAppResp, hdlrResp *HdlrResp) {
	switch obj.GetUpgAppRespVal() {
	case upgrade.UpgRespStateType_PreUpgStatePass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgRespStateType_ProcessesQuiescedPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgRespStateType_PostBinRestartPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgRespStateType_DataplaneDowntimePhase1StartPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgRespStateType_DataplaneDowntimeAdminQHandlingPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgRespStateType_DataplaneDowntimePhase2StartPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgRespStateType_CleanupPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgRespStateType_UpgAbortedPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgRespStateType_PreUpgStateFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgRespStateType_ProcessesQuiescedFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgRespStateType_PostBinRestartFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgRespStateType_DataplaneDowntimePhase1StartFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgRespStateType_DataplaneDowntimeAdminQHandlingFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgRespStateType_DataplaneDowntimePhase2StartFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgRespStateType_CleanupFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgRespStateType_UpgAbortedFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	}
}

func (ctx *upgapprespctx) invokeAgentHandler(obj *upgrade.UpgAppResp) {
	if ctx.agentHdlrs == nil {
		return
	}
	var hdlrResp HdlrResp
	getAppResp(obj, &hdlrResp)
	switch obj.GetUpgAppRespVal() {
	case upgrade.UpgRespStateType_PreUpgStatePass:
		ctx.agentHdlrs.UpgStatePreUpgCheckComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_PreUpgStateFail:
		ctx.agentHdlrs.UpgStatePreUpgCheckComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_ProcessesQuiescedPass:
		ctx.agentHdlrs.UpgStateProcessQuiesceComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_ProcessesQuiescedFail:
		ctx.agentHdlrs.UpgStateProcessQuiesceComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_PostBinRestartPass:
		ctx.agentHdlrs.UpgStatePostBinRestartComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_PostBinRestartFail:
		ctx.agentHdlrs.UpgStatePostBinRestartComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_DataplaneDowntimePhase1StartPass:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase1Complete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_DataplaneDowntimePhase1StartFail:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase1Complete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_DataplaneDowntimeAdminQHandlingPass:
		ctx.agentHdlrs.UpgStateDataplaneDowntimeAdminQComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_DataplaneDowntimeAdminQHandlingFail:
		ctx.agentHdlrs.UpgStateDataplaneDowntimeAdminQComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_DataplaneDowntimePhase2StartPass:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase2Complete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_DataplaneDowntimePhase2StartFail:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase2Complete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_CleanupPass:
		ctx.agentHdlrs.UpgStateCleanupComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_CleanupFail:
		ctx.agentHdlrs.UpgStateCleanupComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_UpgAbortedPass:
		ctx.agentHdlrs.UpgStateAbortedComplete(&hdlrResp, obj.GetKey())
	case upgrade.UpgRespStateType_UpgAbortedFail:
		ctx.agentHdlrs.UpgStateAbortedComplete(&hdlrResp, obj.GetKey())
	}
}

func (ctx *upgapprespctx) OnUpgAppRespCreate(obj *upgrade.UpgAppResp) {
	log.Infof("OnUpgAppRespCreate called %d", obj.GetUpgAppRespVal())
}

func (ctx *upgapprespctx) OnUpgAppRespUpdate(obj *upgrade.UpgAppResp) {
	log.Infof("OnUpgAppRespUpdate called %d", obj.GetUpgAppRespVal())
	ctx.invokeAgentHandler(obj)
}

func (ctx *upgapprespctx) OnUpgAppRespDelete(obj *upgrade.UpgAppResp) {
	log.Infof("OnUpgAppRespDelete called %d", obj.GetUpgAppRespVal())
}

//upgAppRespInit init resp subtree coming from upgrade applications
func upgAppRespInit(client gosdk.Client, hdlrs AgentHandlers) {
	log.Infof("upgAppRespInit called")
	ctx := &upgapprespctx{
		agentHdlrs: hdlrs,
	}
	//TODO
	//upgrade.UpgAppRespMount(client, delphi.MountMode_ReadWriteMode)
	upgrade.UpgAppRespMount(client, delphi.MountMode_ReadMode)
	upgrade.UpgAppRespWatch(client, ctx)
}
