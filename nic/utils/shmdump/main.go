package main

import (
	"flag"
	"fmt"
	"time"

	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/nic/agent/nevtsproxy/reader"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	event = "event"
	fwLog = "fwlog"
)

// main (command source) for shmdump
func main() {
	var (
		filepath    = flag.String("file", "", "Shared memory file")
		messageType = flag.String("type", "event", "Type of message to be read from the shared memory e.g. fwlog, event")
	)

	flag.Parse()

	// Fill logger config params
	config := &log.Config{
		Module:      "shmdump",
		Format:      log.JSONFmt,
		Filter:      log.AllowAllFilter,
		CtxSelector: log.ContextAll,
		LogToFile:   true,
		Debug:       true,
		FileCfg: log.FileConfig{
			Filename:   "/tmp/shmdump.log",
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}
	logger := log.SetConfig(config)
	defer logger.Close()

	if utils.IsEmpty(*filepath) {
		log.Fatal("empty file")
	}

	switch *messageType {
	case event:
		eventsShmReader, err := reader.NewEventReader("", *filepath, 5*time.Second, logger)
		if err != nil {
			fmt.Println(err)
			return
		}

		log.Infof("{%s} reading venice events", *filepath)
		for _, evt := range eventsShmReader.Dump() {
			fmt.Println(evt)
		}
		log.Infof("{%s} done reading venice events", *filepath)
	case fwLog:
		mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
		instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
		fwlogShmReader, err := ipc.NewSharedMem(mSize, instCount, *filepath)
		if err != nil {
			fmt.Println(err)
			return
		}

		for i := 0; i < instCount; i++ { // read fwlog from all the instances/partitions in the shared memory
			log.Infof("{%s} reading fwlog events from IPC instance[%v]", *filepath, i)
			ipc := fwlogShmReader.IPCInstance()
			for _, fwlog := range ipc.Dump() {
				fmt.Println(fwlog)
			}
			log.Infof("{%s} done reading fwlog events from IPC instance[%v]", *filepath, i)

		}
	default:
		log.Fatal("invalid message type")
	}
}
