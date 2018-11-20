package shm

import (
	"context"
	"encoding/binary"
	"reflect"
	"sync"
	"time"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

//
// IPC reader implementation.
//
// This module is responsible to:
// - Read messages from the shared memory in intervals (pollDelay) and update the read index.
// - Convert received messages to requested type (e.g. halproto.Event). And,
//    run the given message handler on it.
//

// MessageHandler handler that will be run on each received message
type MessageHandler func(interface{}) error

// IPCReader to read messages from shared memory
type IPCReader struct {
	*IPC
	RxCount   uint64         // total messages read on this IPC instance by this reader
	ErrCount  uint64         // total number of failed processing
	stopCh    chan struct{}  // to stop the reader
	wg        sync.WaitGroup // to stop the go routine receiving message form shm in intervals
	pollDelay time.Duration  // poll interval
}

// NewIPCReader returns the new IPC reader
func NewIPCReader(ipc *IPC, pollDelay time.Duration) *IPCReader {
	return &IPCReader{
		IPC:       ipc,
		stopCh:    make(chan struct{}, 1),
		pollDelay: pollDelay,
	}
}

// Stop stops the go routine receiving messages
func (r *IPCReader) Stop() {
	close(r.stopCh)
	r.wg.Wait()
}

// Receive processes the messages received on IPC channel
func (r *IPCReader) Receive(ctx context.Context, objType interface{}, handler MessageHandler) {
	r.wg.Add(1)
	defer r.wg.Done()

	for {
		select {
		case <-ctx.Done():
			return
		case <-time.After(r.pollDelay):
			r.receiveMessage(objType, handler)
		case <-r.stopCh:
			return
		}
	}
}

// helper function to process the received message
func (r *IPCReader) receiveMessage(objType interface{}, handler MessageHandler) {
	ro := binary.LittleEndian.Uint32(r.Base[r.ReadIndex:])
	wo := binary.LittleEndian.Uint32(r.Base[r.WriteIndex:])
	avail := int((wo + r.NumBuffers - ro) % r.NumBuffers)
	if avail <= 0 {
		return
	}

	ts := time.Now()
	for ix := 0; ix < avail; ix++ {
		r.processMsg(ro, ts, objType, handler)
		ro = (ro + 1) % r.NumBuffers
	}

	binary.LittleEndian.PutUint32(r.Base[r.ReadIndex:], ro)
}

// helper function to process the received message
func (r *IPCReader) processMsg(offset uint32, ts time.Time, objType interface{}, handler MessageHandler) {
	index := GetSharedConstant("IPC_OVH_SIZE") + offset*GetSharedConstant("SHM_BUF_SIZE")
	msgSize := binary.LittleEndian.Uint32(r.Base[index:]) // get message size from buffer
	index += GetSharedConstant("IPC_HDR_SIZE")            // update the Base pointer of the message

	switch objType.(type) {
	case halproto.Event:
		evt := &halproto.Event{}
		if err := proto.Unmarshal(r.Base[index:(index+msgSize)], evt); err != nil {
			log.Errorf("error reading message to halproto.Event from shared memory, err: %v", err)
			r.ErrCount++
			return
		}

		// send it to the message handler for further processing
		if handler != nil {
			if err := handler(evt); err != nil {
				r.ErrCount++
			}
		}
		r.RxCount++
	default:
		r.ErrCount++
		log.Errorf("object type not supported: %v", reflect.TypeOf(objType))
		return
	}
}
