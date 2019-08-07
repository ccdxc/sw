package main

import (
	"flag"
	"fmt"
	"path/filepath"

	"github.com/pensando/sw/api/generated/cluster"

	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	delphi "github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/gosdk/client_api"
	dproto "github.com/pensando/sw/nic/delphi/proto/delphi"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	tsa "github.com/pensando/sw/venice/utils/techsupport"
)

type service struct {
	name         string
	sysmgrClient *sysmgr.Client
	DelphiClient clientApi.Client
	configFile   string
	tsmClient    *tsa.TSMClient
}

func (s *service) OnMountComplete() {
	log.Infof("OnMountComplete() done for %v\n", s.name)
	s.sysmgrClient.InitDone()

	// walk naples status object
	nslist := delphiProto.NaplesStatusList(s.DelphiClient)

	for _, ns := range nslist {
		log.Infof("Got Controllers : %v", ns.Controllers)
		s.handleNaplesObjectUpdates(ns)
	}
}

func (s *service) Name() string {
	return s.name
}

// OnNaplesStatusCreate event handler
func (s *service) OnNaplesStatusCreate(obj *delphiProto.NaplesStatus) {
	log.Info("Naples Status create handler")
	s.handleNaplesObjectUpdates(obj)
	return
}

// OnNaplesStatusUpdate event handler
func (s *service) OnNaplesStatusUpdate(old, new *delphiProto.NaplesStatus) {
	log.Info("Naples Status Updated")
	s.handleNaplesObjectUpdates(new)
	return
}

// OnNaplesStatusDelete event handler
func (s *service) OnNaplesStatusDelete(obj *delphiProto.NaplesStatus) {
	return
}

func (s *service) handleNaplesObjectUpdates(obj *delphiProto.NaplesStatus) {
	log.Infof("TechSupport reactor called with %v", obj)
	if obj.NaplesMode == delphiProto.NaplesStatus_NETWORK_MANAGED_INBAND || obj.NaplesMode == delphiProto.NaplesStatus_NETWORK_MANAGED_OOB {
		var controllers []string

		for _, ip := range obj.Controllers {
			controllers = append(controllers, fmt.Sprintf("%s:%s", ip, globals.CMDGRPCAuthPort))
		}

		log.Infof("Populating Venice Co-ordinates with %v", controllers)
		if s.tsmClient != nil {
			s.tsmClient.Stop()
		}

		// Create a new Techsupport Agent
		tsmClient := tsa.NewTSMClient(obj.ID, obj.Fru.MacStr, string(cluster.KindSmartNIC), s.configFile, controllers, globals.Localhost+":"+globals.NaplesTechSupportRestPort)
		s.tsmClient = tsmClient

		if s.tsmClient != nil {
			go s.tsmClient.Start()
		} else {
			log.Errorf("Initialization of TSM Client failed. Cannot start TechSupport Client")
		}
	} else {
		if s.tsmClient != nil {
			s.tsmClient.Stop()
		}
		tsmClient := tsa.NewTSMClient("naples-tsa", obj.Fru.MacStr, string(cluster.KindSmartNIC), s.configFile, nil, globals.Localhost+":"+globals.NaplesTechSupportRestPort)
		s.tsmClient = tsmClient

		if s.tsmClient != nil {
			go s.tsmClient.Start()
		} else {
			log.Errorf("Initialization of TSM Client failed. Cannot start TechSupport Client")
		}
	}
}

func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.NaplesTechSupport)), "Redirect logs to file")
		configFile      = flag.String("config", "", "Config file for Tech Support Agent")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.NaplesTechSupport,
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
	logger := log.SetConfig(logConfig)
	defer logger.Close()
	log.Info("Log configuration set.")

	var delphiService = &service{
		name:       globals.NaplesTechSupport,
		configFile: *configFile,
	}

	// Plugging in default values when the agent is created.
	tsmClient := tsa.NewTSMClient("naples-tsa", "NA", string(cluster.KindSmartNIC), delphiService.configFile, nil, globals.Localhost+":"+globals.NaplesTechSupportRestPort)
	delphiService.tsmClient = tsmClient

	if delphiService.tsmClient != nil {
		go delphiService.tsmClient.Start()
	} else {
		log.Errorf("Initialization of TSM Client failed. Cannot start TechSupport Client")
	}

	delphiClient, err := delphi.NewClient(delphiService)
	if err != nil {
		log.Fatalf("delphi NewClient failed")
	}
	delphiService.DelphiClient = delphiClient
	delphiService.sysmgrClient = sysmgr.NewClient(delphiClient, delphiService.Name())

	// Mount delphi naples status object
	delphiProto.NaplesStatusMount(delphiClient, dproto.MountMode_ReadMode)

	// Set up watches
	delphiProto.NaplesStatusWatch(delphiService.DelphiClient, delphiService)

	// run delphi thread in background
	go delphiService.DelphiClient.Run()

	log.Info("Started Delphiclient")

	// wait forever
	select {}
}
