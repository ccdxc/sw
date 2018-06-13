package main

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk"
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

type upgradeCompletion struct {
}

func (u *upgradeCompletion) UpgSuccessful() {
	log.Infof("UpgSuccessful got called\n")
}

func (u *upgradeCompletion) UpgFailed() {
	log.Infof("UpgFailed got called\n")
}

func main() {
	s1 := &service{
		name: "example",
	}
	c1, err := gosdk.NewClient(s1)
	if err != nil {
		panic(err)
	}
	u1 := &upgradeCompletion{}
	upg, err := upgsdk.NewUpgSdk(s1.name, c1, upgsdk.AgentRole, u1)
	if err != nil {
		panic(err)
	}

	err = c1.Dial()
	if err != nil {
		log.Fatalf("Could not connect to delphi hub. Err: %v", err)
	}

	err = upg.StartUpgrade()
	if err != nil {
		log.Fatalf("Could not start upgrade because of %s\n", err)
	}
	a := make(chan struct{})
	_ = <-a
}
