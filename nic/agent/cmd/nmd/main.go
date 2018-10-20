// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
	"flag"
	"fmt"
	"net"
	"path/filepath"
	"strings"
	"time"

	cmd "github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/nic/agent/nmd"
	"github.com/pensando/sw/nic/agent/nmd/platform"
	"github.com/pensando/sw/nic/agent/nmd/upg"
	"github.com/pensando/sw/nic/delphi/gosdk"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// Main function for Naples Management Daemon (NMD)
func main() {
	// command line flags
	var (
		hostIf             = flag.String("hostif", "ntrunk0", "Host facing interface")
		primaryMAC         = flag.String("primary-mac", "", "Primary MAC address")
		nmdDbPath          = flag.String("nmddb", "/tmp/nmd.db", "NMD Database file")
		cmdRegistrationURL = flag.String("cmdregistration", ":"+globals.CMDSmartNICRegistrationAPIPort, "NIC Registration API server URL(s)")
		cmdUpdatesURL      = flag.String("cmdupdates", ":"+globals.CMDSmartNICUpdatesPort, "NIC Updates server URL(s)")
		cmdCertsURL        = flag.String("cmdcerts", ":"+globals.CMDAuthCertAPIPort, "CMD Certificates API URL(s)")
		regInterval        = flag.Int64("reginterval", globals.NicRegIntvl, "NIC registration interval in seconds")
		updInterval        = flag.Int64("updinterval", globals.NicUpdIntvl, "NIC update interval in seconds")
		res                = flag.String("resolver", ":"+globals.CMDResolverPort, "Resolver URL")
		mode               = flag.String("mode", "host", "Naples mode, \"host\" or \"network\" ")
		hostName           = flag.String("hostname", "", "Hostname of Naples Host")
		debugflag          = flag.Bool("debug", false, "Enable debug mode")
		logToStdoutFlag    = flag.Bool("logtostdout", false, "enable logging to stdout")
		logToFile          = flag.String("log-to-file", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Nmd)), "Path of the log file")
		bypassDelphi       = flag.Bool("bypassdelphi", true, "Bypass Delphi")
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
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	// create events recorder
	if _, err := recorder.NewRecorder(&recorder.Config{
		Source:   &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: globals.Nmd},
		EvtTypes: cmd.GetEventTypes()}); err != nil {
		log.Fatalf("failed to create events recorder, err: %v", err)
	}

	// create a dummy channel to wait forver
	waitCh := make(chan bool)

	// read the mac address of the host interface
	macAddr, err := netutils.GetIntfMac(*hostIf)
	if err != nil {
		log.Fatalf("Error getting host interface's mac addr. Err: %v", err)
	}

	if *primaryMAC != "" {
		var mac net.HardwareAddr
		mac, err = net.ParseMAC(*primaryMAC)
		if err != nil {
			log.Fatalf("Invalid MAC %v", *primaryMAC)
		}
		macAddr = mac
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

	// If hose name is not configured, use the MAC-addr of the host interface
	host := *hostName
	if host == "" {
		host = macAddr.String()
	}

	var delphiClient clientAPI.Client
	if !*bypassDelphi {
		dServ := nmd.NewDelphiService()
		delphiClient, err = gosdk.NewClient(dServ)
		if err != nil {
			log.Fatalf("Error creating delphi client . Err: %v", err)
		}
	}

	// create a upgrade client
	uc, err := upg.NewNaplesUpgradeClient(delphiClient)
	if err != nil {
		log.Fatalf("Error creating Upgrade client . Err: %v", err)
	}

	// create the new NMD
	nm, err := nmd.NewAgent(pa, uc,
		*nmdDbPath,
		host,
		macAddr.String(),
		*cmdRegistrationURL,
		*cmdUpdatesURL,
		":"+globals.NmdRESTPort,
		":"+globals.CMDCertAPIPort,
		*cmdCertsURL,
		*mode,
		time.Duration(*regInterval)*time.Second,
		time.Duration(*updInterval)*time.Second,
		resolverClient,
		delphiClient)
	if err != nil {
		log.Fatalf("Error creating NMD. Err: %v", err)
	}

	log.Infof("%s is running {%+v}", globals.Nmd, nm)

	// wait forever
	<-waitCh
}
