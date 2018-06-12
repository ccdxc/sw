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

func main() {
	s1 := &service{
		name: "example",
	}
	c1, err := gosdk.NewClient(s1)
	if err != nil {
		panic(err)
	}
	upg, err := upgsdk.NewUpgSdk(s1.name, c1)
	if err != nil {
		panic(err)
	}

	err = c1.Dial()
	if err != nil {
		log.Fatalf("Could not connect to delphi hub. Err: %v", err)
	}

	upg.StartUpgrade()
	a := make(chan struct{})
	_ = <-a
}
