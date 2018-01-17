// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
	"flag"
	"strings"

	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// Main function
func main() {
	// command line flags
	var (
		hostIf       = flag.String("hostif", "ntrunk0", "Host facing interface")
		uplinkIf     = flag.String("uplink", "eth2", "Uplink interface")
		agentDbPath  = flag.String("agentdb", "/tmp/n4sagent.db", "Agent Database file")
		npmURL       = flag.String("npm", "master.local:"+globals.NpmRPCPort, "NPM RPC server URL")
		debugflag    = flag.Bool("debug", false, "Enable debug mode")
		logToFile    = flag.String("logtofile", "/var/log/pensando/n4sagent.log", "Redirect logs to file")
		resolverURLs = flag.String("resolver-urls", ":"+globals.CMDGRPCPort, "comma separated list of resolver URLs <IP:Port>")
		datapathType = flag.String("datapath-type", "fswitch", "specify the agent datapath type either fswitch or hal")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "N4sAgent",
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   true,
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
	resolverClient := resolver.New(&resolver.Config{Name: "netagent", Servers: strings.Split(*resolverURLs, ",")})

	opt := tsdb.Options{
		ClientName:     "netagent_" + macAddr.String(),
		ResolverClient: resolverClient,
	}
	err = tsdb.Init(tsdb.NewBatchTransmitter(context.TODO()), opt)
	if err != nil {
		log.Infof("Error initializing the tsdb transmitter. Err: %v", err)
	}
	var dp state.NetDatapathAPI

	// create a network datapath
	if *datapathType == "hal" {
		dp, err = datapath.NewHalDatapath("hal")
		if err != nil {
			log.Fatalf("Error creating hal datapath. Err: %v", err)
		}
	} else {
		dp, err = datapath.NewNaplesDatapath(*hostIf, *uplinkIf)
		if err != nil {
			log.Fatalf("Error creating fake datapath. Err: %v", err)
		}
	}

	// create the new NetAgent
	ag, err := netagent.NewAgent(dp, *agentDbPath, macAddr.String(), *npmURL, ":"+globals.AgentRESTPort, resolverClient)
	if err != nil {
		log.Fatalf("Error creating NetAgent. Err: %v", err)
	}
	log.Printf("NetAgent {%+v} is running", ag)

	// wait forever
	<-waitCh
}
