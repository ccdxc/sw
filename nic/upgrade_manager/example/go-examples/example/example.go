package main

import (
	"flag"
	"fmt"
	"path/filepath"
	"time"

	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk"
	"github.com/pensando/sw/venice/globals"
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

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateCompatCheck(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateCompatCheck called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStatePostBinRestart(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStatePostBinRestart called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateProcessQuiesce(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateProcessQuiesce called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateLinkDown(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateLinkDown called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateLinkUp(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateLinkUp called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateDataplaneDowntimeStart(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	//hdlrResp.Resp = upggosdk.InProgress
	hdlrResp.ErrStr = ""
	//hdlrResp.Resp = upggosdk.Fail
	//hdlrResp.ErrStr = "TATATA"
	log.Infof("HandleUpgStateDataplaneDowntimeStart called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateDataplaneDowntimePhase1(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	//hdlrResp.Resp = upggosdk.InProgress
	hdlrResp.ErrStr = ""
	//hdlrResp.Resp = upggosdk.Fail
	//hdlrResp.ErrStr = "TATATA"
	log.Infof("HandleUpgStateDataplaneDowntimePhase1 called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateDataplaneDowntimePhase2(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateDataplaneDowntimePhase2 called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateDataplaneDowntimePhase3(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateDataplaneDowntimePhase3 called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateDataplaneDowntimePhase4(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateDataplaneDowntimePhase4 called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateIsSystemReady(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateIsSystemReady called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateCleanup(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateCleanup called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateSuccess(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleUpgStateSuccess called")
	return
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateFailed(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleUpgStateFailed called")
	return
}

func (usmh *upgradeStateMachineHdlrsCtx) HandleUpgStateAbort(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleUpgStateAbort called")
	return
}

func main() {
	// command line flags
	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, "upgrade-example")), "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
	)
	flag.Parse()
	// Fill logger config params
	logConfig := &log.Config{
		Module:      "Example service",
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	s1 := &service{
		name: "Example go Service",
	}
	c1, err := gosdk.NewClient(s1)
	if err != nil {
		panic(err)
	}
	ushm := &upgradeStateMachineHdlrsCtx{}
	upg, err := upggosdk.NewUpgSdk(s1.name, c1, upggosdk.NonAgentRole, nil, ushm)
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
