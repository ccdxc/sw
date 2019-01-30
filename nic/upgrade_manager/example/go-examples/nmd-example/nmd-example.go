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
	ver, err := upggosdk.UpgCtxGetPreUpgTableVersion(upgCtx, upggosdk.NICMGR)
	if err != nil {
		log.Infof("Error from API %s", err)
	}
	log.Infof("Pre-Version for NICMGR is %s", ver)
	ver, err = upggosdk.UpgCtxGetPostUpgTableVersion(upgCtx, upggosdk.NICMGR)
	if err != nil {
		log.Infof("Error from API %s", err)
	}
	log.Infof("Post-Version for NICMGR is %s", ver)
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

func (usmh *upgradeStateMachineHdlrsCtx) HostUpHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HostUp called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HostDownHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HostDown called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) PostHostDownHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("PostHostDown called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) SaveStateHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("SaveState called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) PostLinkUpHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("PostLinkUp called")
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
	upg.StartDisruptiveUpgrade("naples_fw.tar")
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
	log.Infof("UpgStateCompatCheckCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
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

func (u *upgradeCompletion) HostDownCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("HostDownCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) HostUpCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("HostUpCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) PostHostDownCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("PostHostDownCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) PostLinkUpCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("PostLinkUpCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

func (u *upgradeCompletion) SaveStateCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("SaveStateCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
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

	go c1.Run()

	err = upg.CanPerformDisruptiveUpgrade("naples_fw.tar")
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
