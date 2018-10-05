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

func (usmh *upgradeStateMachineHdlrsCtx) CompatCheckHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("CompatCheck called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) PostRestartHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("PostRestart called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) ProcessQuiesceHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("ProcessQuiesce called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) LinkDownHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("LinkDown called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) LinkUpHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("LinkUp called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) DataplaneDowntimeStartHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	//hdlrResp.Resp = upggosdk.InProgress
	hdlrResp.ErrStr = ""
	//hdlrResp.Resp = upggosdk.Fail
	//hdlrResp.ErrStr = "TATATA"
	log.Infof("DataplaneDowntimeStart called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) DataplaneDowntimePhase1Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	//hdlrResp.Resp = upggosdk.InProgress
	hdlrResp.ErrStr = ""
	//hdlrResp.Resp = upggosdk.Fail
	//hdlrResp.ErrStr = "TATATA"
	log.Infof("DataplaneDowntimePhase1 called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) DataplaneDowntimePhase2Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("DataplaneDowntimePhase2 called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) DataplaneDowntimePhase3Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("DataplaneDowntimePhase3 called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) DataplaneDowntimePhase4Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("DataplaneDowntimePhase4 called")
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
	log.Infof("Cleanup called")
	return hdlrResp
}

func (usmh *upgradeStateMachineHdlrsCtx) SuccessHandler(upgCtx *upggosdk.UpgCtx) {
	log.Infof("Success called")
	return
}

func (usmh *upgradeStateMachineHdlrsCtx) FailedHandler(upgCtx *upggosdk.UpgCtx) {
	log.Infof("Failed called")
	return
}

func (usmh *upgradeStateMachineHdlrsCtx) AbortHandler(upgCtx *upggosdk.UpgCtx) {
	log.Infof("Abort called")
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
