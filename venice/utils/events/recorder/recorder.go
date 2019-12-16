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
	"github.com/satori/go.uuid"
	"google.golang.org/grpc/connectivity"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/events/generated/eventtypes"
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

var singletonRecorder events.Recorder
var m sync.RWMutex
var once sync.Once

// Override overrides the singleton recorder with the given recorder.
// NOTE: this should be used only from the tests.
func Override(recorder events.Recorder) error {
	m.Lock()
	defer m.Unlock()

	if singletonRecorder != nil {
		singletonRecorder.Close()
	}

	singletonRecorder = recorder
	return nil
}

// Event records the given event
func Event(eventType eventtypes.EventType, message string, objRef interface{}) {
	m.RLock()
	defer m.RUnlock()

	if singletonRecorder == nil {
		log.Fatal("initialize events recorder")
	}

	singletonRecorder.Event(eventType, message, objRef)
}

// NOTE: the recorder does not ensure any ordering in the delivery of events during any failure (proxy restart, recorder restart)

// recorderImpl implements `Recorder` interface. Events sources at venice will
// use this recorder to generate events with the given severity, type,
// messsage, etc. Events are sent to the proxy for further processing (dedup, cache, etc.)
type recorderImpl struct {
	id                    string                 // id (unique key) of the recorder
	component             string                 // all the events generated using this recorder will carry this component in the source
	skipEvtsProxy         bool                   // use local store for events; skip sending events to evtsproxy
	eventsProxy           *eventsProxy           // event proxy
	eventsFile            *fileImpl              // events backup store
	failedEventsForwarder *failedEventsForwarder // used to forward failed events to the proxy
	logger                log.Logger             // logger
	close                 sync.Once              // to close the recorder
	shutdown              chan struct{}          // to send shutdown signal to the daemon go routines (i.e. event distribution)
	wg                    sync.WaitGroup         // used to wait for the graceful shutdown of daemon go routines
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
	started         bool                               // whether client has started
}

// failedEventsForwarder helps to forward failed events to the proxy
type failedEventsForwarder struct {
	sync.Mutex
	tick *time.Ticker // failed call to the proxy will be retried as per this ticker
}

// Config represents the recorder configuration
type Config struct {
	Component     string // name of the component recording events
	EvtsProxyURL  string // proxy URL to connect to
	BackupDir     string // store events in a file, if proxy connection becomes unavailable
	SkipEvtsProxy bool   // use local store for events; skip connecting to proxy
}

// NewRecorder creates and returns a recorder instance and instantiates the singleton object.
// if `evtsproxyURL` is empty, it will it use local events proxy RPC port.
// First recorder instance created in the process is same as the singleton recorder.
func NewRecorder(config *Config, logger log.Logger) (events.Recorder, error) {
	if utils.IsEmpty(config.Component) {
		return nil, fmt.Errorf("missing component name")
	}

	if utils.IsEmpty(config.EvtsProxyURL) {
		config.EvtsProxyURL = fmt.Sprintf(":%s", globals.EvtsProxyRPCPort)
	}

	if utils.IsEmpty(config.BackupDir) {
		config.BackupDir = filepath.Join(globals.EventsDir, "recorder")
	}

	eventsFile, err := newFile(config.BackupDir, config.Component)
	if err != nil {
		return nil, err
	}

	recorder := &recorderImpl{
		id:        fmt.Sprintf("recorder-%s", config.Component),
		component: config.Component,
		eventsProxy: &eventsProxy{
			url: config.EvtsProxyURL,
			ctx: context.Background(), // context for all the proxy calls
		},
		eventsFile: eventsFile,
		failedEventsForwarder: &failedEventsForwarder{
			tick: time.NewTicker(2 * time.Second),
		},
		skipEvtsProxy: config.SkipEvtsProxy,
		logger:        logger,
		shutdown:      make(chan struct{}),
	}

	if config.SkipEvtsProxy {
		logger.Debugf("{%s} skipping events proxy", recorder.id)
	} else {
		recorder.wg.Add(1)
		go recorder.createEvtsProxyRPCClient() // create events proxy client
		recorder.eventsProxy.started = true
	}

	once.Do(func() {
		m.Lock()
		singletonRecorder = recorder
		m.Unlock()
	})

	return recorder, nil
}

// StartExport starts exporting events to evtsproxy
func (r *recorderImpl) StartExport() {
	if r.skipEvtsProxy {
		r.skipEvtsProxy = false
	}

	if !r.eventsProxy.started {
		r.wg.Add(1)
		go r.createEvtsProxyRPCClient() // create events proxy client
		r.eventsProxy.started = true
	}
}

// Close closes the recorder
func (r *recorderImpl) Close() {
	r.close.Do(func() {
		r.eventsFile.Close()
		close(r.shutdown)
		r.wg.Wait()
	})
}

