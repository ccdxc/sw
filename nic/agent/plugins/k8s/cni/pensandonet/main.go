// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

//+build !test

package main

import (
	"github.com/appc/cni/pkg/version"
	cni "github.com/containernetworking/cni/pkg/skel"
	cniServer "github.com/pensando/sw/nic/agent/plugins/k8s/cni"
	"github.com/pensando/sw/venice/utils/log"
)

func main() {
	// Fill logger config params
	logConfig := &log.Config{
		Module:      "K8sAgent",
		Format:      log.JSONFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   "/tmp/pensandonet-plugin.log",
			MaxSize:    10, // TODO: These needs to be part of Service Config Object
			MaxBackups: 3,  // TODO: These needs to be part of Service Config Object
			MaxAge:     7,  // TODO: These needs to be part of Service Config Object
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	versionInfo := version.PluginSupports("0.1.0")

	n := NewCNIPlugin(cniServer.CniServerListenURL)
	cni.PluginMain(n.CmdAdd, n.CmdDel, versionInfo)
}
