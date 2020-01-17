package reader

import (
	"context"
	"fmt"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/nevtsproxy/shm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ntranslate"
	// to register key/handles with protobuf; this is needed to convert any to protobuf message
)

// Events reader implementation internally uses shared memory reader (shm/reader.go)
// to read events from a shared memory file. This reader deals with reading events
// from a single shm file.

// Option fills the optional params for evtReader
type Option func(*EvtReader)

// EvtReader reads events from shared memory
type EvtReader struct {
	nodeName       string
	filePath       string
	sm             *shm.SharedMem
	ipcR           *shm.IPCReader
	evtsDispatcher events.Dispatcher
	translator     *ntranslate.Translator
	logger         log.Logger
}

// WithEventsDispatcher passes a custom events dispatcher to dispatch events
func WithEventsDispatcher(evtsDispatcher events.Dispatcher) Option {
	return func(e *EvtReader) {
		e.evtsDispatcher = evtsDispatcher
	}
}

// NewEventReader creates a new events reader
// - Open shared memory identified by the given path and get the IPC instance from shared memory.
// - Use events dispatcher to dispatch events if given.
func NewEventReader(nodeName, path string, pollDelay time.Duration,
	logger log.Logger, opts ...Option) (*EvtReader, error) {

	sm, err := shm.OpenSharedMem(path)
	if err != nil {
		logger.Errorf("failed to open shared memory, err: %v", err)
		return nil, err
	}

	ipc := sm.GetIPCInstance()

	eRdr := &EvtReader{
		nodeName:   nodeName,
		filePath:   path,
		sm:         sm,
		ipcR:       shm.NewIPCReader(ipc, pollDelay),
		translator: ntranslate.MustGetTranslator(),
		logger:     logger,
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
	go r.ipcR.Receive(context.Background(), r.handler)
}

// Dump returns all the available events from shared memory
func (r *EvtReader) Dump() []*halproto.Event {
	return r.ipcR.Dump()
}

// Stop stops the reader
func (r *EvtReader) Stop() {
	r.logger.Infof("received stop, closing reader for shm %s", r.filePath)
	r.ipcR.Stop()
	r.sm.Close() // close the underlying shared mem. file descriptor
}

// TotalEventsRead returns the total events read by this reader so far
func (r *EvtReader) TotalEventsRead() uint64 {
	return r.ipcR.RxCount
}

// TotalErrCount returns the total error count observed at this reader so far
func (r *EvtReader) TotalErrCount() uint64 {
	return r.ipcR.ErrCount
}

// NumPendingEvents returns the total number of pending events to be read from the shared memory
func (r *EvtReader) NumPendingEvents() int {
	return r.ipcR.NumPendingEvents()
}

// message handler to be used by the readers to handle received messages
func (r *EvtReader) handler(nEvt *halproto.Event) error {
	// convert received halproto.Event to venice event
	vEvt := convertToVeniceEvent(nEvt)

	// convert nEvt.ObjectKey to api.ObjectMeta
	if nEvt.ObjectKey != nil {
		dAny := &types.DynamicAny{}
		err := types.UnmarshalAny(nEvt.ObjectKey, dAny)
		if err != nil {
			r.logger.Errorf("failed to unmarshal object key, err: %v", err)
			return err
		}
		// key := dAny.Message.(*halproto.VrfKeyHandle) -> way to convert dynamic any to specific key types
		messageName, err := types.AnyMessageName(nEvt.ObjectKey) // kh.VrfKeyHandle
		if err == nil {
			if meta := r.translator.GetObjectMeta(messageName, dAny); meta != nil {
				// override event's tenant and namespace
				vEvt.ObjectMeta.Tenant = meta.GetTenant()
				vEvt.ObjectMeta.Namespace = meta.GetNamespace()

				// update object ref
				vEvt.EventAttributes.ObjectRef = &api.ObjectRef{
					Tenant:    meta.GetTenant(),
					Namespace: meta.GetNamespace(),
					Name:      meta.GetName(),
					// TODO: update kind, URI
				}
			}
		}
	}

	if r.evtsDispatcher != nil { // dispatch events using the events dispatcher (to venice, syslog, etc..)
		if err := r.evtsDispatcher.Action(*vEvt); err != nil {
			r.logger.Errorf("failed to forward event {%s} from the proxy, err: %v", vEvt.GetUUID(), err)
			return err
		}
	}

	return nil
}

// helper function to convert halproto.Event to venice event
func convertToVeniceEvent(nEvt *halproto.Event) *evtsapi.Event {
	uuid := uuid.NewV4().String()
	t := time.Unix(0, int64(nEvt.GetTime())).UTC() // get time from nsecs
	ts, _ := types.TimestampProto(t)

	eTypeAttrs := eventtypes.GetEventTypeAttrs(eventtypes.EventType(nEvt.GetType()))
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
			Labels:    map[string]string{"_category": globals.Kind2Category("Event")},
		},
		EventAttributes: evtsapi.EventAttributes{
			Type:     eTypeAttrs.EType,
			Severity: eTypeAttrs.Severity,
			Message:  nEvt.GetMessage(),
			Category: eTypeAttrs.Category,
			Source:   &evtsapi.EventSource{Component: nEvt.GetComponent()},
			Count:    1,
		},
	}

	return vEvt
}
