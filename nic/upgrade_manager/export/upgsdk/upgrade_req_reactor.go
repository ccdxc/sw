package upgsdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

type service struct {
	name string
}

func (s *service) OnMountComplete() {
	log.Infof("OnMountComplete() done for %s\n", s.name)
}

func (s *service) Name() string {
	return s.name
}

func (s *service) OnUpgRespCreate(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespCreate called %d\n", obj.GetUpgRespVal())
}

func (s *service) OnUpgRespUpdate(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespUpdate called %d\n", obj.GetUpgRespVal())
}

func (s *service) OnUpgRespDelete(obj *upgrade.UpgResp) {
	log.Infof("OnUpgRespDelete called %d\n", obj.GetUpgRespVal())
}

//UpgRespInit init resp subtree coming from upgrade manager
func UpgRespInit(client gosdk.Client) {
	log.Infof("UpgRespInit called\n")
	s1 := &service{
		name: "UpgRespSvc",
	}
	upgrade.UpgRespMount(client, delphi.MountMode_ReadMode)
	upgrade.UpgRespWatch(client, s1)
}
