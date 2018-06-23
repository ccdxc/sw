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
	log.Infof("OnMountComplete() done for %s", s.name)
}

func (s *service) Name() string {
	return s.name
}

type upgradeStateMachineHdlrsCtx struct {
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateUpgReqRcvd(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateUpgReqRcvd called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStatePreUpgState(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStatePreUpgState called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStatePostBinRestart(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStatePostBinRestart called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateProcessesQuiesced(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateProcessesQuiesced called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateDataplaneDowntimePhase1Start(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase1Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleDataplaneDowntimeAdminQ(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleDataplaneDowntimeAdminQ called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateDataplaneDowntimePhase2Start(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase2Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateCleanup(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateCleanup called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateUpgSuccess(upgCtx *upgsdk.UpgCtx) {
	log.Infof("HandleStateUpgSuccess called")
	return
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateUpgFailed(upgCtx *upgsdk.UpgCtx) {
	log.Infof("HandleStateUpgFailed called")
	return
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateUpgAborted(upgCtx *upgsdk.UpgCtx) {
	log.Infof("HandleStateUpgAborted called")
	return
}

type upgradeCompletion struct {
}

func (u *upgradeCompletion) UpgSuccessful() {
	log.Infof("Upgupgsdk.Successful got called")
}

func (u *upgradeCompletion) UpgFailed(errStrList *[]string) {
	log.Infof("UpgFailed got called")
	for _, val := range *errStrList {
		log.Infof("Error %s", val)
	}
}

func (u *upgradeCompletion) UpgAborted(errStrList *[]string) {
	log.Infof("UpgAborted got called")
}

func (u *upgradeCompletion) UpgStatePreUpgCheckComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStatePreUpgCheckComplete got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateProcessQuiesceComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateProcessQuiesceComplete got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStatePostBinRestartComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStatePostBinRestartComplete got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateDataplaneDowntimePhase1Complete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase1Complete got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateDataplaneDowntimeAdminQComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimeAdminQComplete got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateDataplaneDowntimePhase2Complete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase2Complete got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateCleanupComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateCleanupComplete got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateAbortedComplete(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateAbortedComplete got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func main() {
	s1 := &service{
		name: "NMD go Service",
	}
	c1, err := gosdk.NewClient(s1)
	if err != nil {
		panic(err)
	}
	u1 := &upgradeCompletion{}
	ushm := &upgradeStateMachineHdlrsCtx{}
	upg, err := upgsdk.NewUpgSdk(s1.name, c1, upgsdk.AgentRole, u1, ushm)
	if err != nil {
		panic(err)
	}

	err = c1.Dial()
	if err != nil {
		log.Fatalf("Could not connect to delphi hub. Err: %v", err)
	}

	err = upg.StartUpgrade()
	if err != nil {
		log.Fatalf("Could not start upgrade because of %s", err)
	}
	timer := time.NewTimer(time.Second * 5)
	<-timer.C
	log.Infof("Timer expired")

	retStr := make([]string, 0)
	//err = upg.AbortUpgrade()
	//err = upg.GetUpgradeStatus(&retStr)
	//if upg.IsUpgradeInProgress() {
	//	log.Infof("Upgrade going on!")
	//}
	if err != nil {
		log.Fatalf("Could not abort upgrade because of %s", err)
	}
	for _, str := range retStr {
		log.Infof("%s", str)
	}
	a := make(chan struct{})
	_ = <-a
}
