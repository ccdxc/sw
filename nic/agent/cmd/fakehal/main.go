package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"

	hal "github.com/pensando/sw/nic/agent/cmd/fakehal/hal"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

func main() {
	// command line flags
	var (
		halPort         = flag.String("port", types.HalGRPCDefaultPort, "HAL Port")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		logToFile       = flag.String("log-to-file", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.FakeHal)), "Path of the log file")
	)
	flag.Parse()

	// Logger config params
	logConfig := &log.Config{
		Module:      globals.FakeHal,
		Format:      log.LogFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    2,
			MaxBackups: 1,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	logger := log.SetConfig(logConfig)
	defer logger.Close()

	if envHalPort := os.Getenv("HAL_GRPC_PORT"); envHalPort != "" && envHalPort != types.HalGRPCDefaultPort {
		*halPort = envHalPort
	}

	halURL := fmt.Sprintf("127.0.0.1:%s", *halPort)
	hal.NewFakeHalServer(halURL)
	select {}
}
