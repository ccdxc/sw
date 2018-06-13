package upgsdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

type upgrespctx struct {
	name       string
	agentHdlrs AgentHandlers
}

func (ctx *upgrespctx) OnMountComplete() {
	log.Infof("OnMountComplete() done for %s\n", ctx.name)
}

func (ctx *upgrespctx) Name() string {
	return ctx.name
}

func (ctx *upgrespctx) invokeAgentHandler(respType upgrade.UpgRespType) {
	switch respType {
	case upgrade.UpgRespType_UpgRespPass:
		log.Infof("upgrade success!!\n")
		ctx.agentHdlrs.UpgSuccessful()
	case upgrade.UpgRespType_UpgRespFail:
		log.Infof("upgrade failed!!\n")
		ctx.agentHdlrs.UpgFailed()
	case upgrade.UpgRespType_UpgRespAbort:
		log.Infof("upgrade aborted!!\n")
	}
}

func (ctx *upgrespctx) OnUpgRespCreate(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespCreate called %d\n", obj.GetUpgRespVal())
	ctx.invokeAgentHandler(obj.GetUpgRespVal())
}

func (ctx *upgrespctx) OnUpgRespUpdate(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespUpdate called %d\n", obj.GetUpgRespVal())
	ctx.invokeAgentHandler(obj.GetUpgRespVal())
}

func (ctx *upgrespctx) OnUpgRespDelete(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespDelete called %d\n", obj.GetUpgRespVal())
}

//UpgRespInit init resp subtree coming from upgrade manager
func UpgRespInit(client gosdk.Client, hdlrs AgentHandlers) {
	log.Infof("UpgRespInit called\n")
	ctx := &upgrespctx{
		name:       "UpgRespSvc",
		agentHdlrs: hdlrs,
	}
	upgrade.UpgRespMount(client, delphi.MountMode_ReadMode)
	upgrade.UpgRespWatch(client, ctx)
}
