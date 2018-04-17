// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package recorder

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	evtsproxygrpc "github.com/pensando/sw/venice/evtsproxy/rpcserver/evtsproxyproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// TODO:
// 1. store failed requests in a file and replay.
// 2. add first timestamp to the event and update last timestamp in the dispatcher.
// 3. ObjectRef is missing tenant.

// recorderImpl implements `Recorder` interface. Events sources at venice will
// use this recorder to generate events with the given severity, type,
// messsage, etc. Events are sent to the proxy for further processing (dedup, cache, etc.)
type recorderImpl struct {
	sync.Mutex                       // to protect access to the recorder
	id          string               // id (unique key) of the recorder
	eventSource *evtsapi.EventSource // all the events generated using this recorder will carry this source
	eventTypes  map[string]struct{}  // eventTypes provide a function to validate the given event type
	eventsProxy *eventsProxy         // event proxy
}

// eventsProxy encapsulates all the proxy details including connection string
// and context to be used while making calls.
type eventsProxy struct {
	ctx       context.Context                    // ctx for making calls to events proxy
	url       string                             // events proxy URL
	rpcClient *rpckit.RPCClient                  // RPC client for events proxy
	client    evtsproxygrpc.EventsProxyAPIClient // events proxy client connection
}

// NewRecorder creates and returns a recorder instance.
// if `evtsproxyURL` is empty, it will it use local events proxy RPC port.
func NewRecorder(source *evtsapi.EventSource, eventTypes []string, evtsproxyURL string) (events.Recorder, error) {
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

	recorder := &recorderImpl{
		id:          fmt.Sprintf("%s-%s", source.GetNodeName(), source.GetComponent()),
		eventSource: source,
		eventTypes:  constructEventTypesMap(eventTypes),
		eventsProxy: &eventsProxy{
			url: evtsproxyURL,
			ctx: context.Background(), // context for all the proxy calls
		},
	}

	// create events proxy client
	if err := recorder.createEvtsProxyRPCClient(); err != nil {
		return nil, err
	}

	return recorder, nil
}

// Event records the event by creating a event using the given type, severity, message, etc.
// and sending it to the events proxy for further processing.
// Event sources will call this to record an event.
func (r *recorderImpl) Event(eventType, severity, message string, objRef *api.ObjectRef) error {
	if err := r.validate(eventType, severity); err != nil {
		return err
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
	}

	if objRef != nil {
		meta.Namespace = objRef.GetNamespace()
	}

	// FIXME: add default tenant and namespace

	// create event object
	event := &evtsapi.Event{
		TypeMeta:   api.TypeMeta{Kind: "Event"},
		ObjectMeta: meta,
		EventAttributes: evtsapi.EventAttributes{
			Type:      eventType,
			Severity:  severity,
			Message:   message,
			ObjectRef: objRef,
			Source:    r.eventSource,
			Count:     1,
		},
	}

	// send the event to proxy
	if err := r.sendEvent(event); err != nil {
		log.Errorf("failed to send event %+v to the proxy, err: %v", event, err)
		return err
	}

	return nil
}

// validate validates the given event type and severity
func (r *recorderImpl) validate(eType, severity string) error {
	// validate event
	_, found := r.eventTypes[eType]
	if !found {
		return events.NewError(events.ErrInvalidEventType, "")
	}

	// validate severity
	if _, ok := evtsapi.SeverityLevel_value[severity]; !ok {
		return events.NewError(events.ErrInvalidSeverity, "")
	}

	return nil
}

// sendEvent helper function to send the event to proxy.
func (r *recorderImpl) sendEvent(event *evtsapi.Event) error {
	r.Lock()
	defer r.Unlock()

	// forward the event to events proxy
	_, err := r.eventsProxy.client.ForwardEvent(r.eventsProxy.ctx, event)
	return err
}

// createEvtsProxyRPCClient helper function to create the events proxy RPC client.
func (r *recorderImpl) createEvtsProxyRPCClient() error {
	evtsProxyClient, err := rpckit.NewRPCClient(r.getID(), r.eventsProxy.url, nil)
	if err != nil {
		return fmt.Errorf("error connecting to proxy server using URL: %v, err: %v",
			r.eventsProxy.url, err)
	}

	r.eventsProxy.rpcClient = evtsProxyClient
	r.eventsProxy.client = evtsproxygrpc.NewEventsProxyAPIClient(r.eventsProxy.rpcClient.ClientConn)
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
