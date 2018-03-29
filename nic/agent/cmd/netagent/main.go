// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
// Pensando NetworkAgent binary. This is expected to run on NAPLES

package main

import (
	"context"
	"flag"
	"strings"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/nic/agent/netagent"
	hal "github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/state"
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
		hostIf       = flag.String("hostif", "ntrunk0", "Host facing interface")
		agentDbPath  = flag.String("agentdb", "/tmp/naples-netagent.db", "Agent Database file")
		npmURL       = flag.String("npm", "master.local:"+globals.NpmRPCPort, "NPM RPC server URL")
		debugflag    = flag.Bool("debug", false, "Enable debug mode")
		logToFile    = flag.String("logtofile", "/var/log/pensando/naples-netagent.log", "Redirect logs to file")
		resolverURLs = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
		// ToDo Remove this flag prior to FCS the datapath should be defaulted to HAL
		datapath = flag.String("datapath", "mock", "specify the agent datapath type either mock or hal")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "NaplesNetAgent",
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

	// Set the TLS provider for rpckit
	rpckit.SetN4STLSProvider()

	macAddr, err := netutils.GetIntfMac(*hostIf)
	if err != nil {
		log.Fatalf("Error getting host interface's mac addr. Err: %v", err)
	}

	// create a resolver
	resolverClient := resolver.New(&resolver.Config{Name: "naples-netagent", Servers: strings.Split(*resolverURLs, ",")})
	//
	opt := tsdb.Options{
		ClientName:     "naples-netagent-" + macAddr.String(),
		ResolverClient: resolverClient,
	}

	err = tsdb.Init(tsdb.NewBatchTransmitter(context.TODO()), opt)
	if err != nil {
		log.Infof("Error initializing the tsdb transmitter. Err: %v", err)
	}

	var dp state.NetDatapathAPI
	// ToDo Remove mock hal datapath prior to FCS
	if *datapath == "hal" {
		dp, err = hal.NewHalDatapath("hal")
		if err != nil {
			log.Fatalf("Error creating hal datapath. Err: %v", err)
		}
	} else {
		// Set expectations to allow mock testing
		mockDp, err := hal.NewHalDatapath("mock")
		mockDp.Hal.MockClients.MockNetclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
		mockDp.Hal.MockClients.MockNetclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
		mockDp.Hal.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
		mockDp.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
		mockDp.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
		mockDp.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
		mockDp.Hal.MockClients.MockIfclient.EXPECT().InterfaceCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
		mockDp.Hal.MockClients.MockEpclient.EXPECT().EndpointCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
		if err != nil {
			log.Fatalf("Error creating mock datapath. Err: %v", err)
		}
		dp = mockDp
	}

	// create the new NetAgent
	ag, err := netagent.NewAgent(dp, *agentDbPath, macAddr.String(), *npmURL, ":"+globals.AgentRESTPort, resolverClient)
	if err != nil {
		log.Fatalf("Error creating Naples NetAgent. Err: %v", err)
	}
	log.Printf("Naples NetAgent {%+v} is running", ag)

	// wait forever
	<-waitCh
}
