package shm

import (
	"context"
	"encoding/binary"
	"fmt"
	"sync"
	"time"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
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
type MessageHandler func(*halproto.Event) error

// IPCReader to read messages from shared memory
type IPCReader struct {
	*IPC
	RxCount   uint64         // total messages read on this IPC instance by this reader
	ErrCount  uint64         // total number of failed processing
	stopCh    chan struct{}  // to stop the reader
	wg        sync.WaitGroup // to stop the go routine receiving message form shm in intervals
	pollDelay time.Duration  // poll interval
	once      sync.Once
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
	r.once.Do(func() {
		close(r.stopCh)
		r.wg.Wait()
	})
}

// Receive processes the messages received on IPC channel
func (r *IPCReader) Receive(ctx context.Context, handler MessageHandler) {
	r.wg.Add(1)
	defer r.wg.Done()

	for {
		select {
		case <-ctx.Done():
			return
		case <-time.After(r.pollDelay):
			r.receiveMessage(handler)
		case <-r.stopCh:
			return
		}
	}
}

// Dump returns all the available events from underlying shared memory
func (r *IPCReader) Dump() []*halproto.Event {
	var evts []*halproto.Event
	ro := uint32(0)
	wo := binary.LittleEndian.Uint32(r.Base[r.WriteIndex:])
	avail := int((wo + r.NumBuffers - ro) % r.NumBuffers)
	log.Debugf("reading events from index[%v]...[%v], total messages to be read: %v", ro, wo, avail)
	if avail <= 0 {
		return evts
	}

	for ix := 0; ix < avail; ix++ {
		evt, err := r.readMsg(ro)
		if err != nil {
			log.Errorf("failed to read message from index[%v], err: %v", ro, err)
			continue
		}
		evts = append(evts, evt)
		ro = (ro + 1) % r.NumBuffers
	}

	return evts
}

// helper function to process the received message
func (r *IPCReader) receiveMessage(handler MessageHandler) {
	ro := binary.LittleEndian.Uint32(r.Base[r.ReadIndex:])
	wo := binary.LittleEndian.Uint32(r.Base[r.WriteIndex:])
	avail := int((wo + r.NumBuffers - ro) % r.NumBuffers)
	if avail <= 0 {
		return
	}

	ts := time.Now()
	for ix := 0; ix < avail; ix++ {
		r.processMsg(ro, ts, handler)
		ro = (ro + 1) % r.NumBuffers
	}

	binary.LittleEndian.PutUint32(r.Base[r.ReadIndex:], ro)
}

// NumPendingEvents number of pending events yet to be read from the shared memory
func (r *IPCReader) NumPendingEvents() int {
	ro := binary.LittleEndian.Uint32(r.Base[r.ReadIndex:])
	wo := binary.LittleEndian.Uint32(r.Base[r.WriteIndex:])
	return int((wo + r.NumBuffers - ro) % r.NumBuffers)
}

// helper function to process the received message
func (r *IPCReader) processMsg(offset uint32, ts time.Time, handler MessageHandler) {
	evt, err := r.readMsg(offset)
	if err != nil {
		r.ErrCount++
		return
	}
	r.RxCount++

	// send it to the message handler for further processing
	if handler != nil {
		if err := handler(evt); err != nil {
			r.ErrCount++
		}
	}
}

// readMsg reads halproto.Event from the given offset
func (r *IPCReader) readMsg(offset uint32) (*halproto.Event, error) {
	index := GetSharedConstant("IPC_OVH_SIZE") + offset*GetSharedConstant("SHM_BUF_SIZE")
	msgSize := binary.LittleEndian.Uint32(r.Base[index:]) // get message size from buffer
	index += GetSharedConstant("IPC_HDR_SIZE")            // update the Base pointer of the message

	evt := &halproto.Event{}
	if (index + msgSize) >= uint32(len(r.Base)) {
		err := fmt.Errorf("length %d out of bounds %d when reading Event from shared memory", index+msgSize, len(r.Base))
		log.Error(err.Error())
		return nil, err
	}
	if err := proto.Unmarshal(r.Base[index:(index+msgSize)], evt); err != nil {
		log.Errorf("error reading message to halproto.Event from shared memory, err: %v", err)
		return nil, err
	}

	return evt, nil
}
