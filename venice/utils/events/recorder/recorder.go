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
	id                    string                 // id (unique key) of the recorder
	eventSource           *evtsapi.EventSource   // all the events generated using this recorder will carry this source
	eventTypes            map[string]struct{}    // eventTypes provide a function to validate the given event type
	eventsProxy           *eventsProxy           // event proxy
	eventsFile            *fileImpl              // events backup store
	failedEventsForwarder *failedEventsForwarder // used to forward failed events to the proxy
	started               bool                   // whether client has started
	logger                log.Logger             // logger
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

// Config represents the recorder configuration
type Config struct {
	Source        *evtsapi.EventSource // event source for all the events
	EvtTypes      []string             // list of event types supported by the recorder
	EvtsProxyURL  string               // proxy URL to connect to
	BackupDir     string               // store events in a file, if proxy connection becomes unavailable
	SkipEvtsProxy bool                 // use local store for events; skip connecting to proxy
}

// NewRecorder creates and returns a recorder instance and instantiates the singleton object.
// if `evtsproxyURL` is empty, it will it use local events proxy RPC port.
// First recorder instance created in the process is same as the singleton recorder.
func NewRecorder(config *Config, logger log.Logger) (events.Recorder, error) {
	if config.Source == nil {
		return nil, fmt.Errorf("missing event source")
	}

	if len(config.EvtTypes) == 0 {
		return nil, fmt.Errorf("empty event types")
	}

	if utils.IsEmpty(config.EvtsProxyURL) {
		config.EvtsProxyURL = fmt.Sprintf(":%s", globals.EvtsProxyRPCPort)
	}

	if utils.IsEmpty(config.BackupDir) {
		config.BackupDir = filepath.Join(globals.EventsDir, "recorder")
	}

	eventsFile, err := newFile(config.BackupDir, events.GetSourceKey(config.Source))
	if err != nil {
		return nil, err
	}

	recorder := &recorderImpl{
		id:          fmt.Sprintf("recorder-{%s:%s}", config.Source.GetNodeName(), config.Source.GetComponent()),
		eventSource: config.Source,
		eventTypes:  constructEventTypesMap(config.EvtTypes),
		eventsProxy: &eventsProxy{
			url: config.EvtsProxyURL,
			ctx: context.Background(), // context for all the proxy calls
		},
		eventsFile: eventsFile,
		failedEventsForwarder: &failedEventsForwarder{
			tick: time.NewTicker(2 * time.Second),
			stop: make(chan struct{}),
		},
		logger: logger,
	}

	if config.SkipEvtsProxy {
		logger.Debugf("{%s} skipping events proxy", recorder.id)
		recorder.eventsProxy.connectionAlive = false // all the writes will be sent to file
	} else {
		recorder.started = true
		go recorder.createEvtsProxyRPCClient() // create events proxy client
	}

	once.Do(func() {
		singletonRecorder = recorder
	})

	return recorder, nil
}

func (r *recorderImpl) StartExport() {
	if !r.started {
		r.started = true
		go r.createEvtsProxyRPCClient() // create events proxy client
	}
}

