// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"os"
	"path"
	"path/filepath"
	"runtime/debug"
	"time"

	"github.com/pensando/sw/venice/utils/events/recorder"

	"github.com/pensando/sw/nic/agent/nmd"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	"github.com/pensando/sw/nic/delphi/gosdk"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	sysmgrProto "github.com/pensando/sw/nic/sysmgr/proto/sysmgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

type service struct {
	name         string
	sysmgrClient *sysmgr.Client
}

var srv = &service{
	name: "nmd",
}

func (s *service) OnMountComplete() {
	log.Printf("OnMountComplete() done for %s\n", s.name)
	s.sysmgrClient.InitDone()
}

func (s *service) Name() string {
	return s.name
}

// Main function for Naples Management Daemon (NMD)
func main() {
	// command line flags
	var (
		nmdDbPath       = flag.String("nmddb", globals.NmdDBPath, "NMD Database file")
		regInterval     = flag.Int64("reginterval", globals.NicRegIntvl, "NIC registration interval in seconds")
		updInterval     = flag.Int64("updinterval", globals.NicUpdIntvl, "NIC update interval in seconds")
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		logToFile       = flag.String("log-to-file", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Nmd)), "Path of the log file")
		revProxyURL     = flag.String("rev-proxy-url", ":"+globals.AgentProxyPort, "specify Reverse Proxy Router REST URL")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Nmd,
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    5,
			MaxBackups: 1,
			MaxAge:     7,
		},
	}
	log.SetConfig(logConfig)

	// Initialize logger config
	logger := log.SetConfig(logConfig)

	// set Garbage collection ratio and periodically free OS memory
	debug.SetGCPercent(20)
	go func() {
		for {
			select {
			case <-time.After(time.Minute):
				// force GC and free OS memory
				debug.FreeOSMemory()
			}
		}
	}()

	//create events recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Component: globals.Nmd}, logger)
	if err != nil {
		log.Fatalf("failed to create events recorder, err: %v", err)
	}
	defer evtsRecorder.Close()

	// Create the /sysconfig/config0 if it doesn't exist. Needed for non naples nmd test environments
	if _, err := os.Stat(globals.NmdDBPath); os.IsNotExist(err) {
		os.MkdirAll(path.Dir(globals.NmdDBPath), 0664)
		os.MkdirAll(path.Dir(globals.NmdBackupDBPath), 0664)
	}

	var delphiClient clientAPI.Client
	//var uc api.UpgMgrAPI
	var dServ *nmd.DelphiService
	//if !*standalone {
	dServ = nmd.NewDelphiService()
	delphiClient, err = gosdk.NewClient(dServ)
	if err != nil {
		log.Fatalf("Error creating delphi client . Err: %v", err)
	}
	dServ.DelphiClient = delphiClient

	// mount objects
	delphiProto.NaplesStatusMount(delphiClient, delphi.MountMode_ReadWriteMode)
	log.Infof("Mounting naples status rw")

	srv.sysmgrClient = sysmgr.NewClient(delphiClient, srv.Name())
	nm, err := nmd.NewAgent(delphiClient,
		*nmdDbPath,
		globals.Localhost+":"+globals.NmdRESTPort,
		*revProxyURL,
		time.Duration(*regInterval)*time.Second,
		time.Duration(*updInterval)*time.Second,
	)
	if err != nil {
		log.Fatalf("Error creating NMD. Err: %v", err)
	}

	if delphiClient != nil {
		// mount objects
		delphiProto.NaplesStatusMount(delphiClient, delphi.MountMode_ReadWriteMode)
		log.Infof("Mounting naples status rw")

		sysmgrProto.SysmgrSystemStatusMount(delphiClient, delphi.MountMode_ReadMode)
		log.Infof("Mounting SysmgrSystemStatus")

		nm.DelphiClient = delphiClient
		dServ.Agent = nm
		go delphiClient.Run()
	}

	log.Infof("%s is running {%+v}", globals.Nmd, nm)

	// wait forever
	select {}
}
