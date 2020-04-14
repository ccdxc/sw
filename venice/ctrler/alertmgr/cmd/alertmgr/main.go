// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"path/filepath"
	"strings"

	"github.com/pensando/sw/venice/ctrler/alertmgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	debugflag       = flag.Bool("debug", false, "Enable debug mode")
	logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.AlertMgr)), "Redirect logs to file")
	logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
)

// main (command source) for alert manager
func main() {
	// Logger config.
	logConfig := &log.Config{
		Module:      globals.AlertMgr,
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		CtxSelector: log.ContextAll,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	logger := log.SetConfig(logConfig)
	defer logger.Close()

	// Create resolver client.
	rslvr := resolver.New(&resolver.Config{
		Name:    "alertmgr",
		Servers: strings.Split(*flag.String("resolver-addrs", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>"), ",")})

	// Create the controller.
	// TODO - uncomment after tests
	// amgr, err := alertmgr.New(logger, rslvr)
	_, err := alertmgr.New(logger, rslvr)
	if err != nil {
		log.Fatalf("Failed to create alertmgr, err %v", err)
	}

	// TODO - uncomment after tests
	// amgr.Run()
	// defer amgr.Stop()
}
