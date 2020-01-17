package ipc

import (
	//#include "../../utils/ipc/constants.h"
	//#include "../../utils/agent_api/constants.h"
	"C"
	"context"
	"encoding/binary"
	"fmt"
	"os"
	"path/filepath"
	"syscall"
	"time"

	"github.com/golang/protobuf/proto"

	ipcproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
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

	p := name
	if _, err := os.Stat(name); os.IsNotExist(err) {
		p = filepath.Join(shmPath, name) // look in /dev/shm if the file does not exists
	}

	fd, err := syscall.Open(p, syscall.O_RDWR|syscall.O_CREAT, 0666)
	if err != nil || fd < 0 {
		return nil, fmt.Errorf("Error %s opening %s", err, p)
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

	log.Infof("attached shared memory, size: %v, requested: %v", len(ma), size)
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
	log.Infof("IPC[%d] allocated shared memory %v:%v num_buffers: %d ovh_len: %d hdr_len: %d buff_len: %d", sm.partCount, sm.partCount*ipcSize, (sm.partCount+1)*ipcSize, numBufs, int(C.IPC_OVH_SIZE), int(C.IPC_HDR_SIZE), int(C.IPC_BUF_SIZE))
	sm.partCount++
	return &IPC{
		base:       base,
		numBufs:    uint32(numBufs),
		readIndex:  GetSharedConstant("IPC_READ_OFFSET"),
		writeIndex: GetSharedConstant("IPC_WRITE_OFFSET"),
	}
}

// String prints SharedMem details
func (sm *SharedMem) String() string {
	return fmt.Sprintf("size: %v, channels: %v", sm.mmapSize, sm.maxPart)
}

// Receive processes messages received on the IPC channel
func (ipc *IPC) Receive(ctx context.Context, h func(*ipcproto.FWEvent, time.Time)) {
	for {
		select {
		case <-ctx.Done():
			return
		case <-time.After(pollDelay):
			ipc.processIPC(h)
		}
	}
}

// Dump dumps all the available fw events from the shared memory
func (ipc *IPC) Dump() []*ipcproto.FWEvent {
	var evts []*ipcproto.FWEvent
	ro := uint32(0)
	wo := binary.LittleEndian.Uint32(ipc.base[ipc.writeIndex:])
	avail := int((wo + ipc.numBufs - ro) % ipc.numBufs)
	log.Debugf("reading fwlog events from index[%v]...[%v], total messages to be read: %v", ro, wo, avail)
	if avail <= 0 {
		return evts
	}

	for ix := 0; ix < avail; ix++ {
		ev, err := ipc.readMsg(ro)
		if err != nil {
			log.Errorf("failed to read message from index[%v], err: %v", ro, err)
			continue
		}
		evts = append(evts, ev)
		ro = (ro + 1) % ipc.numBufs
	}

	return evts
}

func (ipc *IPC) processIPC(h func(*ipcproto.FWEvent, time.Time)) {
	ro := binary.LittleEndian.Uint32(ipc.base[ipc.readIndex:])
	wo := binary.LittleEndian.Uint32(ipc.base[ipc.writeIndex:])
	avail := int((wo + ipc.numBufs - ro) % ipc.numBufs)
	if avail <= 0 {
		return
	}

	ts := time.Now().UnixNano()
	for ix := 0; ix < avail; ix++ {
		// timestamp is not set for fwlog events from HAL
		// generate different timestamps for each event
		// TODO: revisit
		ipc.processMsg(ro, time.Unix(0, ts+int64(ix)), h)
		ro = (ro + 1) % ipc.numBufs
	}

	binary.LittleEndian.PutUint32(ipc.base[ipc.readIndex:], ro)
}

// Write is the function to send fwlog event to shm, used in tests
func (ipc *IPC) Write(event *ipcproto.FWEvent) error {
	ro := binary.LittleEndian.Uint32(ipc.base[ipc.readIndex:])
	wo := binary.LittleEndian.Uint32(ipc.base[ipc.writeIndex:])
	if (wo+1)%ipc.numBufs == ro {
		return fmt.Errorf("fwlog shm is full")
	}

	if err := ipc.writeMsg(wo, event); err != nil {
		return fmt.Errorf("failed to write to offset %v, %v", wo, err)
	}

	wo = (wo + 1) % ipc.numBufs
	binary.LittleEndian.PutUint32(ipc.base[ipc.writeIndex:], wo)
	return nil
}

func (ipc *IPC) processMsg(offset uint32, ts time.Time, h func(*ipcproto.FWEvent, time.Time)) {
	ev, err := ipc.readMsg(offset)
	if err != nil {
		return
	}

	ipc.rxCount++

	h(ev, ts)
}

func (ipc *IPC) readMsg(offset uint32) (*ipcproto.FWEvent, error) {
	index := GetSharedConstant("IPC_OVH_SIZE") + offset*GetSharedConstant("IPC_BUF_SIZE")
	msgSize := binary.LittleEndian.Uint32(ipc.base[index:])

	index += GetSharedConstant("IPC_HDR_SIZE")
	ev := &ipcproto.FWEvent{}
	if err := proto.Unmarshal(ipc.base[index:(index+msgSize)], ev); err != nil {
		log.Errorf("Error %v reading message", err)
		return nil, err
	}

	return ev, nil
}

func (ipc *IPC) writeMsg(offset uint32, event *ipcproto.FWEvent) error {
	data, err := proto.Marshal(event)
	if err != nil {
		return fmt.Errorf("failed to marshal data")
	}

	index := GetSharedConstant("IPC_OVH_SIZE") + offset*GetSharedConstant("IPC_BUF_SIZE")
	binary.LittleEndian.PutUint32(ipc.base[index:], uint32(len(data)))
	index += GetSharedConstant("IPC_HDR_SIZE")

	copy(ipc.base[index:], data[0:])
	return nil
}

// String prints IPC details
func (ipc *IPC) String() string {
	ro := binary.LittleEndian.Uint32(ipc.base[ipc.readIndex:])
	wo := binary.LittleEndian.Uint32(ipc.base[ipc.writeIndex:])

	return fmt.Sprintf("readindex: %v (@%v) writeindex: %v (@%v) numbuffer: %v txCountIndex: %v errCountIndex: %v rxErrors: %v rxCount: %v",
		ro, ipc.readIndex, wo, ipc.writeIndex, ipc.numBufs, ipc.txCountIndex, ipc.errCountIndex, ipc.rxErrors, ipc.rxCount)
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
