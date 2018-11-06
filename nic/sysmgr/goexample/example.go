package main

import (
	"log"

	delphi "github.com/pensando/sw/nic/delphi/gosdk"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
)

type service struct {
	name         string
	sysmgrClient *sysmgr.Client
}

var srv = &service{
	name: "goexample",
}

func (s *service) OnMountComplete() {
	log.Printf("OnMountComplete() done for %s\n", s.name)
	s.sysmgrClient.InitDone()
}

func (s *service) Name() string {
	return s.name
}

func main() {
	delphiClient, err := delphi.NewClient(srv)
	if err != nil {
		log.Fatalf("delphi NewClient failed")
	}
	srv.sysmgrClient = sysmgr.NewClient(delphiClient, "goexample")
	go delphiClient.Run()
	forever := make(chan struct{})
	_ = <-forever
}
