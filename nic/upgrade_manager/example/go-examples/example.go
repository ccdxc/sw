package main

import (
	"time"

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

func (u *upgradeCompletion) UpgAborted() {
	log.Infof("UpgAborted got called\n")
}

func (u *upgradeCompletion) UpgStatePreUpgCheckComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStatePreUpgCheckComplete got called with status %d error %s for service %s\n", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateProcessQuiesceComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateProcessQuiesceComplete got called with status %d error %s for service %s\n", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStatePostBinRestartComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStatePostBinRestartComplete got called with status %d error %s for service %s\n", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateDataplaneDowntimePhase1Complete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase1Complete got called with status %d error %s for service %s\n", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateDataplaneDowntimeAdminQComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimeAdminQComplete got called with status %d error %s for service %s\n", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateDataplaneDowntimePhase2Complete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase2Complete got called with status %d error %s for service %s\n", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateCleanupComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateCleanupComplete got called with status %d error %s for service %s\n", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateAbortedComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateAbortedComplete got called with status %d error %s for service %s\n", resp.Resp, resp.ErrStr, svcName)
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
	timer := time.NewTimer(time.Second * 5)
	<-timer.C
	log.Infof("Timer expired")

	//err = upg.AbortUpgrade()
	if err != nil {
		log.Fatalf("Could not abort upgrade because of %s\n", err)
	}
	a := make(chan struct{})
	_ = <-a
}
