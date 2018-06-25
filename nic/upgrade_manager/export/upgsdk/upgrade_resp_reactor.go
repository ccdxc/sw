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

func (ctx *upgrespctx) invokeAgentHandler(obj *upgrade.UpgResp) {
	var errStrList []string
	switch obj.GetUpgRespVal() {
	case upgrade.UpgRespType_UpgRespPass:
		log.Infof("upgrade success!!")
		ctx.agentHdlrs.UpgSuccessful()
	case upgrade.UpgRespType_UpgRespFail:
		log.Infof("upgrade failed!!")
		for idx := 0; idx < obj.GetUpgRespFailStr().Length(); idx++ {
			errStrList = append(errStrList, obj.GetUpgRespFailStr().Get(idx))
		}
		ctx.agentHdlrs.UpgFailed(&errStrList)
	case upgrade.UpgRespType_UpgRespAbort:
		log.Infof("upgrade aborted!!")
		for idx := 0; idx < obj.GetUpgRespFailStr().Length(); idx++ {
			errStrList = append(errStrList, obj.GetUpgRespFailStr().Get(idx))
		}
		ctx.agentHdlrs.UpgAborted(&errStrList)
	}
}

func (ctx *upgrespctx) deleteUpgReqSpec() {
	upgReq := upgrade.GetUpgReq(ctx.sdkClient, 10)
	upgReq.Delete()
}

func (ctx *upgrespctx) OnUpgRespCreate(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespCreate called %d", obj.GetUpgRespVal())
	ctx.invokeAgentHandler(obj)
	ctx.deleteUpgReqSpec()
}

func (ctx *upgrespctx) OnUpgRespUpdate(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespUpdate called %d", obj.GetUpgRespVal())
	ctx.invokeAgentHandler(obj)
	ctx.deleteUpgReqSpec()
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
