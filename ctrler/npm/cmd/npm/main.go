// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"log"

	"github.com/pensando/sw/ctrler/npm"
	"github.com/pensando/sw/ctrler/npm/rpcserver"
)

// main function of network controller
func main() {
	// create a dummy channel to wait forver
	waitCh := make(chan bool)

	// create the controller
	ctrler, err := npm.NewNetctrler(rpcserver.NetctrlerURL)
	if err != nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}

	// FIXME: dummy code to emulate network create event from API server
	ctrler.Watchr.CreateNetwork("default", "default", "10.1.1.0/24", "10.1.1.254")

	// wait forever
	<-waitCh
}
