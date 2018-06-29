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

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateCompatCheck(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStatePreUpgState called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStatePostBinRestart(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStatePostBinRestart called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateProcessQuiesce(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateProcessesQuiesced called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateDataplaneDowntimePhase1(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase1Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateDataplaneDowntimePhase2(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase2Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateDataplaneDowntimePhase3(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase3Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateDataplaneDowntimePhase4(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase4Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateCleanup(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateCleanup called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateSuccess(upgCtx *upgsdk.UpgCtx) {
	log.Infof("HandleStateUpgSuccess called")
	return
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateFailed(upgCtx *upgsdk.UpgCtx) {
	log.Infof("HandleStateUpgFailed called")
	return
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateAbort(upgCtx *upgsdk.UpgCtx) {
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

func (u *upgradeCompletion) UpgStateCompatCheckCompletionHandler(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateCompatCheckCompletionHandlergot called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateProcessQuiesceCompletionHandler(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateProcessQuiesceCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStatePostBinRestartCompletionHandler(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStatePostBinRestartCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateDataplaneDowntimePhase1CompletionHandler(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase1CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateDataplaneDowntimePhase2CompletionHandler(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase2CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateDataplaneDowntimePhase3CompletionHandler(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase3CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateDataplaneDowntimePhase4CompletionHandler(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase4CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateCleanupCompletionHandler(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateCleanupCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) UpgStateAbortCompletionHandler(resp *upgsdk.HdlrResp, svcName string) {
	log.Infof("UpgStateAbortCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
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
