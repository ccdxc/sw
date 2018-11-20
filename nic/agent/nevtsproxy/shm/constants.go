package shm

import (
	//#include "../../../utils/ipc/constants.h"
	//#include "../../../utils/events/recorder/constants.h"
	"C"

	"github.com/pensando/sw/venice/utils/log"
)

// GetSharedMemoryDirectory returns the shared memory directory used at the c++ events recorder library.
func GetSharedMemoryDirectory() string {
	return "/dev/shm/pen-events"
}

// GetSharedConstant gets a shared constant using cgo.
func GetSharedConstant(c string) uint32 {
	switch c {
	case "IPC_OVH_SIZE":
		return uint32(C.IPC_OVH_SIZE)
	case "IPC_HDR_SIZE":
		return uint32(C.IPC_HDR_SIZE)
	case "IPC_READ_OFFSET":
		return uint32(C.IPC_READ_OFFSET)
	case "IPC_WRITE_OFFSET":
		return uint32(C.IPC_WRITE_OFFSET)
	case "IPC_TOTAL_WRITES":
		return uint32(C.IPC_TOTAL_WRITES)
	case "SHM_BUF_SIZE":
		return uint32(C.SHM_BUF_SIZE)
	default:
		log.Fatalf("Unknown constant %s", c)
		return 0
	}
}
