package shm

import (
	"fmt"
	"os"
	"syscall"

	"github.com/pensando/sw/venice/utils/log"
)

// This file contains functions to perform shm Create/Open/Delete operations.
// Events does not partition the shared memory into multiple instances/segments.
// So, there is only one IPC instance at any point.

// SharedMem is an instance of shared memory.
type SharedMem struct {
	mmapSize int    // size of shared memory that was mmapped
	mmapAddr []byte // address to access the shared memory
	fd       int    // shared mem. file descriptor
}

// IPC represents a partition within the shared memory based.
// there is only one partition on events shared memory.
type IPC struct {
	Base       []byte
	ReadIndex  uint32
	WriteIndex uint32
	NumBuffers uint32
}

// OpenSharedMem opens the shared memory identified by given name.
func OpenSharedMem(path string) (*SharedMem, error) {
	size := getFileSize(path)
	if size == 0 {
		return nil, fmt.Errorf("shm. size: 0")
	}

	return sharedMem(path, size, syscall.O_RDWR)
}

// CreateSharedMem creates shared memory of given name and size.
func CreateSharedMem(path string, size int) (*SharedMem, error) {
	return sharedMem(path, size, syscall.O_RDWR|syscall.O_CREAT)
}

// DeleteSharedMem deletes the shared memory identified by given name.
func DeleteSharedMem(path string) {
	os.Remove(path)
}

// GetIPCInstance returns IPC instance/segment from shared memory.
func (sm *SharedMem) GetIPCInstance() *IPC {
	ipcSize := sm.mmapSize
	base := sm.mmapAddr[0:ipcSize]
	numBufs := (ipcSize - int(GetSharedConstant("IPC_OVH_SIZE"))) / int(GetSharedConstant("SHM_BUF_SIZE"))
	return &IPC{
		Base:       base,
		NumBuffers: uint32(numBufs),
		ReadIndex:  GetSharedConstant("IPC_READ_OFFSET"),
		WriteIndex: GetSharedConstant("IPC_WRITE_OFFSET"),
	}
}

// helper function to open/create shared memory with the given attributes (size, mode, etc.)
func sharedMem(path string, size, mode int) (*SharedMem, error) {
	fd, err := syscall.Open(path, mode, 0666)
	if err != nil || fd < 0 {
		return nil, fmt.Errorf("error creating/opening shm %s, fd: %d, err: %v", path, fd, err)
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

	shm := &SharedMem{
		mmapSize: size,
		mmapAddr: ma,
		fd:       fd,
	}

	return shm, nil
}

// helper function to get the size of the given file.
func getFileSize(path string) int {
	f, err := os.Open(path)
	if err != nil {
		log.Errorf("failed to open shm to get size, err: %v", err)
		return 0
	}

	fs, err := f.Stat()
	if err != nil {
		log.Errorf("failed to get shm size, err: %v", err)
		return 0
	}

	return int(fs.Size())
}
