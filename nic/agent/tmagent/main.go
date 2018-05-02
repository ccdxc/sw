package main

import (
	"context"

	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/venice/utils/log"
)

func main() {
	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	shm, err := ipc.NewSharedMem(mSize, instCount, "/fwlog_ipc_shm")
	if err != nil {
		log.Fatal(err)
	}

	ipc := shm.IPCInstance()
	ipc.Receive(context.Background())
}
