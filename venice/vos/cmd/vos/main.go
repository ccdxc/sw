// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"os"
	"path/filepath"

	"fmt"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/vos"
)

var pkgName = globals.Vos

func main() {

	var (
		nsURLs = flag.String("resolver-urls", ":"+globals.CMDResolverPort,
			"comma separated list of resolver URLs of the form 'ip:port'")
		logFile         = flag.String("logfile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Vos)), "redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		debugFlag       = flag.Bool("debug", false, "enable debug mode")
	)

	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      pkgName,
		Format:      log.JSONFmt,
		Filter:      log.AllowAllFilter,
		Debug:       *debugFlag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	log.Infof("resolver-urls %+v", nsURLs)
	log.Infof("starting object store with args : {%+v}", os.Args)

	args := []string{pkgName, "server", "--address", fmt.Sprintf(":%s", globals.VosPort), "/disk1"}
	// init obj store
	err := vos.New(args)
	if err != nil {
		// let the scheduler restart obj store
		log.Fatalf("failed to init object store, %s", err)
	}

	select {}
}
