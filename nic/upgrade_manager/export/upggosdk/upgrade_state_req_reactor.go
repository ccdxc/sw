package upggosdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

type upgstatereqctx struct {
	appHdlrs  UpgAppHandlers
	sdkClient clientApi.Client
	appName   string
}

var upgCtx UpgCtx

func (ctx *upgstatereqctx) invokeAppHdlr(reqType upgrade.UpgReqStateType, hdlrResp *HdlrResp) {
	switch reqType {
	case upgrade.UpgReqStateType_UpgStateUpgPossible:
		log.Infof("Upgrade: Pre-upgrade check for UpgStateUpgPossible")
		*hdlrResp = ctx.appHdlrs.CompatCheckHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateCompatCheck:
		log.Infof("Upgrade: Pre-upgrade check")
		*hdlrResp = ctx.appHdlrs.CompatCheckHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateLinkDown:
		log.Infof("Upgrade: Link Down")
		*hdlrResp = ctx.appHdlrs.LinkDownHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateLinkUp:
		log.Infof("Upgrade: Link Up")
		*hdlrResp = ctx.appHdlrs.LinkUpHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStatePostRestart:
		log.Infof("Upgrade: Post-binary restart")
		*hdlrResp = ctx.appHdlrs.PostRestartHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateProcessQuiesce:
		log.Infof("Upgrade: Processes Quiesced")
		*hdlrResp = ctx.appHdlrs.ProcessQuiesceHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase1:
		log.Infof("Upgrade: Dataplane Downtime Phase1 Start")
		*hdlrResp = ctx.appHdlrs.DataplaneDowntimePhase1Handler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase2:
		log.Infof("Upgrade: Dataplane Downtime Phase2 Start")
		*hdlrResp = ctx.appHdlrs.DataplaneDowntimePhase2Handler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase3:
		log.Infof("Upgrade: Dataplane Downtime Phase3 Start")
		*hdlrResp = ctx.appHdlrs.DataplaneDowntimePhase3Handler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase4:
		log.Infof("Upgrade: Dataplane Downtime Phase4 Start")
		*hdlrResp = ctx.appHdlrs.DataplaneDowntimePhase4Handler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateHostDown:
		log.Infof("Upgrade: Host Down")
		*hdlrResp = ctx.appHdlrs.HostDownHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateHostUp:
		log.Infof("Upgrade: Host Up")
		*hdlrResp = ctx.appHdlrs.HostUpHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStatePostHostDown:
		log.Infof("Upgrade: Post Host Down")
		*hdlrResp = ctx.appHdlrs.PostHostDownHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStatePostLinkUp:
		log.Infof("Upgrade: Post Link Up")
		*hdlrResp = ctx.appHdlrs.PostLinkUpHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateSaveState:
		log.Infof("Upgrade: Save State")
		*hdlrResp = ctx.appHdlrs.SaveStateHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateSuccess:
		log.Infof("Upgrade: Succeeded")
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
		ctx.appHdlrs.SuccessHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateFailed:
		log.Infof("Upgrade: Failed")
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
		ctx.appHdlrs.FailedHandler(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateAbort:
		log.Infof("Upgrade: Aborted")
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
		ctx.appHdlrs.AbortHandler(&upgCtx)
	}
}

func (ctx *upgstatereqctx) getUpgCtx(obj *upgrade.UpgStateReq) error {
	upgCtx.upgType = obj.GetUpgReqType()
	return getUpgCtxFromMeta(&upgCtx)
}

func (ctx *upgstatereqctx) OnUpgStateReqCreate(obj *upgrade.UpgStateReq) {
	log.Infof("OnUpgStateReqCreate called")
	err := ctx.getUpgCtx(obj)
	if err != nil {
		log.Infof("getUpgCtxFromMeta failed")
		return
	}
	if canInvokeHandler(ctx.sdkClient, ctx.appName, obj.GetUpgReqState()) {
		createUpgAppResp(ctx.sdkClient, ctx.appName)
		var hdlrResp HdlrResp
		ctx.invokeAppHdlr(obj.GetUpgReqState(), &hdlrResp)
		if hdlrResp.Resp != InProgress {
			updateUpgAppResp(getUpgAppRespNext(obj.GetUpgReqState(), (hdlrResp.Resp == Success)), &hdlrResp, ctx.appName, ctx.sdkClient)
		} else {
			log.Infof("Application still processing")
		}
	}
}

func (ctx *upgstatereqctx) OnUpgStateReqUpdate(old, obj *upgrade.UpgStateReq) {
	if obj.GetUpgReqState() != upgrade.UpgReqStateType_UpgStateTerminal {
		log.Infof("===== Incoming Message =====")
		err := ctx.getUpgCtx(obj)
		if err != nil {
			log.Infof("getUpgCtxFromMeta failed")
			return
		}
		var hdlrResp HdlrResp
		ctx.invokeAppHdlr(obj.GetUpgReqState(), &hdlrResp)
		if hdlrResp.Resp != InProgress {
			updateUpgAppResp(getUpgAppRespNext(obj.GetUpgReqState(), (hdlrResp.Resp == Success)), &hdlrResp, ctx.appName, ctx.sdkClient)
		} else {
			log.Infof("Application still processing")
		}
	}
}

func (ctx *upgstatereqctx) OnUpgStateReqDelete(obj *upgrade.UpgStateReq) {
	log.Infof("OnUpgStateReqDelete called")
	deleteUpgAppResp(ctx.appName, ctx.sdkClient)
}

func upgStateReqInit(client clientApi.Client, hdlrs UpgAppHandlers, name string) {
	log.Infof("UpgRespInit called")
	ctx := &upgstatereqctx{
		appHdlrs:  hdlrs,
		sdkClient: client,
		appName:   name,
	}
	upgrade.UpgStateReqMount(client, delphi.MountMode_ReadMode)
	upgrade.UpgStateReqWatch(client, ctx)

	client.WatchMount(ctx)
}

func (ctx *upgstatereqctx) OnMountComplete() {
	registerUpgApp(ctx.sdkClient, ctx.appName)
	log.Infof("OnMountComplete got called to restore UpgStateReq")
	upgstatereq := upgrade.GetUpgStateReq(ctx.sdkClient)
	if upgstatereq == nil {
		log.Infof("OnMountComplete could not find UpgStateReq")
		return
	}
	ctx.OnUpgStateReqCreate(upgstatereq)
}
