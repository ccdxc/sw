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
	case upgrade.UpgStateRespType_UpgStateCompatCheckRespPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgStateRespType_UpgStateProcessQuiesceRespPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgStateRespType_UpgStatePostBinRestartRespPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase1RespPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase2RespPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase3RespPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase4RespPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgStateRespType_UpgStateCleanupRespPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgStateRespType_UpgStateAbortRespPass:
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
	case upgrade.UpgStateRespType_UpgStateCompatCheckRespFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgStateRespType_UpgStateProcessQuiesceRespFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgStateRespType_UpgStatePostBinRestartRespFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase1RespFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase2RespFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase3RespFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase4RespFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgStateRespType_UpgStateCleanupRespFail:
		hdlrResp.Resp = Fail
		hdlrResp.ErrStr = getAppRespStr(obj)
	case upgrade.UpgStateRespType_UpgStateAbortRespFail:
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
	case upgrade.UpgStateRespType_UpgStateCompatCheckRespPass:
		ctx.agentHdlrs.UpgStateCompatCheckCompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateCompatCheckRespFail:
		ctx.agentHdlrs.UpgStateCompatCheckCompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateProcessQuiesceRespPass:
		ctx.agentHdlrs.UpgStateProcessQuiesceCompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateProcessQuiesceRespFail:
		ctx.agentHdlrs.UpgStateProcessQuiesceCompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStatePostBinRestartRespPass:
		ctx.agentHdlrs.UpgStatePostBinRestartCompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStatePostBinRestartRespFail:
		ctx.agentHdlrs.UpgStatePostBinRestartCompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase1RespPass:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase1CompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase1RespFail:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase1CompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase2RespPass:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase2CompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase2RespFail:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase2CompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase3RespPass:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase3CompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase3RespFail:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase3CompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase4RespPass:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase4CompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateDataplaneDowntimePhase4RespFail:
		ctx.agentHdlrs.UpgStateDataplaneDowntimePhase4CompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateCleanupRespPass:
		ctx.agentHdlrs.UpgStateCleanupCompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateCleanupRespFail:
		ctx.agentHdlrs.UpgStateCleanupCompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateAbortRespPass:
		ctx.agentHdlrs.UpgStateAbortCompletionHandler(&hdlrResp, obj.GetKey())
	case upgrade.UpgStateRespType_UpgStateAbortRespFail:
		ctx.agentHdlrs.UpgStateAbortCompletionHandler(&hdlrResp, obj.GetKey())
	}
}

func (ctx *upgapprespctx) OnUpgAppRespCreate(obj *upgrade.UpgAppResp) {
	log.Infof("OnUpgAppRespCreate called %d", obj.GetUpgAppRespVal())
	ctx.invokeAgentHandler(obj)
}

func (ctx *upgapprespctx) OnUpgAppRespUpdate(obj *upgrade.UpgAppResp) {
	log.Infof("OnUpgAppRespUpdate called %d", obj.GetUpgAppRespVal())
	ctx.invokeAgentHandler(obj)
}

func (ctx *upgapprespctx) OnUpgAppRespDelete(obj *upgrade.UpgAppResp) {
	log.Infof("OnUpgAppRespDelete called %d", obj.GetUpgAppRespVal())
}

//upgAppRespInit init resp subtree coming from upgrade applications
func upgAppRespInit(client gosdk.Client, hdlrs AgentHandlers, name string) {
	log.Infof("upgAppRespInit called")
	ctx := &upgapprespctx{
		agentHdlrs: hdlrs,
	}
	//TODO
	//upgrade.UpgAppRespMount(client, delphi.MountMode_ReadWriteMode)
	upgrade.UpgAppRespMountKey(client, name, delphi.MountMode_ReadMode)
	upgrade.UpgAppRespWatch(client, ctx)
}
