// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"log"

	"github.com/pensando/sw/agent"
	"github.com/pensando/sw/agent/netagent/datapath"
	"github.com/pensando/sw/agent/plugins/k8s/cni"
	"github.com/pensando/sw/ctrler/npm/rpcserver"
)

// Main function
func main() {
	// create a dummy channel to wait forver
	waitCh := make(chan bool)

	// create a network datapath
	dp, err := datapath.NewFakeDatapath()
	if err != nil {
		log.Fatalf("Error creating fake datapath. Err: %v", err)
	}

	// create the new agent
	ag, err := agent.NewAgent(dp, rpcserver.NetctrlerURL)
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
