// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package events is a auto generated package.
Input file: svc_events.proto
*/
package events

import (
	"context"
	"errors"
	"fmt"
	"net/http"
	"net/url"
	"strings"
	"time"

	"github.com/go-kit/kit/endpoint"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/trace"
)

// Dummy definitions to suppress nonused warnings
var _ api.ObjectMeta
var _ grpc.ServerStream
var _ fmt.Formatter

// MiddlewareEventsV1Client add middleware to the client
type MiddlewareEventsV1Client func(ServiceEventsV1Client) ServiceEventsV1Client

// EndpointsEventsV1Client is the endpoints for the client
type EndpointsEventsV1Client struct {
	Client                       EventsV1Client
	AutoWatchSvcEventsV1Endpoint endpoint.Endpoint

	GetEventEndpoint  endpoint.Endpoint
	GetEventsEndpoint endpoint.Endpoint
}

// EndpointsEventsV1RestClient is the REST client
type EndpointsEventsV1RestClient struct {
	logger   log.Logger
	client   *http.Client
	instance string
	bufferId string

	AutoWatchSvcEventsV1Endpoint endpoint.Endpoint
	GetEventEndpoint             endpoint.Endpoint
	GetEventsEndpoint            endpoint.Endpoint
}

// MiddlewareEventsV1Server adds middle ware to the server
type MiddlewareEventsV1Server func(ServiceEventsV1Server) ServiceEventsV1Server

// EndpointsEventsV1Server is the server endpoints
type EndpointsEventsV1Server struct {
	svcWatchHandlerEventsV1 func(options *api.ListWatchOptions, stream grpc.ServerStream) error

	GetEventEndpoint  endpoint.Endpoint
	GetEventsEndpoint endpoint.Endpoint
}

// GetEvent is endpoint for GetEvent
func (e EndpointsEventsV1Client) GetEvent(ctx context.Context, in *GetEventRequest) (*Event, error) {
	resp, err := e.GetEventEndpoint(ctx, in)
	if err != nil {
		return &Event{}, err
	}
	return resp.(*Event), nil
}

type respEventsV1GetEvent struct {
	V   Event
	Err error
}

// GetEvents is endpoint for GetEvents
func (e EndpointsEventsV1Client) GetEvents(ctx context.Context, in *api.ListWatchOptions) (*EventList, error) {
	resp, err := e.GetEventsEndpoint(ctx, in)
	if err != nil {
		return &EventList{}, err
	}
	return resp.(*EventList), nil
}

type respEventsV1GetEvents struct {
	V   EventList
	Err error
}

func (e EndpointsEventsV1Client) AutoWatchSvcEventsV1(ctx context.Context, in *api.ListWatchOptions) (EventsV1_AutoWatchSvcEventsV1Client, error) {
	return nil, errors.New("not implemented")
}

// GetEvent implementation on server Endpoint
func (e EndpointsEventsV1Server) GetEvent(ctx context.Context, in GetEventRequest) (Event, error) {
	resp, err := e.GetEventEndpoint(ctx, in)
	if err != nil {
		return Event{}, err
	}
	return *resp.(*Event), nil
}

// MakeEventsV1GetEventEndpoint creates  GetEvent endpoints for the service
func MakeEventsV1GetEventEndpoint(s ServiceEventsV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*GetEventRequest)
		v, err := s.GetEvent(ctx, *req)
		return respEventsV1GetEvent{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("EventsV1:GetEvent")(f)
}

// GetEvents implementation on server Endpoint
func (e EndpointsEventsV1Server) GetEvents(ctx context.Context, in api.ListWatchOptions) (EventList, error) {
	resp, err := e.GetEventsEndpoint(ctx, in)
	if err != nil {
		return EventList{}, err
	}
	return *resp.(*EventList), nil
}

// MakeEventsV1GetEventsEndpoint creates  GetEvents endpoints for the service
func MakeEventsV1GetEventsEndpoint(s ServiceEventsV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*api.ListWatchOptions)
		v, err := s.GetEvents(ctx, *req)
		return respEventsV1GetEvents{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("EventsV1:GetEvents")(f)
}

// MakeAutoWatchSvcEventsV1Endpoint creates the Watch endpoint for the service
func MakeAutoWatchSvcEventsV1Endpoint(s ServiceEventsV1Server, logger log.Logger) func(options *api.ListWatchOptions, stream grpc.ServerStream) error {
	return func(options *api.ListWatchOptions, stream grpc.ServerStream) error {
		return errors.New("not implemented")
	}
}

