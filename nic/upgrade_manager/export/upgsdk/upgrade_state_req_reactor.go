package upgsdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

type upgstatereqctx struct {
	appHdlrs  UpgAppHandlers
	sdkClient gosdk.Client
	appName   string
}

func (ctx *upgstatereqctx) invokeAppHdlr(reqType upgrade.UpgReqStateType, hdlrResp *HdlrResp, upgCtx *UpgCtx) {
	switch reqType {
	case upgrade.UpgReqStateType_UpgStateCompatCheck:
		log.Infof("Upgrade: Pre-upgrade check")
		*hdlrResp = ctx.appHdlrs.HandleStatePreUpgState(upgCtx)
	case upgrade.UpgReqStateType_UpgStatePostBinRestart:
		log.Infof("Upgrade: Post-binary restart")
		*hdlrResp = ctx.appHdlrs.HandleStatePostBinRestart(upgCtx)
	case upgrade.UpgReqStateType_UpgStateProcessQuiesce:
		log.Infof("Upgrade: Processes Quiesced")
		*hdlrResp = ctx.appHdlrs.HandleStateProcessesQuiesced(upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase1:
		log.Infof("Upgrade: Dataplane Downtime Phase1 Start")
		*hdlrResp = ctx.appHdlrs.HandleStateDataplaneDowntimePhase1Start(upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase2:
		log.Infof("Upgrade: Dataplane Downtime Phase2 Start")
		*hdlrResp = ctx.appHdlrs.HandleStateDataplaneDowntimePhase2Start(upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase3:
		log.Infof("Upgrade: Dataplane Downtime Phase3 Start")
		*hdlrResp = ctx.appHdlrs.HandleStateDataplaneDowntimePhase3Start(upgCtx)
	case upgrade.UpgReqStateType_UpgStateDataplaneDowntimePhase4:
		log.Infof("Upgrade: Dataplane Downtime Phase4 Start")
		*hdlrResp = ctx.appHdlrs.HandleStateDataplaneDowntimePhase4Start(upgCtx)
	case upgrade.UpgReqStateType_UpgStateCleanup:
		log.Infof("Upgrade: Cleanup Request Received")
		*hdlrResp = ctx.appHdlrs.HandleStateCleanup(upgCtx)
	case upgrade.UpgReqStateType_UpgStateSuccess:
		log.Infof("Upgrade: Succeeded")
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
		ctx.appHdlrs.HandleStateUpgSuccess(upgCtx)
	case upgrade.UpgReqStateType_UpgStateFailed:
		log.Infof("Upgrade: Failed")
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
		ctx.appHdlrs.HandleStateUpgFailed(upgCtx)
	case upgrade.UpgReqStateType_UpgStateAbort:
		log.Infof("Upgrade: Aborted")
		hdlrResp.Resp = Success
		hdlrResp.ErrStr = ""
		ctx.appHdlrs.HandleStateUpgAborted(upgCtx)
	}
}

func (ctx *upgstatereqctx) getUpgCtx(upgCtx *UpgCtx, obj *upgrade.UpgStateReq) {
	upgCtx.upgType = obj.GetUpgReqType()
}

func (ctx *upgstatereqctx) OnUpgStateReqCreate(obj *upgrade.UpgStateReq) {
	log.Infof("OnUpgStateReqCreate called")
	if canInvokeHandler(ctx.sdkClient, ctx.appName, obj.GetUpgReqState()) {
		createUpgAppResp(ctx.sdkClient, ctx.appName)
		var hdlrResp HdlrResp
		var upgctx UpgCtx
		ctx.getUpgCtx(&upgctx, obj)
		ctx.invokeAppHdlr(obj.GetUpgReqState(), &hdlrResp, &upgctx)
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
	var upgctx UpgCtx
	ctx.getUpgCtx(&upgctx, obj)
	ctx.invokeAppHdlr(obj.GetUpgReqState(), &hdlrResp, &upgctx)
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