// Event records the event by creating a event using the given type, severity, message, etc.
// and sending it to the events proxy for further processing.
// Event sources will call this to record an event.
func (r *recorderImpl) Event(eventType string, severity evtsapi.SeverityLevel, message string, objRef interface{}) {
	if err := r.validate(eventType, severity); err != nil {
		r.logger.Fatalf("{%s} validation failed [%s, %s], err: %v", r.id, eventType, severity, err)
	}

	var objRefMeta *api.ObjectMeta
	var objRefKind string
	var err error

	// derive reference object details from the given object
	if objRef != nil {
		objRefMeta, err = runtime.GetObjectMeta(objRef)
		if err != nil {
			r.logger.Fatalf("{%s} failed to get the object meta from reference object, err: %v", r.id, err)
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
		SelfLink:  fmt.Sprintf("/events/v1/events/%s", uuid),
		Labels:    map[string]string{"_category": globals.Kind2Category["Event"]},
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

	if objRefMeta != nil {
		// update tenant and namespace for tenant scoped object events; all the other events go to default tenant.
		tenantScoped, err := runtime.GetDefaultScheme().IsTenantScoped(objRefKind)
		if err != nil {
			r.logger.Errorf("{%s} failed to get scope for kind: %s, err: %v", r.id, objRefKind, err)
		}

		if tenantScoped {
			event.ObjectMeta.Tenant = objRefMeta.GetTenant()
			event.ObjectMeta.Namespace = objRefMeta.GetNamespace()
		}

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
		r.logger.Fatalf("{%s} failed to record event %v, err: %v", r.id, event.GetUUID(), err)
	}
}

// validate validates the given event type and severity
func (r *recorderImpl) validate(eType string, severity evtsapi.SeverityLevel) error {
	// validate event
	_, found := r.eventTypes[eType]
	if !found {
		return events.NewError(events.ErrInvalidEventType, eType)
	}

	// validate severity
	if _, ok := evtsapi.SeverityLevel_name[int32(severity)]; !ok {
		return events.NewError(events.ErrInvalidSeverity, fmt.Sprintf("%s", severity))
	}

	return nil
}

// sendEvent helper function to send the event to proxy.
func (r *recorderImpl) sendEvent(event *evtsapi.Event) error {
	r.eventsProxy.Lock()
	defer r.eventsProxy.Unlock()

	if !r.eventsProxy.connectionAlive {
		r.logger.Debugf("{%s} connection to evtsproxy unavailable. so, writing event {%s} to a file", r.id, events.MinifyEvent(event))
		return r.writeToFile(event)
	}

	// cancel proxy call in 100ms
	ctx, cancel := context.WithDeadline(r.eventsProxy.ctx, time.Now().Add(100*time.Millisecond))
	defer cancel()

	// forward the event to events proxy
	// NOTE: it is possible that the event was written to elastic and context got cancelled before call returned.
	// in such cases, we'll re-send the same event again and that will result in a duplicate.
	if _, err := r.eventsProxy.client.ForwardEvent(ctx, event); err != nil {
		r.logger.Errorf("{%s} failed to forward the event {%+v} to evtsproxy, err: %v", r.id, event, err)
		r.eventsProxy.connectionAlive = false

		// write event to the file
		if wErr := r.writeToFile(event); wErr != nil {
			return wErr
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
		evtsProxyClient, err := rpckit.NewRPCClient(r.getID(), r.eventsProxy.url, rpckit.WithTLSProvider(nil))
		if err != nil {
			r.logger.Errorf("{%s} error connecting to proxy server using URL: %v, err: %v", r.id, r.eventsProxy.url, err)
			time.Sleep(2 * time.Second)
			continue
		}

		r.logger.Infof("{%s} events recorder connected to events proxy", r.id)

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

// getID the unique ID of this recorder.
func (r *recorderImpl) getID() string {
	return r.id
}

// reconnect helper function to re-establish the connection with events proxy
func (r *recorderImpl) reconnect() {
	for {
		if err := r.eventsProxy.rpcClient.Reconnect(); err != nil {
			r.logger.Debugf("{%s} failed to reconnect to events proxy, retrying.. err: %v", r.id, err)
			time.Sleep(1 * time.Second)
			continue
		}

		// make sure the connection is stable
		time.Sleep(10 * time.Millisecond)
		if r.eventsProxy.rpcClient.ClientConn.GetState() != connectivity.Ready {
			r.logger.Errorf("{%s} connection not ready after 10ms", r.id)
			continue
		}

		r.eventsProxy.Lock()
		r.eventsProxy.client = evtsproxygrpc.NewEventsProxyAPIClient(r.eventsProxy.rpcClient.ClientConn)
		r.eventsProxy.connectionAlive = true
		r.logger.Debugf("{%s} reconnected with events proxy", r.id)
		r.eventsProxy.Unlock()
		break
	}

	// start processing the failed events
	r.processFailedEvents()
}

// processFailedEvents helper function to process all the failed events
func (r *recorderImpl) processFailedEvents() {
	r.logger.Debugf("{%s} processing failed events (reading from the file)", r.id)
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
		r.logger.Errorf("{%s} failed to rotate recorder events file, err: %v", r.id, err)
		return
	}

	// process the failed events from backed files
	evts, filenames, err := r.eventsFile.GetEvents()
	if err != nil {
		r.logger.Errorf("{%s} failed to get events file, err: %v", r.id, err)
		return
	}

	r.logger.Debugf("{%s} forwarding failed events (from the file) to evtsproxy: %v", r.id, events.Minify(evts))
	// forward all the failed events to proxy
	if err := r.forwardEvents(evts); err != nil {
		return
	}

	// as the events are successfully sent, delete the backed data
	r.eventsFile.DeleteBackupFiles(filenames)
}

// forwardEvents helper function to forward given list of events to the proxy.
// it will be retried until success or stop
func (r *recorderImpl) forwardEvents(evts []*evtsapi.Event) error {
	r.failedEventsForwarder.wg.Add(1)
	defer r.failedEventsForwarder.wg.Done()

	r.eventsProxy.Lock()
	_, err := r.eventsProxy.client.ForwardEvents(r.eventsProxy.ctx, &evtsapi.EventList{Items: evts})
	r.eventsProxy.Unlock()
	if err != nil {
		r.logger.Errorf("{%s} failed to re-send failed events, err: %v", r.id, err)
	} else {
		return nil
	}

	for {
		select {
		case <-r.failedEventsForwarder.tick.C:
			r.eventsProxy.Lock()
			_, err := r.eventsProxy.client.ForwardEvents(r.eventsProxy.ctx, &evtsapi.EventList{Items: evts})
			r.eventsProxy.Unlock()
			if err != nil {
				r.logger.Errorf("{%s} failed to re-send failed events, err: %v", r.id, err)
				continue
			}

			return nil // events are sent successfully
		case <-r.failedEventsForwarder.stop:
			return fmt.Errorf("failed to re-send failed events")
		}
	}
}