// MakeEventsV1ServerEndpoints creates server endpoints
func MakeEventsV1ServerEndpoints(s ServiceEventsV1Server, logger log.Logger) EndpointsEventsV1Server {
	return EndpointsEventsV1Server{
		svcWatchHandlerEventsV1: MakeAutoWatchSvcEventsV1Endpoint(s, logger),

		GetEventEndpoint:  MakeEventsV1GetEventEndpoint(s, logger),
		GetEventsEndpoint: MakeEventsV1GetEventsEndpoint(s, logger),
	}
}

// LoggingEventsV1MiddlewareClient adds middleware for the client
func LoggingEventsV1MiddlewareClient(logger log.Logger) MiddlewareEventsV1Client {
	return func(next ServiceEventsV1Client) ServiceEventsV1Client {
		return loggingEventsV1MiddlewareClient{
			logger: logger,
			next:   next,
		}
	}
}

type loggingEventsV1MiddlewareClient struct {
	logger log.Logger
	next   ServiceEventsV1Client
}

// LoggingEventsV1MiddlewareServer adds middleware for the client
func LoggingEventsV1MiddlewareServer(logger log.Logger) MiddlewareEventsV1Server {
	return func(next ServiceEventsV1Server) ServiceEventsV1Server {
		return loggingEventsV1MiddlewareServer{
			logger: logger,
			next:   next,
		}
	}
}

type loggingEventsV1MiddlewareServer struct {
	logger log.Logger
	next   ServiceEventsV1Server
}

func (m loggingEventsV1MiddlewareClient) GetEvent(ctx context.Context, in *GetEventRequest) (resp *Event, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "EventsV1", "method", "GetEvent", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.GetEvent(ctx, in)
	return
}
func (m loggingEventsV1MiddlewareClient) GetEvents(ctx context.Context, in *api.ListWatchOptions) (resp *EventList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "EventsV1", "method", "GetEvents", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.GetEvents(ctx, in)
	return
}

func (m loggingEventsV1MiddlewareClient) AutoWatchSvcEventsV1(ctx context.Context, in *api.ListWatchOptions) (EventsV1_AutoWatchSvcEventsV1Client, error) {
	return nil, errors.New("not implemented")
}

func (m loggingEventsV1MiddlewareServer) GetEvent(ctx context.Context, in GetEventRequest) (resp Event, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "EventsV1", "method", "GetEvent", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetEvent(ctx, in)
	return
}
func (m loggingEventsV1MiddlewareServer) GetEvents(ctx context.Context, in api.ListWatchOptions) (resp EventList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "EventsV1", "method", "GetEvents", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetEvents(ctx, in)
	return
}

func (m loggingEventsV1MiddlewareServer) AutoWatchSvcEventsV1(in *api.ListWatchOptions, stream EventsV1_AutoWatchSvcEventsV1Server) error {
	return errors.New("Not implemented")
}

func (r *EndpointsEventsV1RestClient) getHTTPRequest(ctx context.Context, in interface{}, method, path string) (*http.Request, error) {
	target, err := url.Parse(r.instance)
	if err != nil {
		return nil, fmt.Errorf("invalid instance %s", r.instance)
	}
	target.Path = path
	req, err := http.NewRequest(method, target.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("could not create request (%s)", err)
	}
	val, ok := loginctx.AuthzHeaderFromContext(ctx)
	if ok {
		req.Header.Add("Authorization", val)
	}
	if err = encodeHTTPRequest(ctx, req, in); err != nil {
		return nil, fmt.Errorf("could not encode request (%s)", err)
	}
	return req, nil
}

func (r *EndpointsEventsV1RestClient) EventsV1GetEventEndpoint(ctx context.Context, in *GetEventRequest) (*Event, error) {
	return nil, errors.New("not allowed")
}

func (r *EndpointsEventsV1RestClient) EventsV1GetEventsEndpoint(ctx context.Context, in *api.ListWatchOptions) (*EventList, error) {
	return nil, errors.New("not allowed")
}

// MakeEventsV1RestClientEndpoints make REST client endpoints
func MakeEventsV1RestClientEndpoints(instance string) (EndpointsEventsV1RestClient, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}

	return EndpointsEventsV1RestClient{
		instance: instance,
		client:   http.DefaultClient,
	}, nil

}

// MakeEventsV1StagedRestClientEndpoints makes staged REST client endpoints
func MakeEventsV1StagedRestClientEndpoints(instance string, bufferId string) (EndpointsEventsV1RestClient, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}

	return EndpointsEventsV1RestClient{
		instance: instance,
		bufferId: bufferId,
		client:   http.DefaultClient,
	}, nil
}
