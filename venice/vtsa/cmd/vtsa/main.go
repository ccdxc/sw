package main

import (
	"flag"
	"fmt"
	"path/filepath"
	"strings"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/k8s"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	tsa "github.com/pensando/sw/venice/utils/techsupport"
)

func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.VeniceTechSupport)), "Redirect logs to file")
		configFile      = flag.String("config", "", "Config file for Tech Support Agent")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		resolverURL     = flag.String("resolver-urls", "", "resovler url")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.VeniceTechSupport,
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
	logger := log.SetConfig(logConfig)
	defer logger.Close()
	log.Info("Log configuration set.")
	controllers := strings.Split(*resolverURL, ",")

	var hostname string
	hostname = k8s.GetNodeName()
	if len(hostname) == 0 {
		hostname = "venice"
	}

	// Plugging in default values when the agent is created.
	tsmClient := tsa.NewTSMClient(hostname, "NA", string(cluster.KindNode), *configFile, controllers, "", "")

	go tsmClient.Start()

	log.Info("Started Venice TSA")

	// wait forever
	select {}
}
