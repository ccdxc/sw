// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package recorder

import (
	"context"
	"encoding/json"
	"fmt"
	"path/filepath"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"
	"google.golang.org/grpc/connectivity"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	evtsproxygrpc "github.com/pensando/sw/venice/evtsproxy/rpcserver/evtsproxyproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

// Recorder library to be used by the events sources to record events in the system.
// Event sources(clients) can use singleton recorder or the recorder object returned from
// `NewRecorder(...)` to record events. Singleton recorder gets instantiated when `NewRecorder(...)`
// is called for the very first time. Any further calls to `NewRecorder(...)` will not change
// singleton recorder.

var singletonRecorder *recorderImpl
var once sync.Once

// Event records the given event
func Event(eventType string, severity evtsapi.SeverityLevel, message string, objRef interface{}) {
	if singletonRecorder == nil {
		log.Fatal("initialize events recorder")
	}

	singletonRecorder.Event(eventType, severity, message, objRef)
}

// NOTE: the recorder does not ensure any ordering in the delivery of events during any failure (proxy restart, recorder restart)

// recorderImpl implements `Recorder` interface. Events sources at venice will
// use this recorder to generate events with the given severity, type,
// messsage, etc. Events are sent to the proxy for further processing (dedup, cache, etc.)
type recorderImpl struct {
	sync.Mutex                                   // to protect access to the recorder
	id                    string                 // id (unique key) of the recorder
	eventSource           *evtsapi.EventSource   // all the events generated using this recorder will carry this source
	eventTypes            map[string]struct{}    // eventTypes provide a function to validate the given event type
	eventsProxy           *eventsProxy           // event proxy
	eventsFile            *fileImpl              // events backup store
	failedEventsForwarder *failedEventsForwarder // used to forward failed events to the proxy
}

// eventsProxy encapsulates all the proxy details including connection string
// and context to be used while making calls.
type eventsProxy struct {
	sync.Mutex
	ctx             context.Context                    // ctx for making calls to events proxy
	url             string                             // events proxy URL
	rpcClient       *rpckit.RPCClient                  // RPC client for events proxy
	client          evtsproxygrpc.EventsProxyAPIClient // events proxy client connection
	connectionAlive bool                               // represents the connection status; alive or dead
}

// failedEventsForwarder helps to forward failed events to the proxy
type failedEventsForwarder struct {
	sync.Mutex
	tick *time.Ticker  // failed call to the proxy will be retried as per this ticker
	stop chan struct{} // to stop any outstanding process that is processing the failed events
	wg   sync.WaitGroup
}

// NewRecorder creates and returns a recorder instance and instantiates the singleton object.
// if `evtsproxyURL` is empty, it will it use local events proxy RPC port.
// First recorder instance created in the process is same as the singleton recorder.
func NewRecorder(source *evtsapi.EventSource, eventTypes []string, evtsproxyURL, eventsBackupDir string) (events.Recorder, error) {
	if source == nil {
		return nil, fmt.Errorf("missing event source")
	}

	if eventTypes == nil {
		return nil, fmt.Errorf("missing event types")
	}

	if len(eventTypes) == 0 {
		return nil, fmt.Errorf("empty event types")
	}

	if utils.IsEmpty(evtsproxyURL) {
		evtsproxyURL = fmt.Sprintf(":%s", globals.EvtsProxyRPCPort)
	}

	if utils.IsEmpty(eventsBackupDir) {
		eventsBackupDir = filepath.Join(globals.EventsDir, "recorder")
	}

	eventsFile, err := newFile(eventsBackupDir, events.GetSourceKey(source))
	if err != nil {
		return nil, err
	}

	recorder := &recorderImpl{
		id:          fmt.Sprintf("%s-%s", source.GetNodeName(), source.GetComponent()),
		eventSource: source,
		eventTypes:  constructEventTypesMap(eventTypes),
		eventsProxy: &eventsProxy{
			url: evtsproxyURL,
			ctx: context.Background(), // context for all the proxy calls
		},
		eventsFile: eventsFile,
		failedEventsForwarder: &failedEventsForwarder{
			tick: time.NewTicker(2 * time.Second),
			stop: make(chan struct{}),
		},
	}

	// create events proxy client
	go recorder.createEvtsProxyRPCClient()

	once.Do(func() {
		singletonRecorder = recorder
	})

	return recorder, nil
}

// Event records the event by creating a event using the given type, severity, message, etc.
// and sending it to the events proxy for further processing.
// Event sources will call this to record an event.
func (r *recorderImpl) Event(eventType string, severity evtsapi.SeverityLevel, message string, objRef interface{}) {
	if err := r.validate(eventType, severity); err != nil {
		log.Fatalf("validation failed, err: %v", err)
	}

	var objRefMeta *api.ObjectMeta
	var objRefKind string
	var err error

	// derive reference object details from the given object
	if objRef != nil {
		objRefMeta, err = runtime.GetObjectMeta(objRef)
		if err != nil {
			log.Fatalf("failed to get the object meta from reference object, err: %v", err)
		}

		objRefKind = objRef.(runtime.Object).GetObjectKind()
	}

	// create UUID/name for the event
	uuid := uuid.NewV4().String()

	creationTime, _ := types.TimestampProto(time.Now())

	// create object meta for the event object
	meta := api.ObjectMeta{
		Name: uuid,
		UUID: uuid,
		CreationTime: api.Timestamp{
			Timestamp: *creationTime,
		},
		ModTime: api.Timestamp{
			Timestamp: *creationTime,
		},
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}

	// create event object
	event := &evtsapi.Event{
		TypeMeta:   api.TypeMeta{Kind: "Event"},
		ObjectMeta: meta,
		EventAttributes: evtsapi.EventAttributes{
			Type:     eventType,
			Severity: evtsapi.SeverityLevel_name[int32(severity)],
			Message:  message,
			Source:   r.eventSource,
			Count:    1,
		},
	}

	// FIXME: update self link

	if objRefMeta != nil {
		// update namespace and tenant
		event.ObjectMeta.Namespace = objRefMeta.GetNamespace()
		event.ObjectMeta.Tenant = objRefMeta.GetTenant()

		// update object ref
		event.ObjectRef = &api.ObjectRef{
			Tenant:    objRefMeta.GetTenant(),
			Namespace: objRefMeta.GetNamespace(),
			Kind:      objRefKind,
			Name:      objRefMeta.GetName(),
			URI:       objRefMeta.GetSelfLink(),
		}
	}

	// send the event to a file or proxy
	if err := r.sendEvent(event); err != nil {
		log.Fatalf("failed to record event %v, err: %v", event.GetUUID(), err)
	}
}

// validate validates the given event type and severity
func (r *recorderImpl) validate(eType string, severity evtsapi.SeverityLevel) error {
	// validate event
	_, found := r.eventTypes[eType]
	if !found {
		return events.NewError(events.ErrInvalidEventType, "")
	}

	// validate severity
	if _, ok := evtsapi.SeverityLevel_name[int32(severity)]; !ok {
		return events.NewError(events.ErrInvalidSeverity, "")
	}

	return nil
}

// sendEvent helper function to send the event to proxy.
func (r *recorderImpl) sendEvent(event *evtsapi.Event) error {
	r.eventsProxy.Lock()
	defer r.eventsProxy.Unlock()

	if !r.eventsProxy.connectionAlive {
		log.Debug("connection to evtsproxy unavailable. so, writing to a file")
		return r.writeToFile(event)
	}

	// forward the event to events proxy
	if _, err := r.eventsProxy.client.ForwardEvent(r.eventsProxy.ctx, event); err != nil {
		r.eventsProxy.connectionAlive = false

		// write event to the file
		if err = r.writeToFile(event); err != nil {
			return err
		}

		// try reconnecting with the proxy
		go r.reconnect()
	}

	return nil
}

// writeToFile helper function to write to a backup events file
func (r *recorderImpl) writeToFile(event *evtsapi.Event) error {
	evt, err := json.Marshal(event)
	if err != nil {
		return err
	}

	return r.eventsFile.Write(append(evt, '\n'))
}

// createEvtsProxyRPCClient helper function to create the events proxy RPC client.
func (r *recorderImpl) createEvtsProxyRPCClient() error {
	for {
		evtsProxyClient, err := rpckit.NewRPCClient(r.getID(), r.eventsProxy.url, rpckit.WithRemoteServerName(globals.EvtsProxy))
		if err != nil {
			log.Errorf("error connecting to proxy server using URL: %v, err: %v", r.eventsProxy.url, err)
			continue
		}

		log.Infof("recorder connected to events proxy")

		r.eventsProxy.Lock()
		r.eventsProxy.rpcClient = evtsProxyClient
		r.eventsProxy.client = evtsproxygrpc.NewEventsProxyAPIClient(r.eventsProxy.rpcClient.ClientConn)
		r.eventsProxy.connectionAlive = true
		r.eventsProxy.Unlock()
		break
	}

	// start processing the failed events
	r.processFailedEvents()

	return nil
}

// getEvtsProxyRPCClient returns the events proxy RPC client associated with this recorder.
func (r *recorderImpl) getEvtsProxyRPCClient() *rpckit.RPCClient {
	return r.eventsProxy.rpcClient
}

// getID the unique ID of this recorder.
func (r *recorderImpl) getID() string {
	return r.id
}

// reconnect helper function to re-establish the connection with events proxy
func (r *recorderImpl) reconnect() {
	for {
		if err := r.eventsProxy.rpcClient.Reconnect(); err != nil {
			log.Debugf("failed to reconnect to events proxy, retrying.. err: %v", err)
			time.Sleep(1 * time.Second)
			continue
		}

		// make sure the connection is stable
		time.Sleep(10 * time.Millisecond)
		if r.eventsProxy.rpcClient.ClientConn.GetState() != connectivity.Ready {
			log.Errorf("connnection not ready after 10ms")
			continue
		}

		r.eventsProxy.Lock()
		r.eventsProxy.client = evtsproxygrpc.NewEventsProxyAPIClient(r.eventsProxy.rpcClient.ClientConn)
		r.eventsProxy.connectionAlive = true
		log.Debug("reconnected with events proxy")
		r.eventsProxy.Unlock()
		break
	}

	// start processing the failed events
	r.processFailedEvents()
}

// processFailedEvents helper function to process all the failed events
func (r *recorderImpl) processFailedEvents() {
	// make sure the previous thread (if there is) that is processing failed events
	// is stopped, otherwise there will be 2 threads trying
	// to re-send the failed events. As a result, there could be duplicates.
	select {
	case r.failedEventsForwarder.stop <- struct{}{}:
	default: // no receiver
		break
	}
	r.failedEventsForwarder.wg.Wait()

	// this will make sure that there will be only thread processing failed events at any point
	r.failedEventsForwarder.Lock()
	defer r.failedEventsForwarder.Unlock()

	if err := r.eventsFile.Rotate(); err != nil {
		log.Errorf("failed to rotate recorder events file, err: %v", err)
		return
	}

	// process the failed events from backed files
	evts, filenames, err := r.eventsFile.GetEvents()
	if err != nil {
		log.Errorf("failed to get events file, err: %v", err)
		return
	}

	// forward all the failed events to proxy
	if err := r.forwardEvents(evts); err != nil {
		return
	}

	// as the events are successfully sent, delete the backed data
	r.eventsFile.DeleteBackupFiles(filenames)
}

// forwardEvents helper function to foward given list of events to the proxy.
// it will be retried until success or stop
func (r *recorderImpl) forwardEvents(evts []*evtsapi.Event) error {
	r.failedEventsForwarder.wg.Add(1)
	defer r.failedEventsForwarder.wg.Done()

	r.eventsProxy.Lock()
	_, err := r.eventsProxy.client.ForwardEvents(r.eventsProxy.ctx, &evtsapi.EventList{Events: evts})
	r.eventsProxy.Unlock()
	if err != nil {
		log.Errorf("failed to re-send failed events, err: %v", err)
	} else {
		return nil
	}

	for {
		select {
		case <-r.failedEventsForwarder.tick.C:
			r.eventsProxy.Lock()
			_, err := r.eventsProxy.client.ForwardEvents(r.eventsProxy.ctx, &evtsapi.EventList{Events: evts})
			r.eventsProxy.Unlock()
			if err != nil {
				log.Errorf("failed to re-send failed events, err: %v", err)
				continue
			}

			return nil // events are sent successfully
		case <-r.failedEventsForwarder.stop:
			return fmt.Errorf("failed to re-send failed events")
		}
	}
}
