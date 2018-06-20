package upgsdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

type upgrespctx struct {
	agentHdlrs AgentHandlers
	sdkClient  gosdk.Client
}

func (ctx *upgrespctx) invokeAgentHandler(respType upgrade.UpgRespType) {
	switch respType {
	case upgrade.UpgRespType_UpgRespPass:
		log.Infof("upgrade success!!")
		ctx.agentHdlrs.UpgSuccessful()
	case upgrade.UpgRespType_UpgRespFail:
		log.Infof("upgrade failed!!")
		//TODO for (int i=0; i<resp->upgrespfailstr_size(); i++) {
		ctx.agentHdlrs.UpgFailed()
	case upgrade.UpgRespType_UpgRespAbort:
		log.Infof("upgrade aborted!!")
		//TODO for (int i=0; i<resp->upgrespfailstr_size(); i++) {
		ctx.agentHdlrs.UpgAborted()
	}
}

func (ctx *upgrespctx) DeleteUpgReqSpec() {
	upgReq := upgrade.GetUpgReq(ctx.sdkClient, 10)
	upgReq.Delete()
}

func (ctx *upgrespctx) OnUpgRespCreate(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespCreate called %d", obj.GetUpgRespVal())
	ctx.invokeAgentHandler(obj.GetUpgRespVal())
	ctx.DeleteUpgReqSpec()
}

func (ctx *upgrespctx) OnUpgRespUpdate(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespUpdate called %d", obj.GetUpgRespVal())
	ctx.invokeAgentHandler(obj.GetUpgRespVal())
	ctx.DeleteUpgReqSpec()
}

func (ctx *upgrespctx) OnUpgRespDelete(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespDelete called %d", obj.GetUpgRespVal())
}

//upgRespInit init resp subtree coming from upgrade manager
func upgRespInit(client gosdk.Client, hdlrs AgentHandlers) {
	log.Infof("upgRespInit called")
	ctx := &upgrespctx{
		agentHdlrs: hdlrs,
		sdkClient:  client,
	}
	upgrade.UpgRespMount(client, delphi.MountMode_ReadMode)
	upgrade.UpgRespWatch(client, ctx)
}
