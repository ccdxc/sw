// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
	"flag"
	"strings"

	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/plugins/k8s/cni"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// Main function
func main() {
	var (
		uplinkIf        = flag.String("uplink", "eth2", "Uplink interface")
		agentDbPath     = flag.String("agentdb", "/tmp/n4sagent.db", "Agent Database file")
		npmURL          = flag.String("npm", "master.local:"+globals.NpmRPCPort, "NPM RPC server URL")
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		logToFile       = flag.String("logtofile", "/var/log/pensando/k8sagent.log", "Redirect logs to file")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Netagent,
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

	// create a dummy channel to wait forver
	waitCh := make(chan bool)

	// Set the TLS provider for rpckit
	rpckit.SetN4STLSProvider()

	// read the mac address of the uplink interface
	macAddr, err := netutils.GetIntfMac(*uplinkIf)
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

	// create a network datapath
	dp, err := datapath.NewHalDatapath("mock")
	if err != nil {
		log.Fatalf("Error creating fake datapath. Err: %v", err)
	}

	// create the new NetAgent
	ag, err := netagent.NewAgent(dp, *agentDbPath, macAddr.String(), *npmURL, resolverClient, state.AgentMode_MANAGED)
	if err != nil {
		log.Fatalf("Error creating network agent. Err: %v", err)
	}
	restServer, err := restapi.NewRestServer(ag.NetworkAgent, nil, ":"+globals.AgentRESTPort)
	ag.RestServer = restServer
	log.Printf("%s {%+v} is running", globals.Netagent, ag)

	// create a CNI server
	cniServer, err := cni.NewCniServer(cni.CniServerListenURL, ag.NetworkAgent)
	if err != nil {
		log.Fatalf("Error creating CNI server. Err: %v", err)
	}
	log.Infof("CNI server is running {%+v}", cniServer)

	// wait forever
	<-waitCh
}
