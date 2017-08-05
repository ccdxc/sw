// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

//+build !test

package main

import (
	"os"

	log "github.com/Sirupsen/logrus"
	"github.com/appc/cni/pkg/version"
	cni "github.com/containernetworking/cni/pkg/skel"
	cniServer "github.com/pensando/sw/agent/plugins/k8s/cni"
)

func main() {
	// Open a logfile
	f, err := os.OpenFile("/tmp/pensandonet-plugin.log", os.O_RDWR|os.O_CREATE|os.O_APPEND, 0666)
	if err != nil {
		log.Fatalf("error opening file: %v", err)
	}
	defer f.Close()

	// set the log file as the default log output
	log.SetOutput(f)

	versionInfo := version.PluginSupports("0.1.0")

	n := NewCNIPlugin(cniServer.CniServerListenURL)
	cni.PluginMain(n.CmdAdd, n.CmdDel, versionInfo)
}
