package reader

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"testing"
	"time"

	google_protobuf1 "github.com/gogo/protobuf/types"
	"github.com/golang/protobuf/proto"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/nevtsproxy/shm"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// NOTE: this test will not work with -race as the writers and reader are working on a single shared memory, there will be race.

var (
	rdrLogger = log.GetNewLogger(log.GetDefaultConfig("events-reader-test"))
)

// TestReader tests the reader by creating multiple shm writers
// and let the reader handle the rest (file watcher should create event readers
// and receive events). It also ensures readers and writers are in sync.
func TestReader(t *testing.T) {
	tLogger := rdrLogger.WithContext("t_name", t.Name())
	dir, err := ioutil.TempDir("", "shm")
	AssertOk(t, err, "failed to create temp dir")
	defer os.RemoveAll(dir)

	var wg sync.WaitGroup
	wg.Add(1)
	var rdr *Reader
	go func() {
		defer wg.Done()

		rdr = NewReader(t.Name(), dir, 10*time.Millisecond, nil, tLogger)
		err := rdr.Start()
		AssertOk(t, err, "failed start file watcher, err: %v", err)
	}()
	wg.Wait()

	numWriters := 5
	shmPathToIndex := map[string]int{}
	totalEventsRecorded := make([]uint64, numWriters)
	stopWriters := make(chan struct{}, 1)
	var writerWG sync.WaitGroup
	writerWG.Add(numWriters)

	shmSize := 1024
	totalBuffers := (uint32(shmSize)-shm.GetSharedConstant("IPC_OVH_SIZE"))/shm.GetSharedConstant("SHM_BUF_SIZE") - 1

	halKey, err := google_protobuf1.MarshalAny(&halproto.VrfKeyHandle{KeyOrHandle: &halproto.VrfKeyHandle_VrfId{VrfId: 1001}})
	AssertOk(t, err, "failed to marshal vrf key to any, err: %v", err)

	// spin up multiple writers; each writer will create a shm of it's own and start writing events.
	for i := 0; i < numWriters; i++ {
		shmPath := filepath.Join(dir, uuid.NewV4().String()) + ".events"
		shmPathToIndex[shmPath] = i

		sm, err := shm.CreateSharedMem(shmPath, shmSize)
		AssertOk(t, err, "failed to create shared memory, err: %v", err)
		ipc := sm.GetIPCInstance()
		ipcW := shm.NewIPCWriter(ipc) // create IPC writer

		go func(shmPath string, index int, writer *shm.IPCWriter) {
			defer writerWG.Done()
			for {
				select {
				case <-stopWriters:
					return
				case <-time.After(10 * time.Millisecond):
					hEvt := &halproto.Event{
						Type:      int32(eventtypes.SERVICE_RUNNING),
						Component: "shm-reader-test",
						Message:   "test-msg",
						ObjectKey: halKey,
					}

					msgSize := hEvt.Size()
					buf := ipcW.GetBuffer(msgSize) // get buffer
					if buf == nil {
						log.Errorf("failed to get buffer")
						continue
					}

					out, err := proto.Marshal(hEvt)
					AssertOk(t, err, "failed to marshal event: %s", shmPath)
					copy(buf, out)
					ipcW.PutBuffer(buf, msgSize)
					totalEventsRecorded[index]++
				}
			}

		}(shmPath, i, ipcW)
	}

	// writers will continue to send events for this duration. If the readers were not receiving messages that were
	// sent by these writers, then the buffer will run out of space and cause the writer to fail.
	time.Sleep(2 * time.Second)
	close(stopWriters)
	writerWG.Wait() // wait for all the writers to finish

	// ensure all the messages were received by the event readers
	for _, eRdr := range rdr.GetEventReaders() {
		AssertEventually(t, func() (bool, interface{}) {
			shmPath := eRdr.filePath
			index, ok := shmPathToIndex[eRdr.filePath]
			if !ok {
				return false, fmt.Sprintf("shm: %s, expected: %d, got: %d", shmPath, 0, eRdr.TotalEventsRead())
			}

			sent := totalEventsRecorded[index]
			if sent <= uint64(totalBuffers) { // because the reader should have read the messages
				return false, fmt.Sprintf("shm: %s, total sent messages expected: > %d, got: %d", shmPath, totalBuffers, sent)
			}

			received := eRdr.TotalEventsRead()
			if sent == received {
				log.Infof("shm: %s, sent: %d, received: %d", shmPath, sent, received)
				return true, nil
			}

			return false, fmt.Sprintf("shm: %s, expected: %d, got: %d", shmPath, sent, received)
		}, "unexpected number of events", string("5ms"), string("5s"))
	}

	rdr.Stop() // stop the reader (all the event readers reading from shm files will be stopped)
}

// TestReaderInstantiation tests reader instantiation
func TestReaderInstantiation(t *testing.T) {
	tLogger := rdrLogger.WithContext("t_name", t.Name())
	rdr := NewReader(t.Name(), "/invalid", time.Second, nil, tLogger)
	err := rdr.Start()
	Assert(t, strings.Contains(err.Error(), "no such file or directory"), "expected failure, init succeeded")

	dir, err := ioutil.TempDir("", "shm")
	AssertOk(t, err, "failed to create temp dir")

	f1, err := ioutil.TempFile(dir, "*tfile.events")
	AssertOk(t, err, "failed to create temp file")
	defer f1.Close()

	rdr = NewReader(t.Name(), dir, time.Second, nil, tLogger)
	err = rdr.Start()
	AssertOk(t, err, "failed to start file watcher, err: %v", err)

	// invalid file name, files should end with ".events"
	f2, err := ioutil.TempFile(dir, "*tfile")
	AssertOk(t, err, "failed to create temp file")
	defer f2.Close()

	// wait for file readers to be created
	time.Sleep(1 * time.Second)

	// delete the directory under watch
	// watcher should be intact event after the directory is deleted; watcher will receive the REMOVE event on the directory
	os.RemoveAll(dir)

	rdr.Stop() // stop will close the watcher

	time.Sleep(10 * time.Millisecond)
	_, ok := <-rdr.fileWatcher.Events
	Assert(t, !ok, "file watcher should have been closed")
}
