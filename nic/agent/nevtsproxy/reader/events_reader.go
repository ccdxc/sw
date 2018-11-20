package reader

import (
	"context"
	"fmt"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/nevtsproxy/shm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	// to register key/handles with protobuf; this is needed to convert any to protobuf message
	_ "github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

// Events reader implementation internally uses shared memory reader (shm/reader.go)
// to read events from a shared memory file. This reader deals with reading events
// from a single shm file.

// Option fills the optional params for evtReader
type Option func(*EvtReader)

// EvtReader reads events from shared memory
type EvtReader struct {
	filePath       string
	sm             *shm.SharedMem
	ipcR           *shm.IPCReader
	evtsDispatcher events.Dispatcher
}

// WithEventsDispatcher passes a custom events dispatcher to dispatch events
func WithEventsDispatcher(evtsDispatcher events.Dispatcher) Option {
	return func(e *EvtReader) {
		e.evtsDispatcher = evtsDispatcher
	}
}

// NewEventReader creates a new events reader
// - Open shared memory identified by the given name (path) and get the IPC instance from shared memory.
// - Use events dispatcher to dispatch events if given.
func NewEventReader(path string, pollDelay time.Duration, opts ...Option) (*EvtReader, error) {
	sm, err := shm.OpenSharedMem(path)
	if err != nil {
		log.Errorf("failed to open shared memory, err: %v", err)
		return nil, err
	}

	ipc := sm.GetIPCInstance()

	eRdr := &EvtReader{
		filePath: path,
		sm:       sm,
		ipcR:     shm.NewIPCReader(ipc, pollDelay),
	}

	for _, opt := range opts {
		if opt != nil {
			opt(eRdr)
		}
	}

	return eRdr, nil
}

// Start start receiving events from shared memory
func (r *EvtReader) Start() {
	go r.ipcR.Receive(context.Background(), halproto.Event{}, r.handler)
}

// Stop stops the reader
func (r *EvtReader) Stop() {
	log.Infof("received stop, closing reader for shm %s", r.filePath)
	r.ipcR.Stop()
}

// TotalEventsRead returns the total events read by this reader so far
func (r *EvtReader) TotalEventsRead() uint64 {
	return r.ipcR.RxCount
}

// TotalErrCount returns the total error count observed at this reader so far
func (r *EvtReader) TotalErrCount() uint64 {
	return r.ipcR.ErrCount
}

// message handler to be used by the readers to handle received messages
func (r *EvtReader) handler(msg interface{}) error {
	nEvt, ok := msg.(*halproto.Event)
	if !ok {
		err := fmt.Errorf("failed to type cast the message from shared memory to halproto.Event")
		return err
	}

	// convert received halproto.Event to venice event
	vEvt := convertToVeniceEvent(nEvt)

	// convert nEvt.ObjectKey to api.ObjectMeta
	// TODO: update tenant, namespace and object ref (object key to venice name conversion)
	if nEvt.ObjectKey != nil {
		dAny := &types.DynamicAny{}
		err := types.UnmarshalAny(nEvt.ObjectKey, dAny)
		if err != nil {
			log.Errorf("failed to unmarshal object key, err: %v", err)
			return err
		}
	}

	if r.evtsDispatcher != nil { // dispatch events using the events dispatcher (to venice, syslog, etc..)
		if err := r.evtsDispatcher.Action(*vEvt); err != nil {
			log.Errorf("failed to forward event {%s} from the proxy, err: %v", vEvt.GetUUID(), err)
			return err
		}
	}

	return nil
}

// helper function to convert halproto.Event to venice event
func convertToVeniceEvent(nEvt *halproto.Event) *evtsapi.Event {
	uuid := uuid.NewV4().String()
	t := time.Unix(int64(nEvt.GetTime()), 0).UTC()
	ts, _ := types.TimestampProto(t)

	vEvt := &evtsapi.Event{ // create event object
		TypeMeta: api.TypeMeta{Kind: "Event"},
		ObjectMeta: api.ObjectMeta{
			Name: uuid,
			UUID: uuid,
			CreationTime: api.Timestamp{
				Timestamp: *ts,
			},
			ModTime: api.Timestamp{
				Timestamp: *ts,
			},
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			SelfLink:  fmt.Sprintf("/events/v1/events/%s", uuid),
			Labels:    map[string]string{"_category": globals.Kind2Category["Event"]},
		},
		EventAttributes: evtsapi.EventAttributes{
			Type:     nEvt.GetType(),
			Severity: halproto.Severity_name[int32(nEvt.GetSeverity())],
			Message:  nEvt.GetMessage(),
			Source:   &evtsapi.EventSource{Component: nEvt.GetComponent(), NodeName: utils.GetHostname()},
			Count:    1,
		},
	}

	return vEvt
}
