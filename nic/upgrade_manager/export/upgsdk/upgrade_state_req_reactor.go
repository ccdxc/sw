package upgsdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

type upgstatereqctx struct {
	hdlrs     UpgAppHandlers
	sdkClient gosdk.Client
	appName   string
}

func (ctx *upgstatereqctx) invokeAppHdlr(reqType upgrade.UpgReqStateType, hdlrResp *HdlrResp, upgCtx *UpgCtx) {
	hdlrResp.Resp = Success
	hdlrResp.ErrStr = ""
}

func (ctx *upgstatereqctx) getUpgCtx(upgCtx *UpgCtx, obj *upgrade.UpgStateReq) {
	upgCtx.upgType = obj.GetUpgReqType()
}

func (ctx *upgstatereqctx) OnUpgStateReqCreate(obj *upgrade.UpgStateReq) {
	log.Infof("OnUpgStateReqCreate got created")
	if canInvokeHandler(obj.GetUpgReqState()) {
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

func upgStateReqInit(client gosdk.Client /*hdlrs UpgAppHandlers,*/, name string) {
	log.Infof("UpgRespInit called")
	ctx := &upgstatereqctx{
		//hdlrs:     hdlrs,
		sdkClient: client,
		appName:   name,
	}
	upgrade.UpgStateReqMount(client, delphi.MountMode_ReadMode)
	upgrade.UpgStateReqWatch(client, ctx)
	//WatchMountComplete
}
