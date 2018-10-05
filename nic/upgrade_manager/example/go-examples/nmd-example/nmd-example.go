package main

import (
	"time"

	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk"
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

func (usmh *upgradeStateMachineHdlrsCtx) CompatCheckHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	log.Infof("HandleStatePreUpgState called")
	ver, err := upggosdk.UpgCtxGetPreUpgTableVersion(upgCtx, "SAMPLETABLE-1")
	if err != nil {
		log.Infof("Error from API %s", err)
	}
	log.Infof("Version for SAMPLETABLE-1 table is %d", ver)
	ver, err = upggosdk.UpgCtxGetPreUpgTableVersion(upgCtx, "TABLE-10")
	if err != nil {
		log.Infof("Error from API %s", err)
	}
	ver, err = upggosdk.UpgCtxGetPreUpgTableVersion(upgCtx, "")
	if err != nil {
		log.Infof("Error from API %s", err)
	}
	ver, err = upggosdk.UpgCtxGetPreUpgComponentVersion(upgCtx, "COMPONENT-1")
	if err != nil {
		log.Infof("Error from API %s", err)
	}
	log.Infof("Version for SAMPLECOMPONENT-1 table is %d", ver)
	ver, err = upggosdk.UpgCtxGetPreUpgComponentVersion(upgCtx, "COMPONENT-5")
	if err != nil {
		log.Infof("Error from API %s", err)
	}
	ver, err = upggosdk.UpgCtxGetPreUpgComponentVersion(upgCtx, "")
	if err != nil {
		log.Infof("Error from API %s", err)
	}
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) PostRestartHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStatePostRestart called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) ProcessQuiesceHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateProcessesQuiesced called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) LinkDownHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateLinkDown called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) LinkUpHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateLinkUp called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) DataplaneDowntimeStartHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("DataplaneDowntimeStart called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) DataplaneDowntimePhase1Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase1Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) DataplaneDowntimePhase2Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase2Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) DataplaneDowntimePhase3Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase3Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) DataplaneDowntimePhase4Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase4Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) IsSystemReadyHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("IsSystemReady called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) CleanupHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateCleanup called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) SuccessHandler(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleStateUpgSuccess called")
	return
}

func (usmh *upgradeStateMachineHdlrsCtx) FailedHandler(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleStateUpgFailed called")
	return
}

func (usmh *upgradeStateMachineHdlrsCtx) AbortHandler(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleStateUpgAborted called")
	return
}

type upgradeCompletion struct {
}

func (u *upgradeCompletion) UpgSuccessful() {
	log.Infof("Upgupggosdk.Successful got called")
}

func (u *upgradeCompletion) UpgPossible(upgCtx *upggosdk.UpgCtx) {
	log.Infof("UpgPossible got called")
	//upg.StartNonDisruptiveUpgrade()
	upg.StartDisruptiveUpgrade()
}

func (u *upgradeCompletion) UpgNotPossible(upgCtx *upggosdk.UpgCtx, errStrList *[]string) {
	log.Infof("UpgNotPossible got called")
	for _, val := range *errStrList {
		log.Infof("Error %s", val)
	}
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

func (u *upgradeCompletion) CompatCheckCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateCompatCheckCompletionHandlergot called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) ProcessQuiesceCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateProcessQuiesceCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) LinkDownCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateLinkDownCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) LinkUpCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateLinkUpCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) PostRestartCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStatePostRestartCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) DataplaneDowntimeStartCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimeStartCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) DataplaneDowntimePhase1CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase1CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) DataplaneDowntimePhase2CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase2CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) DataplaneDowntimePhase3CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase3CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) DataplaneDowntimePhase4CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase4CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) IsSystemReadyCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateIsSystemReadyCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) CleanupCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateCleanupCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) AbortCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateAbortCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

var upg upggosdk.UpgSdk
var err error

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
	upg, err = upggosdk.NewUpgSdk(s1.name, c1, upggosdk.AgentRole, u1, ushm)
	if err != nil {
		panic(err)
	}

	err = c1.Dial()
	if err != nil {
		log.Fatalf("Could not connect to delphi hub. Err: %v", err)
	}

	err = upg.CanPerformNonDisruptiveUpgrade()
	//err = upg.StartNonDisruptiveUpgrade()
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
