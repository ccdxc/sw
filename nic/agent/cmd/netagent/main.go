// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// Pensando NetworkAgent is responsible for handling all management plane functions on NAPLES

package main

import (
	"context"
	"runtime/debug"
	"time"

	"github.com/pensando/sw/venice/utils/tsdb"

	"github.com/pensando/sw/nic/agent/dscagent"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/venice/utils/log"
)

// periodicFreeMemory forces garbage collection every minute and frees OS memory
func periodicFreeMemory() {
	for {
		select {
		case <-time.After(types.DefaultGCDuration):
			// force GC and free OS memory
			debug.FreeOSMemory()
		}
	}
}

// Main function
func main() {
	logConfig := &log.Config{
		Module:      types.Netagent,
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   types.NetagentLogFile,
			MaxSize:    5,
			MaxBackups: 1,
			MaxAge:     7,
		},
	}
	logger := log.SetConfig(logConfig)
	tsdb.Init(context.Background(), &tsdb.Opts{
		ClientName:              types.Netagent,
		Collector:               types.Collector,
		DBName:                  "default",
		SendInterval:            time.Minute,
		ConnectionRetryInterval: types.StatsRetryInterval,
	})
	defer tsdb.Cleanup()

	ag, err := dscagent.NewDSCAgent(logger, types.Npm, types.Tpm, types.Tsm, types.DefaultAgentRestURL)
	if err != nil {
		log.Fatalf("Agent failed to start. Err: %v", err)
	}

	debug.SetGCPercent(20)
	go periodicFreeMemory()

	defer ag.Stop()
	log.Infof("Agent up and running: %v", ag)
	select {}
}
