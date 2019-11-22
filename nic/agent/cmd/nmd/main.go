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
	"github.com/pensando/sw/nic/agent/nmd/pipeline"
	"github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/nic/agent/nmd/state/ipif"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// Main function for Naples Management Daemon (NMD)
func main() {
	// command line flags
	var (
		nmdDbPath       = flag.String("nmddb", globals.NmdDBPath, "NMD Database file")
		regInterval     = flag.Int64("reginterval", globals.NicRegIntvl, "NIC registration interval in seconds")
		updInterval     = flag.Int64("updinterval", globals.NicUpdIntvl, "NIC update interval in seconds")
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		oobInterface    = flag.String("oob-interface", "", "naples oob interface")
		inbInterface    = flag.String("inb-interface", "", "naples inband interface")
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
			MaxSize:    2,
			MaxBackups: 1,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	logger := log.SetConfig(logConfig)
	defer logger.Close()

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
	}

	// Create the /sysconfig/config1 if it doesn't exist. Needed for non naples nmd test environments
	if _, err := os.Stat(globals.NmdBackupDBPath); os.IsNotExist(err) {
		os.MkdirAll(path.Dir(globals.NmdBackupDBPath), 0664)
	}

	pipelineType := globals.NaplesPipelineIris
	if val, ok := os.LookupEnv("NAPLES_PIPELINE"); ok {
		pipelineType = val
	}
	p, err := pipeline.NewPipeline(state.Kind(pipelineType))
	if err != nil {
		log.Fatalf("Error creating setting up pipeline. Err: %v", err)
	}

	p.InitDelphi()

	// init sysmgr
	p.InitSysmgr()

	if *oobInterface != "" {
		ipif.NaplesOOBInterface = *oobInterface
	}
	if *inbInterface != "" {
		ipif.NaplesInbandInterface = *inbInterface
	}
	nm, err := nmd.NewAgent(p,
		*nmdDbPath,
		globals.Localhost+":"+globals.NmdRESTPort,
		*revProxyURL,
		time.Duration(*regInterval)*time.Second,
		time.Duration(*updInterval)*time.Second,
	)
	if err != nil {
		log.Fatalf("Error creating NMD. Err: %v", err)
	}

	if p.GetPipelineType() == globals.NaplesPipelineIris {
		p.MountDelphiObjects()

		p.MountSysmgrObjects()

		p.RunDelphiClient(state.Agent(*nm))
	}

	log.Infof("%s is running {%+v}", globals.Nmd, nm)

	// wait forever
	select {}
}
