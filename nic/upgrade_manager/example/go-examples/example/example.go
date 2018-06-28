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
	//hdlrResp.Resp = upgsdk.InProgress
	hdlrResp.ErrStr = ""
	//hdlrResp.Resp = upgsdk.Fail
	//hdlrResp.ErrStr = "TATATA"
	log.Infof("HandleStateDataplaneDowntimePhase1Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateDataplaneDowntimePhase2Start(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase2Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateDataplaneDowntimePhase3Start(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase3Start called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleStateDataplaneDowntimePhase4Start(upgCtx *upgsdk.UpgCtx) upgsdk.HdlrResp {
	var hdlrResp upgsdk.HdlrResp
	hdlrResp.Resp = upgsdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase4Start called")
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

func main() {
	s1 := &service{
		name: "Example go Service",
	}
	c1, err := gosdk.NewClient(s1)
	if err != nil {
		panic(err)
	}
	ushm := &upgradeStateMachineHdlrsCtx{}
	upg, err := upgsdk.NewUpgSdk(s1.name, c1, upgsdk.NonAgentRole, nil, ushm)
	if err != nil {
		panic(err)
	}
	_ = upg
	err = c1.Dial()
	if err != nil {
		log.Fatalf("Could not connect to delphi hub. Err: %v", err)
	}

	timer := time.NewTimer(time.Second * 20)
	<-timer.C
	log.Infof("Timer expired")

	//err = upg.SendAppRespSuccess()
	//err = upg.GetUpgradeStatus(&retStr)
	if err != nil {
		log.Fatalf("Could not send success  %s", err)
	}

	a := make(chan struct{})
	_ = <-a
}
