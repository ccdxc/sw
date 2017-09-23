// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"io/ioutil"

	"github.com/pensando/sw/venice/ctrler/ckm"
	"github.com/pensando/sw/venice/ctrler/ckm/rpcserver"
	"github.com/pensando/sw/venice/utils/log"
)

// main function of the Cluster Key Manager
func main() {

	var (
		debugflag = flag.Bool("debug", false, "Enable debug mode")
		logToFile = flag.String("logtofile", "/var/log/pensando/ckm.log", "Redirect logs to file")
	)

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "CKM",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
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

	// create a dummy channel to wait forever
	waitCh := make(chan bool)

	// TODO: This needs to be part of Service Config Object
	keyStoreDir, err := ioutil.TempDir("", "ckm")
	if err != nil {
		log.Fatal(err)
	}

	// create the controller
	_, err = ckm.NewCKMctrler(rpcserver.CKMctrlerURL, keyStoreDir)
	if err != nil {
		log.Fatalf("Error creating controller instance: %v", err)
	}

	// wait forever
	<-waitCh
}
