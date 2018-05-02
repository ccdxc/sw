// +build linux

package ipc

import (
	"encoding/binary"
	"testing"

	"github.com/golang/protobuf/proto"

	. "github.com/pensando/sw/nic/agent/ipc/proto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type ipcClient struct {
	base       []byte
	readIndex  uint32
	writeIndex uint32
	numBufs    uint32
}

func newIPCClient(shm *SharedMem, inst int) *ipcClient {
	if inst >= shm.maxPart {
		return nil
	}

	size := shm.mmapSize / shm.maxPart
	return &ipcClient{
		base:       shm.mmapAddr[inst*size : (inst+1)*size],
		numBufs:    uint32((size - int(GetSharedConstant("IPC_OVH_SIZE"))) / int(GetSharedConstant("IPC_BUF_SIZE"))),
		readIndex:  GetSharedConstant("IPC_READ_OFFSET"),
		writeIndex: GetSharedConstant("IPC_WRITE_OFFSET"),
	}
}

func (ic *ipcClient) getBuffer() []byte {
	ri := binary.LittleEndian.Uint32(ic.base[ic.readIndex:])
	wi := binary.LittleEndian.Uint32(ic.base[ic.writeIndex:])
	avail := (ri + ic.numBufs - 1 - wi) % ic.numBufs
	if avail <= 0 {
		return nil
	}

	ipcBufSize := GetSharedConstant("IPC_BUF_SIZE")
	ipcHdrSize := GetSharedConstant("IPC_HDR_SIZE")
	ipcOvhSize := GetSharedConstant("IPC_OVH_SIZE")
	offset := ipcOvhSize + wi*ipcBufSize + ipcHdrSize
	return ic.base[offset:(offset + ipcBufSize - ipcHdrSize)]
}

func (ic *ipcClient) putBuffer(buf []byte, size int) {
	wi := binary.LittleEndian.Uint32(ic.base[ic.writeIndex:])
	ipcOvhSize := GetSharedConstant("IPC_OVH_SIZE")
	ipcBufSize := GetSharedConstant("IPC_BUF_SIZE")
	ipcHdrSize := GetSharedConstant("IPC_HDR_SIZE")
	offset := ipcOvhSize + wi*ipcBufSize
	wi = (wi + 1) % ic.numBufs
	if len(buf) <= int((ipcBufSize - ipcHdrSize)) {
		binary.LittleEndian.PutUint32(ic.base[offset:], uint32(size))
		binary.LittleEndian.PutUint32(ic.base[ic.writeIndex:], wi)
	}
}

func wrFWLog(buf []byte, sport uint16) (int, error) {
	ev := &FWEvent{
		SipV4:     0xc0000001,
		DipV4:     0x80000001,
		Sport:     uint32(sport),
		Dport:     2000,
		Direction: 1,
		Action:    1,
	}

	out, err := proto.Marshal(ev)
	if err != nil {
		return 0, err
	}

	copy(buf, out)
	return len(out), nil
}

func TestBasicIPC(t *testing.T) {
	ipcMemSize := int(GetSharedConstant("IPC_MEM_SIZE"))
	ipcInstances := int(GetSharedConstant("IPC_INSTANCES"))
	shm, err := NewSharedMem(ipcMemSize, ipcInstances, "/fwlog_ipc_shm")
	Assert(t, err == nil, "Failed to open shared mem", err)
	ipc1 := shm.IPCInstance()
	ipc2 := shm.IPCInstance()
	clientShm, err := NewSharedMem(ipcMemSize, ipcInstances, "/fwlog_ipc_shm")
	Assert(t, err == nil, "Failed to open shared mem", err)
	client1 := newIPCClient(clientShm, 0)
	client2 := newIPCClient(clientShm, 1)

	// write two messages tp ipc1 and one to ipc2.
	buf := client1.getBuffer()
	size, err := wrFWLog(buf, 20000)
	Assert(t, err == nil, "Failed to write log", err)
	client1.putBuffer(buf, size)
	buf = client1.getBuffer()
	Assert(t, buf != nil, "failed to get buf")
	size, err = wrFWLog(buf, 20001)
	Assert(t, err == nil, "Failed to write log", err)
	client1.putBuffer(buf, size)
	buf = client2.getBuffer()
	Assert(t, buf != nil, "failed to get buf")
	size, err = wrFWLog(buf, 20002)
	Assert(t, err == nil, "Failed to write log", err)
	client2.putBuffer(buf, size)

	ipc1.processIPC()
	ipc2.processIPC()
	Assert(t, ipc1.rxCount == 2, "Expected 2 msgs")
	Assert(t, ipc2.rxCount == 1, "Expected 1 msg")

	// some -ve cases for coverage
	shm, err = NewSharedMem(0, ipcInstances, "/fwlog_ipc_shm")
	Assert(t, err != nil, "Expected err", err)
	shm, err = NewSharedMem(16, ipcInstances, "/dev/fwlog_ipc_shm")
	Assert(t, err != nil, "Expected err", err)
}
