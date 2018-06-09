package main

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	upgrade "github.com/pensando/sw/nic/upgrade_manager/example/go-examples/nic/upgrade_manager/proto"
	"github.com/pensando/sw/venice/utils/log"
)

type service struct {
	name string
}

func (s *service) OnMountComplete() {
	log.Printf("OnMountComplete() done for %s\n", s.name)
}

func (s *service) Name() string {
	return s.name
}

func (s *service) OnUpgRespCreate(obj *upgrade.UpgResp) {
	log.Printf("OnUpgRespCreate called %d\n", obj.GetUpgRespVal())
}

func (s *service) OnUpgRespUpdate(obj *upgrade.UpgResp) {
	log.Printf("OnUpgRespUpdate called %d\n", obj.GetUpgRespVal())
}

func (s *service) OnUpgRespDelete(obj *upgrade.UpgResp) {
	log.Printf("OnUpgRespDelete called %d\n", obj.GetUpgRespVal())
}

func main() {
	s1 := &service{
		name: "example",
	}
	c1, err := gosdk.NewClient(s1)
	if err != nil {
		panic(err)
	}
	upgrade.UpgReqMount(c1, delphi.MountMode_ReadWriteMode)
	upgrade.UpgRespMount(c1, delphi.MountMode_ReadMode)
	upgrade.UpgRespWatch(c1, s1)
	err = c1.Dial()
	if err != nil {
		log.Fatalf("Could not connect to upgrade manager. Err: %v", err)
	}
	u := upgrade.NewUpgReq(c1)
	u.SetKey(10)
	u.SetUpgReqCmd(upgrade.UpgReqType_UpgStart)
	a := make(chan struct{})
	_ = <-a
}
