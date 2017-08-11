// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"

	"github.com/pensando/sw/agent"
	"github.com/pensando/sw/agent/netagent/datapath"
	"github.com/pensando/sw/agent/netagent/netutils"
	"github.com/pensando/sw/agent/plugins/k8s/cni"
	"github.com/pensando/sw/ctrler/npm/rpcserver"
	"github.com/pensando/sw/utils/log"
)

// Main function
func main() {
	var (
		uplinkIf  = flag.String("uplink", "eth2", "Uplink interface")
		debugflag = flag.Bool("debug", false, "Enable debug mode")
		logToFile = flag.String("logtofile", "/var/log/pensando/k8sagent.log", "Redirect logs to file")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "K8sAgent",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
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

	// read the mac address of the uplink interface
	macAddr, err := netutils.GetIntfMac(*uplinkIf)
	if err != nil {
		log.Fatalf("Error getting host interface's mac addr. Err: %v", err)
	}

	// create a network datapath
	dp, err := datapath.NewFakeDatapath(*uplinkIf)
	if err != nil {
		log.Fatalf("Error creating fake datapath. Err: %v", err)
	}

	// create the new agent
	ag, err := agent.NewAgent(dp, macAddr.String(), rpcserver.NetctrlerURL)
	if err != nil {
		log.Fatalf("Error creating network agent. Err: %v", err)
	}

	// create a CNI server
	cniServer, err := cni.NewCniServer(cni.CniServerListenURL, ag.Netagent)
	if err != nil {
		log.Fatalf("Error creating CNI server. Err: %v", err)
	}

	log.Printf("CNI server {%+v} is running", cniServer)

	// wait forever
	<-waitCh
}