// Event records the event by creating a event using the given type, severity, message, etc.
// and sending it to the events proxy for further processing.
// Event sources will call this to record an event.
func (r *recorderImpl) Event(eventType eventtypes.EventType, message string, objRef interface{}) {
	var objRefMeta *api.ObjectMeta
	var objRefKind string
	var err error

	if objRef != nil {
		// derive reference object details from the given object
		objRefMeta, err = runtime.GetObjectMeta(objRef)
		if err != nil {
			r.logger.Fatalf("{%s} failed to get the object meta from reference object, err: %v", r.id, err)
		}

		objRefKind = objRef.(runtime.Object).GetObjectKind()
	}

	// get event attrs
	eTypeAttrs := eventtypes.GetEventTypeAttrs(eventType)
	if eTypeAttrs == nil {
		r.logger.Fatalf("{%s} could not get the event attributes from given event type: %v", r.id, eventType)
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
		Labels:    map[string]string{"_category": globals.Kind2Category("Event")},
	}

	// create event object
	event := &evtsapi.Event{
		TypeMeta:   api.TypeMeta{Kind: "Event"},
		ObjectMeta: meta,
		EventAttributes: evtsapi.EventAttributes{
			Type:     eTypeAttrs.EType,
			Severity: eTypeAttrs.Severity,
			Message:  message,
			Category: eTypeAttrs.Category,
			Source:   &evtsapi.EventSource{Component: r.component},
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
			URI:       objRefMeta.GetSelfLink(),
			Name:      objRefMeta.GetName(),
		}

		// update the user friendly name of nic in object-ref
		if r.isNicKind(objRefKind) {
			if nic, ok := objRef.(*cluster.DistributedServiceCard); ok {
				event.ObjectRef.Name = nic.Spec.ID
			}
		}
	}

	// send the event to a file or proxy
	if err := r.sendEvent(event); err != nil {
		r.logger.Fatalf("{%s} failed to record event %v, err: %v", r.id, event.GetUUID(), err)
	}
}

// sendEvent helper function to send the event to proxy.
func (r *recorderImpl) sendEvent(event *evtsapi.Event) error {
	r.eventsProxy.Lock()
	defer r.eventsProxy.Unlock()

	if r.skipEvtsProxy || !r.eventsProxy.connectionAlive {
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

		r.logger.Debugf("{%s} connection to evtsproxy unavailable. will start writing events to a file", r.id)
		// write event to the file
		if wErr := r.writeToFile(event); wErr != nil {
			return wErr
		}

		// try reconnecting with the proxy
		r.wg.Add(1)
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
func (r *recorderImpl) createEvtsProxyRPCClient() {
	defer r.wg.Done()
	for {
		select {
		case <-r.shutdown:
			r.logger.Error("received shutdown signal, closing {createEvtsProxyRPCClient}")
			return
		case <-time.After(2 * time.Second):
			evtsProxyClient, err := rpckit.NewRPCClient(r.getID(), r.eventsProxy.url, rpckit.WithTLSProvider(nil))
			if err != nil {
				r.logger.Errorf("{%s} error connecting to proxy server using URL: %v, err: %v", r.id, r.eventsProxy.url, err)
				continue
			}

			r.logger.Infof("{%s} events recorder connected to events proxy", r.id)

			r.eventsProxy.Lock()
			r.eventsProxy.rpcClient = evtsProxyClient
			r.eventsProxy.client = evtsproxygrpc.NewEventsProxyAPIClient(r.eventsProxy.rpcClient.ClientConn)
			r.eventsProxy.connectionAlive = true
			r.eventsProxy.Unlock()

			// start processing the failed events
			r.processFailedEvents()
			return
		}
	}
}

// getID the unique ID of this recorder.
func (r *recorderImpl) getID() string {
	return r.id
}

// reconnect helper function to re-establish the connection with events proxy
func (r *recorderImpl) reconnect() {
	defer r.wg.Done()
	for {
		select {
		case <-r.shutdown:
			r.logger.Error("received shutdown signal, closing {reconnect}")
			return
		case <-time.After(1 * time.Second):
			if err := r.eventsProxy.rpcClient.Reconnect(); err != nil {
				r.logger.Debugf("{%s} failed to reconnect to events proxy, retrying.. err: %v", r.id, err)
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

			// start processing the failed events
			r.processFailedEvents()
			return
		}
	}
}

// processFailedEvents helper function to process all the failed events
func (r *recorderImpl) processFailedEvents() {
	// this will make sure that there will be only thread processing failed events at any point
	r.failedEventsForwarder.Lock()
	defer r.failedEventsForwarder.Unlock()

	r.logger.Debugf("{%s} processing failed events (reading from the file)", r.id)

	// rotate moves the file *.bak/ and does the further processing (read and send to evtsproxy) on it.
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

	// forward all the failed events to proxy
	r.logger.Debugf("{%s} forwarding failed events (from the file) to evtsproxy: %v", r.id, events.Minify(evts))
	if err := r.forwardEvents(evts); err != nil {
		r.logger.Errorf("failed to re-send events, err: %v", err)
		return
	}

	// as the events are successfully sent, delete the backed data
	r.eventsFile.DeleteBackupFiles(filenames)
}

// forwardEvents helper function to forward given list of events to the proxy.
// it will be retried until success or stop
func (r *recorderImpl) forwardEvents(evts []*evtsapi.Event) error {
	r.eventsProxy.Lock()
	_, err := r.eventsProxy.client.ForwardEvents(r.eventsProxy.ctx, &evtsapi.EventList{Items: evts})
	r.eventsProxy.Unlock()
	if err == nil {
		return nil
	}

	r.logger.Errorf("{%s} failed to re-send failed events, err: %v, retrying...", r.id, err)

	for {
		select {
		case <-r.shutdown:
			return fmt.Errorf("received shutdown signal, closing {forwardEvents}")
		case <-time.After(60 * time.Second):
			return fmt.Errorf("timed out after 60s")
		case <-r.failedEventsForwarder.tick.C:
			r.eventsProxy.Lock()
			_, err := r.eventsProxy.client.ForwardEvents(r.eventsProxy.ctx, &evtsapi.EventList{Items: evts})
			r.eventsProxy.Unlock()
			if err != nil {
				r.logger.Errorf("{%s} failed to re-send failed events, err: %v, retrying...", r.id, err)
				continue
			}

			return nil // events are sent successfully
		}
	}
}

// isNicKind returns true if the given kind matches the smart nic kind
func (r *recorderImpl) isNicKind(kind string) bool {
	nic := &cluster.DistributedServiceCard{}
	nic.Defaults("all")
	return nic.GetKind() == kind
}
