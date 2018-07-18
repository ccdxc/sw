package upggosdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

type upgstatereqctx struct {
	appHdlrs  UpgAppHandlers
	sdkClient gosdk.Client
	appName   string
}

var upgCtx UpgCtx

func (ctx *upgstatereqctx) invokeAppHdlr(reqType upgrade.UpgReqStateType, hdlrResp *HdlrResp) {
	switch reqType {
	case upgrade.UpgReqStateType_UpgStateCompatCheck:
		log.Infof("Upgrade: Pre-upgrade check")
		*hdlrResp = ctx.appHdlrs.HandleUpgStateCompatCheck(&upgCtx)
	case upgrade.UpgReqStateType_UpgStatePostBinRestart:
		log.Infof("Upgrade: Post-binary restart")
		*hdlrResp = ctx.appHdlrs.HandleUpgStatePostBinRestart(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateProcessQuiesce:
		log.Infof("Upgrade: Processes Quiesced")
		*hdlrResp = ctx.appHdlrs.HandleUpgStateProcessQuiesce(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase1:
		log.Infof("Upgrade: Dataplane Downtime Phase1 Start")
		*hdlrResp = ctx.appHdlrs.HandleUpgStateDataplaneDowntimePhase1(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase2:
		log.Infof("Upgrade: Dataplane Downtime Phase2 Start")
		*hdlrResp = ctx.appHdlrs.HandleUpgStateDataplaneDowntimePhase2(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase3:
		log.Infof("Upgrade: Dataplane Downtime Phase3 Start")
		*hdlrResp = ctx.appHdlrs.HandleUpgStateDataplaneDowntimePhase3(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase4:
		log.Infof("Upgrade: Dataplane Downtime Phase4 Start")
		*hdlrResp = ctx.appHdlrs.HandleUpgStateDataplaneDowntimePhase4(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateCleanup:
		log.Infof("Upgrade: Cleanup Request Received")
		*hdlrResp = ctx.appHdlrs.HandleUpgStateCleanup(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateSuccess:
		log.Infof("Upgrade: Succeeded")
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
		ctx.appHdlrs.HandleUpgStateSuccess(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateFailed:
		log.Infof("Upgrade: Failed")
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
		ctx.appHdlrs.HandleUpgStateFailed(&upgCtx)
	case upgrade.UpgReqStateType_UpgStateAbort:
		log.Infof("Upgrade: Aborted")
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
		ctx.appHdlrs.HandleUpgStateAbort(&upgCtx)
	}
}

func (ctx *upgstatereqctx) getUpgCtx(obj *upgrade.UpgStateReq) {
	upgCtx.upgType = obj.GetUpgReqType()
	getUpgCtxFromMeta(&upgCtx)
}

func (ctx *upgstatereqctx) OnUpgStateReqCreate(obj *upgrade.UpgStateReq) {
	log.Infof("OnUpgStateReqCreate called")
	if canInvokeHandler(ctx.sdkClient, ctx.appName, obj.GetUpgReqState()) {
		createUpgAppResp(ctx.sdkClient, ctx.appName)
		var hdlrResp HdlrResp
		ctx.getUpgCtx(obj)
		ctx.invokeAppHdlr(obj.GetUpgReqState(), &hdlrResp)
		if hdlrResp.Resp != InProgress {
			updateUpgAppResp(getUpgAppRespNext(obj.GetUpgReqState(), (hdlrResp.Resp == Success)), &hdlrResp, ctx.appName, ctx.sdkClient)
		} else {
			log.Infof("Application still processing")
		}
	}
}

func (ctx *upgstatereqctx) OnUpgStateReqUpdate(obj *upgrade.UpgStateReq) {
	if obj.GetUpgReqState() != upgrade.UpgReqStateType_UpgStateTerminal {
		log.Infof("===== Incoming Message =====")
	}
	var hdlrResp HdlrResp
	ctx.invokeAppHdlr(obj.GetUpgReqState(), &hdlrResp)
	if hdlrResp.Resp != InProgress {
		updateUpgAppResp(getUpgAppRespNext(obj.GetUpgReqState(), (hdlrResp.Resp == Success)), &hdlrResp, ctx.appName, ctx.sdkClient)
	} else {
		log.Infof("Application still processing")
	}
}

func (ctx *upgstatereqctx) OnUpgStateReqDelete(obj *upgrade.UpgStateReq) {
	log.Infof("OnUpgStateReqDelete called")
	deleteUpgAppResp(ctx.appName, ctx.sdkClient)
}

func upgStateReqInit(client gosdk.Client, hdlrs UpgAppHandlers, name string) {
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
	upgstatereq := upgrade.GetUpgStateReq(ctx.sdkClient, 10)
	if upgstatereq == nil {
		log.Infof("OnMountComplete could not find UpgStateReq")
		return
	}
	ctx.OnUpgStateReqCreate(upgstatereq)
}
