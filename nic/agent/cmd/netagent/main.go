// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
// Pensando NetworkAgent binary. This is expected to run on NAPLES

package main

import (
	"context"
	"flag"
	"strings"

	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	hal "github.com/pensando/sw/nic/agent/netagent/datapath"
	protos "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/troubleshooting"
	tshal "github.com/pensando/sw/nic/agent/troubleshooting/datapath/hal"
	tstypes "github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// Main function
func main() {
	// command line flags
	var (
		hostIf          = flag.String("hostif", "ntrunk0", "Host facing interface")
		agentDbPath     = flag.String("agentdb", "/tmp/naples-netagent.db", "Agent Database file")
		npmURL          = flag.String("npm", "master.local:"+globals.NpmRPCPort, "NPM RPC server URL")
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", "/var/log/pensando/naples-netagent.log", "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
		restURL         = flag.String("rest-url", ":"+globals.AgentRESTPort, "specify Agent REST URL")
		// ToDo Remove this flag prior to FCS the datapath should be defaulted to HAL
		datapath = flag.String("datapath", "mock", "specify the agent datapath type either mock or hal")
		mode     = flag.String("mode", "classic", "specify the agent mode either classic or managed")
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

	macAddr, err := netutils.GetIntfMac(*hostIf)
	if err != nil {
		log.Fatalf("Error getting host interface's mac addr. Err: %v", err)
	}

	var dp types.NetDatapathAPI
	// ToDo Remove mock hal datapath prior to FCS
	if *datapath == "hal" {
		dp, err = hal.NewHalDatapath("hal")
		if err != nil {
			log.Fatalf("Error creating hal datapath. Err: %v", err)
		}
	} else {
		// Set expectations to allow mock testing
		dp, err = hal.NewHalDatapath("mock")
		if err != nil {
			log.Fatalf("Error creating mock datapath. Err: %v", err)
		}
	}
	var agMode protos.AgentMode
	var resolverClient resolver.Interface
	if *mode == "managed" {
		agMode = protos.AgentMode_MANAGED

		// create a resolver
		resolverClient = resolver.New(&resolver.Config{Name: "naples-netagent", Servers: strings.Split(*resolverURLs, ",")})
		//
		opt := tsdb.Options{
			ClientName:     "naples-netagent-" + macAddr.String(),
			ResolverClient: resolverClient,
		}

		err = tsdb.Init(tsdb.NewBatchTransmitter(context.TODO()), opt)
		if err != nil {
			log.Infof("Error initializing the tsdb transmitter. Err: %v", err)
		}
	} else {
		agMode = protos.AgentMode_CLASSIC
	}

	// create the new NetAgent
	ag, err := netagent.NewAgent(dp, *agentDbPath, macAddr.String(), *npmURL, resolverClient, agMode)
	if err != nil {
		log.Fatalf("Error creating Naples NetAgent. Err: %v", err)
	}
	log.Printf("NetAgent {%+v} instantiated", ag)

	// create the new Troublehshooting agent
	var tsdp tstypes.TsDatapathAPI
	// ToDo Remove mock hal datapath prior to FCS
	if *datapath == "hal" {
		tsdp, err = tshal.NewHalDatapath("hal")
		if err != nil {
			log.Fatalf("Error creating hal datapath. Err: %v", err)
		}
	} else {
		// Set expectations to allow mock testing
		mockDp, err := tshal.NewHalDatapath("mock")
		if err != nil {
			log.Fatalf("Error creating hal datapath. Err: %v", err)
		}
		tsdp = mockDp
	}
	tsa, err := troubleshooting.NewTsAgent(tsdp, "/tmp/naples-TsAgent.db", macAddr.String(), *npmURL, resolverClient, agMode)
	if err != nil {
		log.Fatalf("Error creating Naples NetAgent. Err: %v", err)
	}
	log.Printf("TroubleShooting Agent {%+v} instantiated", tsa)

	// create REST api server
	restServer, err := restapi.NewRestServer(ag.NetworkAgent, tsa.TroubleShootingAgent, *restURL)
	if err != nil {
		log.Errorf("Error creating the rest API server. Err: %v", err)
	}
	ag.RestServer = restServer
	log.Infof("%s is running {%+v}", globals.Netagent, ag)

	// wait forever
	<-waitCh
}
