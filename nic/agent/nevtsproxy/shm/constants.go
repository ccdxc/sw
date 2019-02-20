package shm

import (
	//#include "../../../utils/events/recorder/constants.h"
	"C"

	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/venice/utils/log"
)

// GetSharedMemoryDirectory returns the shared memory directory used at the c++ events recorder library.
func GetSharedMemoryDirectory() string {
	return "/dev/shm"
}

// GetSharedConstant gets a shared constant using cgo.
func GetSharedConstant(c string) uint32 {
	switch c {
	case "IPC_OVH_SIZE", "IPC_HDR_SIZE", "IPC_READ_OFFSET", "IPC_WRITE_OFFSET", "IPC_TOTAL_WRITES":
		return ipc.GetSharedConstant(c)
	case "SHM_BUF_SIZE":
		return uint32(C.SHM_BUF_SIZE)
	default:
		log.Fatalf("Unknown constant %s", c)
		return 0
	}
}
