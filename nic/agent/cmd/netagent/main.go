// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
// Pensando NetworkAgent binary. This is expected to run on NAPLES

package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"
	"strings"

	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/revproxy"
	protos "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/tpa"
	"github.com/pensando/sw/nic/agent/troubleshooting"
	tshal "github.com/pensando/sw/nic/agent/troubleshooting/datapath/hal"
	tstypes "github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	_ "github.com/pensando/sw/nic/delphi/sdk/proto"
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
		hostIf          = flag.String("hostif", "lo", "Host facing interface")
		agentDbPath     = flag.String("agentdb", "/tmp/naples-netagent.db", "Agent Database file")
		npmURL          = flag.String("npm", globals.Npm, "NPM RPC server URL")
		tpmURL          = flag.String("tpm", "master.local:"+globals.TpmRPCPort, "TPM RPC server URL")
		disableTSA      = flag.Bool("disabletsa", false, " Disable Telemetry and Troubleshooting agents")
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Netagent)), "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
		restURL         = flag.String("rest-url", ":"+globals.AgentRESTPort, "specify Agent REST URL")
		revProxyURL     = flag.String("rev-proxy-url", ":"+globals.RevProxyPort, "specify Reverse Proxy Router REST URL")
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
	var err error
	var hostIfMAC string

	// Initialize logger config
	log.SetConfig(logConfig)

	// create a dummy channel to wait forver
	waitCh := make(chan bool)

	// ToDo Remove this prior to fcs.
	if len(*hostIf) > 0 {
		macAddr, err := netutils.GetIntfMac(*hostIf)
		if err != nil {
			log.Fatalf("Error getting host interface's mac addr. Err: %v", err)
		}

		hostIfMAC = macAddr.String()
	}

	var agMode protos.AgentMode
	var resolverClient resolver.Interface

	if *mode == "managed" {
		agMode = protos.AgentMode_MANAGED
		// TODO Remove this once e2e is migrated to IOTA
		resolverClient = resolver.New(&resolver.Config{Name: globals.Netagent, Servers: strings.Split(*resolverURLs, ",")})
		opt := tsdb.Options{
			ClientName:     "naples-netagent",
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
	ag, err := netagent.NewAgent(*datapath, *agentDbPath, *npmURL, resolverClient, agMode)
	if err != nil {
		log.Fatalf("Error creating Naples NetAgent. Err: %v", err)
	}
	log.Printf("NetAgent {%+v} instantiated", ag)

	// TODO Remove manual setting up of npm client based on cmdline flag once venice e2e tests are moved to IOTA
	if agMode == protos.AgentMode_MANAGED {
		// create the NPM client
		npmClient, err := ctrlerif.NewNpmClient(ag.NetworkAgent, *npmURL, ag.ResolverClient)
		if err != nil {
			log.Errorf("Error creating NPM client. Err: %v", err)
		}
		ag.NpmClient = npmClient
	}
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

	// create the reverse proxy router
	if proxyRouter, ok := revproxy.NewRevProxyRouter(*revProxyURL); ok == nil {
		log.Printf("Reverse Proxy Router instantiated")
		revproxy.AddRevProxyDest("api", globals.NmdRESTPort)
		revproxy.AddRevProxyDest("monitoring", globals.NmdRESTPort)
		revproxy.AddRevProxyDest("telemetry", globals.TmAGENTRestPort)
		revproxy.AddRevProxyDest("cores", globals.NmdRESTPort)
		revproxy.AddRevProxyDest("cmd", globals.NmdRESTPort)
		revproxy.AddRevProxyDest("update", globals.NmdRESTPort)
		defer proxyRouter.Stop()
	} else {
		log.Fatalf("Could not start Reverse Proxy Router. Err: %v", err)
	}

	// TODO remove the command line switch
	if *disableTSA {
		restServer, err := restapi.NewRestServer(ag.NetworkAgent, nil, nil, *restURL)
		if err != nil {
			log.Errorf("Error creating the rest API server. Err: %v", err)
		}
		ag.RestServer = restServer
	} else {

		tsa, err := troubleshooting.NewTsAgent(tsdp, ag.NetworkAgent.NodeUUID, *tpmURL, resolverClient, agMode, ag.NetworkAgent)
		if err != nil {
			log.Fatalf("Error creating Naples NetAgent. Err: %v", err)
		}
		log.Printf("TroubleShooting Agent {%+v} instantiated", tsa)

		// telemetry policy agent
		tpa, err := tpa.NewPolicyAgent(ag.NetworkAgent.NodeUUID, *npmURL, resolverClient, agMode, *datapath, ag.NetworkAgent, "")
		if err != nil {
			log.Fatalf("Error creating telemetry policy agent, Err: %v", err)
		}
		log.Printf("telemetry policy agent {%+v} instantiated", tpa)

		// create REST api server
		restServer, err := restapi.NewRestServer(ag.NetworkAgent, tsa.TroubleShootingAgent, tpa.TpState, *restURL)
		if err != nil {
			log.Errorf("Error creating the rest API server. Err: %v", err)
		}
		ag.RestServer = restServer
	}

	// TODO remove prior to fcs or once fru objects are writen and read from delphi
	if len(hostIfMAC) > 0 {
		ag.NetworkAgent.NodeUUID = hostIfMAC
	}

	log.Infof("%s is running {%+v}.  With UUID: %v", globals.Netagent, ag, ag.NetworkAgent.NodeUUID)

	// wait forever
	<-waitCh
}
