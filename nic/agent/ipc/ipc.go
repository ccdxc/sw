package ipc

import (
	//#include "../../utils/agent_api/shared_constants.c"
	"C"
	"context"
	"encoding/binary"
	"fmt"
	"path/filepath"
	"syscall"
	"time"

	"github.com/golang/protobuf/proto"

	ipcproto "github.com/pensando/sw/nic/agent/ipc/proto"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	shmPath   = "/dev/shm"
	pollDelay = 50 * time.Millisecond
)

// SharedMem is an instance of shared memory that may be
// partitioned into multiple ipc channels.
type SharedMem struct {
	mmapSize  int    // size of shared memory that was mmapped
	mmapAddr  []byte // address to access the shared memory
	maxPart   int    // max number of partitions
	partCount int    // current number of partitions
	fd        int
}

// IPC is one ipc channel
type IPC struct {
	base          []byte
	readIndex     uint32
	writeIndex    uint32
	numBufs       uint32
	txCountIndex  uint32
	errCountIndex uint32
	rxErrors      uint64
	rxCount       uint64
}

// NewSharedMem creates an instance of sharedmem
func NewSharedMem(size, parts int, name string) (*SharedMem, error) {
	if parts <= 0 {
		return nil, fmt.Errorf("partitions must be > 0")
	}

	p := filepath.Join(shmPath, name)
	fd, err := syscall.Open(p, syscall.O_RDWR|syscall.O_CREAT, 0666)
	if err != nil || fd < 0 {
		return nil, fmt.Errorf("Error %s opening %s", err, name)
	}

	err = syscall.Ftruncate(fd, int64(size))
	if err != nil {
		syscall.Close(fd)
		return nil, err
	}

	ma, err := syscall.Mmap(fd, 0, size, syscall.PROT_WRITE|syscall.PROT_READ, syscall.MAP_SHARED)
	if err != nil {
		syscall.Close(fd)
		return nil, err
	}

	log.Infof("size of ma: %v, specified: %v", len(ma), size)
	shm := &SharedMem{
		mmapSize:  size,
		mmapAddr:  ma,
		maxPart:   parts,
		partCount: 0,
		fd:        fd,
	}

	return shm, nil
}

// IPCInstance returns an IPC from the sharedmem
func (sm *SharedMem) IPCInstance() *IPC {
	if sm.partCount == sm.maxPart {
		return nil
	}

	ipcSize := sm.mmapSize / sm.maxPart
	base := sm.mmapAddr[sm.partCount*ipcSize : (sm.partCount+1)*ipcSize]
	numBufs := (ipcSize - int(C.IPC_OVH_SIZE)) / int(C.IPC_BUF_SIZE)
	sm.partCount++
	return &IPC{
		base:       base,
		numBufs:    uint32(numBufs),
		readIndex:  GetSharedConstant("IPC_READ_OFFSET"),
		writeIndex: GetSharedConstant("IPC_WRITE_OFFSET"),
	}
}

// Receive processes messages received on the IPC channel
func (ipc *IPC) Receive(ctx context.Context) {
	for {
		select {
		case <-ctx.Done():
			return
		case <-time.After(pollDelay):
			ipc.processIPC()
		}
	}
}

func (ipc *IPC) processIPC() {
	ro := binary.LittleEndian.Uint32(ipc.base[ipc.readIndex:])
	wo := binary.LittleEndian.Uint32(ipc.base[ipc.writeIndex:])
	avail := int((wo + ipc.numBufs - ro) % ipc.numBufs)
	if avail <= 0 {
		return
	}

	for ix := 0; ix < avail; ix++ {
		ipc.processMsg(ro)
		ro = (ro + 1) % ipc.numBufs
	}

	binary.LittleEndian.PutUint32(ipc.base[ipc.readIndex:], ro)
}

func (ipc *IPC) processMsg(offset uint32) {
	index := GetSharedConstant("IPC_OVH_SIZE") + offset*GetSharedConstant("IPC_BUF_SIZE")
	msgSize := binary.LittleEndian.Uint32(ipc.base[index:])

	log.Infof(":: msgSize is %v, numBufs: %v::", msgSize, ipc.numBufs)
	index += GetSharedConstant("IPC_HDR_SIZE")
	ev := &ipcproto.FWEvent{}
	if err := proto.Unmarshal(ipc.base[index:(index+msgSize)], ev); err != nil {
		log.Errorf("Error %v reading message", err)
		return
	}

	ipc.rxCount++

	log.Infof("=> sip: %v dip: %v sport: %v dport: %v", ev.GetSipV4(), ev.GetDipV4(), ev.GetSport(), ev.GetDport())
}

// GetSharedConstant gets a shared constant from cgo
func GetSharedConstant(c string) uint32 {
	switch c {
	case "IPC_OVH_SIZE":
		return uint32(C.IPC_OVH_SIZE)
	case "IPC_BUF_SIZE":
		return uint32(C.IPC_BUF_SIZE)
	case "IPC_HDR_SIZE":
		return uint32(C.IPC_HDR_SIZE)
	case "IPC_READ_OFFSET":
		return uint32(C.IPC_READ_OFFSET)
	case "IPC_WRITE_OFFSET":
		return uint32(C.IPC_WRITE_OFFSET)
	case "IPC_MEM_SIZE":
		return uint32(C.IPC_MEM_SIZE)
	case "IPC_INSTANCES":
		return uint32(C.IPC_INSTANCES)
	default:
		log.Fatalf("Unknown constant %s", c)
	}

	return 0
}
