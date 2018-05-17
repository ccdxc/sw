// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

//+build !test

package main

import (
	"flag"

	"github.com/appc/cni/pkg/version"
	cni "github.com/containernetworking/cni/pkg/skel"

	cniServer "github.com/pensando/sw/nic/agent/plugins/k8s/cni"
	"github.com/pensando/sw/venice/utils/log"
)

func main() {
	var (
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "K8sAgent",
		Format:      log.JSONFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   "/tmp/pensandonet-plugin.log",
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	versionInfo := version.PluginSupports("0.1.0")

	n := NewCNIPlugin(cniServer.CniServerListenURL)
	cni.PluginMain(n.CmdAdd, n.CmdDel, versionInfo)
}
