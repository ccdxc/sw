package shm

import "encoding/binary"

// This writer implementation should be used only by go tests; this is a workaround to avoid
// dealing with cgo.

// IPCWriter to write messages to the shared memory
type IPCWriter struct {
	*IPC
}

// NewIPCWriter creates new IPC writer
func NewIPCWriter(ipc *IPC) *IPCWriter {
	return &IPCWriter{ipc}
}

// GetBuffer allocates a buffer and returns the address
func (ic *IPCWriter) GetBuffer(size int) []byte {
	ipcBufSize := GetSharedConstant("SHM_BUF_SIZE")
	ipcHdrSize := GetSharedConstant("IPC_HDR_SIZE")
	ipcOvhSize := GetSharedConstant("IPC_OVH_SIZE")

	if (uint32(size) + ipcHdrSize) > ipcBufSize { // if the requested size > buffer size
		return nil
	}

	ri := binary.LittleEndian.Uint32(ic.Base[ic.ReadIndex:])
	wi := binary.LittleEndian.Uint32(ic.Base[ic.WriteIndex:])
	avail := (ri + ic.NumBuffers - 1 - wi) % ic.NumBuffers
	if avail <= 0 { // no available empty buffers
		return nil
	}

	offset := ipcOvhSize + wi*ipcBufSize + ipcHdrSize
	return ic.Base[offset:(offset + ipcBufSize - ipcHdrSize)]
}

// PutBuffer writes the size to given buffer and updates write index
func (ic *IPCWriter) PutBuffer(buf []byte, size int) {
	wi := binary.LittleEndian.Uint32(ic.Base[ic.WriteIndex:])
	ipcOvhSize := GetSharedConstant("IPC_OVH_SIZE")
	ipcBufSize := GetSharedConstant("SHM_BUF_SIZE")
	ipcHdrSize := GetSharedConstant("IPC_HDR_SIZE")
	offset := ipcOvhSize + wi*ipcBufSize
	wi = (wi + 1) % ic.NumBuffers
	if len(buf) <= int(ipcBufSize-ipcHdrSize) {
		binary.LittleEndian.PutUint32(ic.Base[offset:], uint32(size))
		binary.LittleEndian.PutUint32(ic.Base[ic.WriteIndex:], wi)
	}
}
