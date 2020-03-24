package ipc

import (
	"encoding/binary"
	"fmt"
	"io/ioutil"
	"math"
	"os"
	"reflect"
	"strings"
	"testing"
	"time"

	"github.com/golang/protobuf/proto"

	. "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
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
		Sipv4:     0xc0000001,
		Dipv4:     0x80000001,
		Sport:     uint32(sport),
		Dport:     2000,
		Direction: 1,
		Fwaction:  1,
	}

	out, err := proto.Marshal(ev)
	if err != nil {
		return 0, err
	}

	copy(buf, out)
	return len(out), nil
}

func TestBasicIPC(t *testing.T) {
	f, err := ioutil.TempFile("/tmp", "ipc")
	defer os.Remove(f.Name())

	ipcMemSize := int(GetSharedConstant("IPC_MEM_SIZE"))
	ipcInstances := int(GetSharedConstant("IPC_INSTANCES"))
	shm, err := NewSharedMem(ipcMemSize, ipcInstances, f.Name())
	Assert(t, err == nil, "Failed to open shared mem", err)
	ipc1 := shm.IPCInstance()
	clientShm, err := NewSharedMem(ipcMemSize, ipcInstances, f.Name())
	Assert(t, err == nil, "Failed to open shared mem", err)
	shmInfo := clientShm.String()
	Assert(t, strings.Contains(shmInfo, fmt.Sprintf("size: %d", ipcMemSize)), "invalid size", shmInfo)
	Assert(t, strings.Contains(shmInfo, fmt.Sprintf("channels: %d", ipcInstances)), "invalid channels", shmInfo)

	ipc1Info := ipc1.String()
	Assert(t, strings.Contains(ipc1Info, "readindex: 0"), "invalid readindex", ipc1Info)
	Assert(t, strings.Contains(ipc1Info, "writeindex: 0"), "invalid writeindex", ipc1Info)
	Assert(t, strings.Contains(ipc1Info, "numbuffer: 1598"), "invalid num_buffer", ipc1Info)

	client1 := newIPCClient(clientShm, 0)

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

	callCount := 0
	h := func(ev *FWEvent, ts time.Time) {
		callCount++
	}
	Assert(t, len(ipc1.Dump()) == 2, "expected 2 msgs")
	ipc1.processIPC(h)
	Assert(t, ipc1.rxCount == 2, "Expected 2 msgs")
	Assert(t, callCount == 2, "Expected 3 invocations, got %v", callCount)

	// some -ve cases for coverage
	shm, err = NewSharedMem(0, ipcInstances, "/fwlog_ipc_shm")
	Assert(t, err != nil, "Expected err", err)
	shm, err = NewSharedMem(16, ipcInstances, "/dev/fwlog_ipc_shm")
	Assert(t, err != nil, "Expected err", err)
}

func TestFwEventSize(t *testing.T) {

	ev := &FWEvent{}
	v := reflect.ValueOf(ev).Elem()
	// fill all 1s
	for i := 0; i < v.NumField(); i++ {
		f := v.Field(i)
		switch f.Kind() {
		case reflect.Uint32:
			f.SetUint(math.MaxUint32)
		case reflect.Int32:
			f.SetInt(math.MaxInt32)
		case reflect.Uint64:
			f.SetUint(math.MaxUint64)
		default:
			Assert(t, true, "unhandled kind: ", f.Kind())
		}
	}

	l, err := proto.Marshal(ev)
	AssertOk(t, err, "failed to marshal fwevent")

	ipcBufSize := GetSharedConstant("IPC_BUF_SIZE")
	ipcHdrSize := GetSharedConstant("IPC_HDR_SIZE")
	Assert(t, len(l) < int(ipcBufSize-ipcHdrSize), "too big fwevent got %d, expected < %d", len(l), int(ipcBufSize-ipcHdrSize))
}

func TestWriteMsg(t *testing.T) {
	f, err := ioutil.TempFile("/tmp", "fwlogshm")
	defer os.Remove(f.Name())

	ipcMemSize := int(GetSharedConstant("IPC_MEM_SIZE"))
	ipcInstances := int(GetSharedConstant("IPC_INSTANCES"))
	shm, err := NewSharedMem(ipcMemSize, ipcInstances, f.Name())
	Assert(t, err == nil, "Failed to open shared mem", err)
	ipc1 := shm.IPCInstance()

	numBuff := (ipcMemSize/ipcInstances - int(GetSharedConstant("IPC_OVH_SIZE"))) / int(GetSharedConstant("IPC_BUF_SIZE"))
	for i := 0; i < numBuff-1; i++ {
		ev := &FWEvent{
			Sipv4: uint32(0xFFFF + i),
		}

		err = ipc1.Write(ev)
		AssertOk(t, err, "Failed to write %d event to shm", i+1)
	}

	// should be full
	ev := &FWEvent{
		Sipv4: 0x6666,
	}
	err = ipc1.Write(ev)
	Assert(t, err != nil, "didn't fill shm, numbuff: %d", numBuff)

}
