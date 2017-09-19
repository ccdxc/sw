// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"

	"github.com/pensando/sw/nic/agent"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// Main function
func main() {
	// command line flags
	var (
		hostIf       = flag.String("hostif", "ntrunk0", "Host facing interface")
		uplinkIf     = flag.String("uplink", "eth2", "Uplink interface")
		dbPath       = flag.String("db", "/tmp/n4sagent.db", "Database file")
		npmURL       = flag.String("npm", "master.local:"+globals.NpmRPCPort, "NPM RPC server URL")
		debugflag    = flag.Bool("debug", false, "Enable debug mode")
		logToFile    = flag.String("logtofile", "/var/log/pensando/n4sagent.log", "Redirect logs to file")
		resolverURLs = flag.String("resolver-urls", ":"+globals.CMDGRPCPort, "comma separated list of resolver URLs <IP:Port>")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "N4sAgent",
		Format:      log.LogFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10, // TODO: These needs to be part of Service Config Object
			MaxBackups: 3,  // TODO: These needs to be part of Service Config Object
			MaxAge:     7,  // TODO: These needs to be part of Service Config Object
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	// create a dummy channel to wait forver
	waitCh := make(chan bool)

	// read the mac address of the host interface
	macAddr, err := netutils.GetIntfMac(*hostIf)
	if err != nil {
		log.Fatalf("Error getting host interface's mac addr. Err: %v", err)
	}

	// create a network datapath
	dp, err := datapath.NewNaplesDatapath(*hostIf, *uplinkIf)
	if err != nil {
		log.Fatalf("Error creating fake datapath. Err: %v", err)
	}

	// create the new agent
	ag, err := agent.NewAgent(dp, *dbPath, macAddr.String(), *npmURL, *resolverURLs)
	if err != nil {
		log.Fatalf("Error creating network agent. Err: %v", err)
	}

	log.Printf("Agent {%+v} is running", ag)

	// wait forever
	<-waitCh
}
