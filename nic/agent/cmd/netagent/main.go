// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
// Pensando NetworkAgent binary. This is expected to run on NAPLES

package main

import (
	"flag"
	"fmt"
	"path/filepath"
	"time"

	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/revproxy"
	"github.com/pensando/sw/nic/agent/tpa"
	"github.com/pensando/sw/nic/agent/troubleshooting"
	tshal "github.com/pensando/sw/nic/agent/troubleshooting/datapath/hal"
	tstypes "github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// Main function
func main() {
	// command line flags
	var (
		hostIf          = flag.String("hostif", "lo", "Host facing interface")
		agentDbPath     = flag.String("agentdb", "/tmp/naples-netagent.db", "Agent Database file")
		npmURL          = flag.String("npm", globals.Npm, "NPM RPC server URL")
		disableTSA      = flag.Bool("disabletsa", false, " Disable Telemetry and Troubleshooting agents")
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Netagent)), "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		restURL         = flag.String("rest-url", "localhost:"+globals.AgentRESTPort, "specify Agent REST URL")
		revProxyURL     = flag.String("rev-proxy-url", ":"+globals.AgentProxyPort, "specify Reverse Proxy Router REST URL")
		// ToDo Remove this flag prior to FCS the datapath should be defaulted to HAL
		datapath = flag.String("datapath", "mock", "specify the agent datapath type either mock or hal")
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
			MaxSize:    5,
			MaxBackups: 1,
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

	// create the new NetAgent
	ag, err := netagent.NewAgent(*datapath, *agentDbPath, *npmURL, nil)
	if err != nil {
		log.Fatalf("Error creating Naples NetAgent. Err: %v", err)
	}
	log.Printf("NetAgent {%+v} instantiated", ag)

	// Set start time
	ag.NetworkAgent.NetAgentStartTime = time.Now()

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

	// TODO remove the command line switch
	if *disableTSA {
		restServer, err := restapi.NewRestServer(ag.NetworkAgent, nil, nil, *restURL)
		if err != nil {
			log.Errorf("Error creating the rest API server. Err: %v", err)
		}
		ag.RestServer = restServer
	} else {

		tsa, err := troubleshooting.NewTsAgent(tsdp, ag.NetworkAgent.NodeUUID, ag.NetworkAgent, ag.GetMgmtIPAddr)
		if err != nil {
			log.Fatalf("Error creating Naples NetAgent. Err: %v", err)
		}

		ag.TroubleShoot = tsa
		log.Printf("TroubleShooting Agent {%+v} instantiated", tsa)

		// telemetry policy agent
		tpa, err := tpa.NewPolicyAgent(*datapath, ag.NetworkAgent, ag.GetMgmtIPAddr)
		if err != nil {
			log.Fatalf("Error creating telemetry policy agent, Err: %v", err)
		}
		log.Printf("telemetry policy agent {%+v} instantiated", tpa)
		ag.Tmagent = tpa

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

	// start reverse proxy after local REST server is initialized
	proxyConfig := map[string]string{
		// NMD
		"/api/v1/naples": "http://127.0.0.1:" + globals.NmdRESTPort,
		"/monitoring/":   "http://127.0.0.1:" + globals.NmdRESTPort,
		"/cores/":        "http://127.0.0.1:" + globals.NmdRESTPort,
		"/cmd/":          "http://127.0.0.1:" + globals.NmdRESTPort,
		"/update/":       "http://127.0.0.1:" + globals.NmdRESTPort,

		// TM-AGENT
		"/telemetry/":           "http://127.0.0.1:" + globals.TmAGENTRestPort,
		"/api/telemetry/fwlog/": "http://127.0.0.1:" + globals.TmAGENTRestPort,

		// EVENTS
		"/api/eventpolicies/": "http://127.0.0.1:" + globals.EvtsProxyRESTPort,

		// NET-AGENT
		"/api/telemetry/flowexports/": "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/networks/":              "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/endpoints/":             "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/sgs/":                   "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/tenants/":               "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/interfaces/":            "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/namespaces/":            "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/nat/pools/":             "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/nat/policies/":          "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/routes/":                "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/nat/bindings/":          "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/ipsec/policies/":        "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/ipsec/encryption/":      "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/ipsec/decryption/":      "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/security/policies/":     "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/security/profiles/":     "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/tunnels/":               "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/tcp/proxies/":           "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/system/ports":           "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/apps":                   "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/vrfs":                   "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/mirror/sessions/":       "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/system/info":            "http://127.0.0.1:" + globals.AgentRESTPort,
		"/api/system/debug":           "http://127.0.0.1:" + globals.AgentRESTPort,

		"/api ": "http://127.0.0.1:" + globals.AgentRESTPort,

		// Techsupport
		"/api/techsupport/": "http://127.0.0.1:" + globals.NaplesTechSupportRestPort,
		"/api/diagnostics/": "http://127.0.0.1:" + globals.NaplesTechSupportRestPort,
	}

	proxyRouter, err := revproxy.NewRevProxyRouter(*revProxyURL, proxyConfig)
	if err != nil {
		log.Fatalf("Could not start Reverse Proxy Router. Err: %v", err)
	}

	defer proxyRouter.Stop()

	log.Infof("%s is running {%+v}.  With UUID: %v", globals.Netagent, ag, ag.NetworkAgent.NodeUUID)

	// wait forever
	<-waitCh
}
