// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
	"flag"
	"strings"

	"github.com/pensando/sw/nic/agent/nmd"
	"github.com/pensando/sw/nic/agent/nmd/platform"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// Main function for Naples Management Daemon (NMD)
func main() {
	// command line flags
	var (
		hostIf             = flag.String("hostif", "ntrunk0", "Host facing interface")
		nmdDbPath          = flag.String("nmddb", "/tmp/nmd.db", "NMD Database file")
		cmdRegistrationURL = flag.String("cmdregistration", ":"+globals.CMDSmartNICRegistrationAPIPort, "NIC Registration API server URL(s)")
		cmdUpdatesURL      = flag.String("cmdupdates", ":"+globals.CMDSmartNICUpdatesPort, "NIC Updates server URL(s)")
		res                = flag.String("resolver", ":"+globals.CMDResolverPort, "Resolver URL")
		mode               = flag.String("mode", "classic", "Naples mode, \"classic\" or \"managed\" ")
		debugflag          = flag.Bool("debug", false, "Enable debug mode")
		logToFile          = flag.String("logtofile", "/tmp/nmd.log", "Redirect logs to file")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "NMD",
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

	// read the mac address of the host interface
	macAddr, err := netutils.GetIntfMac(*hostIf)
	if err != nil {
		log.Fatalf("Error getting host interface's mac addr. Err: %v", err)
	}

	// init resolver client
	resolverClient := resolver.New(&resolver.Config{Name: "NMD", Servers: strings.Split(*res, ",")})

	/// init tsdb
	opt := tsdb.Options{
		ClientName:     "netagent_" + macAddr.String(),
		ResolverClient: resolverClient,
	}
	err = tsdb.Init(tsdb.NewBatchTransmitter(context.TODO()), opt)
	if err != nil {
		log.Infof("Error initializing the tsdb transmitter. Err: %v", err)
	}

	// create a platform agent
	pa, err := platform.NewNaplesPlatformAgent()
	if err != nil {
		log.Fatalf("Error creating platform agent. Err: %v", err)
	}

	// create the new NMD
	nm, err := nmd.NewAgent(pa, *nmdDbPath, macAddr.String(), *cmdRegistrationURL, *cmdUpdatesURL, ":"+globals.NmdRESTPort, *mode, resolverClient)
	if err != nil {
		log.Fatalf("Error creating NMD. Err: %v", err)
	}

	log.Printf("NMD {%+v} is running", nm)

	// wait forever
	<-waitCh
}
