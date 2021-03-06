// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package routing is a auto generated package.
Input file: svc_routing.proto
*/
package routing

import (
	"context"
	"crypto/tls"
	"errors"
	"fmt"
	"net/http"
	"net/url"
	"strings"
	"time"

	"github.com/go-kit/kit/endpoint"
	"github.com/gorilla/websocket"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/listerwatcher"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/trace"
)

// Dummy definitions to suppress nonused warnings
var _ api.ObjectMeta
var _ grpc.ServerStream
var _ fmt.Formatter
var _ *listerwatcher.WatcherClient

// MiddlewareRoutingV1Client add middleware to the client
type MiddlewareRoutingV1Client func(ServiceRoutingV1Client) ServiceRoutingV1Client

// EndpointsRoutingV1Client is the endpoints for the client
type EndpointsRoutingV1Client struct {
	Client                        RoutingV1Client
	AutoWatchSvcRoutingV1Endpoint endpoint.Endpoint

	AutoAddNeighborEndpoint    endpoint.Endpoint
	AutoDeleteNeighborEndpoint endpoint.Endpoint
	AutoGetNeighborEndpoint    endpoint.Endpoint
	AutoLabelNeighborEndpoint  endpoint.Endpoint
	AutoListNeighborEndpoint   endpoint.Endpoint
	AutoUpdateNeighborEndpoint endpoint.Endpoint
}

// EndpointsRoutingV1RestClient is the REST client
type EndpointsRoutingV1RestClient struct {
	logger   log.Logger
	client   *http.Client
	instance string
	bufferId string

	AutoAddNeighborEndpoint       endpoint.Endpoint
	AutoDeleteNeighborEndpoint    endpoint.Endpoint
	AutoGetNeighborEndpoint       endpoint.Endpoint
	AutoLabelNeighborEndpoint     endpoint.Endpoint
	AutoListNeighborEndpoint      endpoint.Endpoint
	AutoUpdateNeighborEndpoint    endpoint.Endpoint
	AutoWatchNeighborEndpoint     endpoint.Endpoint
	AutoWatchSvcRoutingV1Endpoint endpoint.Endpoint
}

// MiddlewareRoutingV1Server adds middle ware to the server
type MiddlewareRoutingV1Server func(ServiceRoutingV1Server) ServiceRoutingV1Server

// EndpointsRoutingV1Server is the server endpoints
type EndpointsRoutingV1Server struct {
	svcWatchHandlerRoutingV1 func(options *api.AggWatchOptions, stream grpc.ServerStream) error

	AutoAddNeighborEndpoint    endpoint.Endpoint
	AutoDeleteNeighborEndpoint endpoint.Endpoint
	AutoGetNeighborEndpoint    endpoint.Endpoint
	AutoLabelNeighborEndpoint  endpoint.Endpoint
	AutoListNeighborEndpoint   endpoint.Endpoint
	AutoUpdateNeighborEndpoint endpoint.Endpoint

	watchHandlerNeighbor func(options *api.ListWatchOptions, stream grpc.ServerStream) error
}

// AutoAddNeighbor is endpoint for AutoAddNeighbor
func (e EndpointsRoutingV1Client) AutoAddNeighbor(ctx context.Context, in *Neighbor) (*Neighbor, error) {
	resp, err := e.AutoAddNeighborEndpoint(ctx, in)
	if err != nil {
		return &Neighbor{}, err
	}
	return resp.(*Neighbor), nil
}

type respRoutingV1AutoAddNeighbor struct {
	V   Neighbor
	Err error
}

// AutoDeleteNeighbor is endpoint for AutoDeleteNeighbor
func (e EndpointsRoutingV1Client) AutoDeleteNeighbor(ctx context.Context, in *Neighbor) (*Neighbor, error) {
	resp, err := e.AutoDeleteNeighborEndpoint(ctx, in)
	if err != nil {
		return &Neighbor{}, err
	}
	return resp.(*Neighbor), nil
}

type respRoutingV1AutoDeleteNeighbor struct {
	V   Neighbor
	Err error
}

// AutoGetNeighbor is endpoint for AutoGetNeighbor
func (e EndpointsRoutingV1Client) AutoGetNeighbor(ctx context.Context, in *Neighbor) (*Neighbor, error) {
	resp, err := e.AutoGetNeighborEndpoint(ctx, in)
	if err != nil {
		return &Neighbor{}, err
	}
	return resp.(*Neighbor), nil
}

type respRoutingV1AutoGetNeighbor struct {
	V   Neighbor
	Err error
}

// AutoLabelNeighbor is endpoint for AutoLabelNeighbor
func (e EndpointsRoutingV1Client) AutoLabelNeighbor(ctx context.Context, in *api.Label) (*Neighbor, error) {
	resp, err := e.AutoLabelNeighborEndpoint(ctx, in)
	if err != nil {
		return &Neighbor{}, err
	}
	return resp.(*Neighbor), nil
}

type respRoutingV1AutoLabelNeighbor struct {
	V   Neighbor
	Err error
}

// AutoListNeighbor is endpoint for AutoListNeighbor
func (e EndpointsRoutingV1Client) AutoListNeighbor(ctx context.Context, in *api.ListWatchOptions) (*NeighborList, error) {
	resp, err := e.AutoListNeighborEndpoint(ctx, in)
	if err != nil {
		return &NeighborList{}, err
	}
	return resp.(*NeighborList), nil
}

type respRoutingV1AutoListNeighbor struct {
	V   NeighborList
	Err error
}

// AutoUpdateNeighbor is endpoint for AutoUpdateNeighbor
func (e EndpointsRoutingV1Client) AutoUpdateNeighbor(ctx context.Context, in *Neighbor) (*Neighbor, error) {
	resp, err := e.AutoUpdateNeighborEndpoint(ctx, in)
	if err != nil {
		return &Neighbor{}, err
	}
	return resp.(*Neighbor), nil
}

type respRoutingV1AutoUpdateNeighbor struct {
	V   Neighbor
	Err error
}

func (e EndpointsRoutingV1Client) AutoWatchSvcRoutingV1(ctx context.Context, in *api.AggWatchOptions) (RoutingV1_AutoWatchSvcRoutingV1Client, error) {
	return e.Client.AutoWatchSvcRoutingV1(ctx, in)
}

// AutoWatchNeighbor performs Watch for Neighbor
func (e EndpointsRoutingV1Client) AutoWatchNeighbor(ctx context.Context, in *api.ListWatchOptions) (RoutingV1_AutoWatchNeighborClient, error) {
	return e.Client.AutoWatchNeighbor(ctx, in)
}

// AutoAddNeighbor implementation on server Endpoint
func (e EndpointsRoutingV1Server) AutoAddNeighbor(ctx context.Context, in Neighbor) (Neighbor, error) {
	resp, err := e.AutoAddNeighborEndpoint(ctx, in)
	if err != nil {
		return Neighbor{}, err
	}
	return *resp.(*Neighbor), nil
}

// MakeRoutingV1AutoAddNeighborEndpoint creates  AutoAddNeighbor endpoints for the service
func MakeRoutingV1AutoAddNeighborEndpoint(s ServiceRoutingV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Neighbor)
		v, err := s.AutoAddNeighbor(ctx, *req)
		return respRoutingV1AutoAddNeighbor{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("RoutingV1:AutoAddNeighbor")(f)
}

// AutoDeleteNeighbor implementation on server Endpoint
func (e EndpointsRoutingV1Server) AutoDeleteNeighbor(ctx context.Context, in Neighbor) (Neighbor, error) {
	resp, err := e.AutoDeleteNeighborEndpoint(ctx, in)
	if err != nil {
		return Neighbor{}, err
	}
	return *resp.(*Neighbor), nil
}

// MakeRoutingV1AutoDeleteNeighborEndpoint creates  AutoDeleteNeighbor endpoints for the service
func MakeRoutingV1AutoDeleteNeighborEndpoint(s ServiceRoutingV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Neighbor)
		v, err := s.AutoDeleteNeighbor(ctx, *req)
		return respRoutingV1AutoDeleteNeighbor{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("RoutingV1:AutoDeleteNeighbor")(f)
}

// AutoGetNeighbor implementation on server Endpoint
func (e EndpointsRoutingV1Server) AutoGetNeighbor(ctx context.Context, in Neighbor) (Neighbor, error) {
	resp, err := e.AutoGetNeighborEndpoint(ctx, in)
	if err != nil {
		return Neighbor{}, err
	}
	return *resp.(*Neighbor), nil
}

// MakeRoutingV1AutoGetNeighborEndpoint creates  AutoGetNeighbor endpoints for the service
func MakeRoutingV1AutoGetNeighborEndpoint(s ServiceRoutingV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Neighbor)
		v, err := s.AutoGetNeighbor(ctx, *req)
		return respRoutingV1AutoGetNeighbor{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("RoutingV1:AutoGetNeighbor")(f)
}

// AutoLabelNeighbor implementation on server Endpoint
func (e EndpointsRoutingV1Server) AutoLabelNeighbor(ctx context.Context, in api.Label) (Neighbor, error) {
	resp, err := e.AutoLabelNeighborEndpoint(ctx, in)
	if err != nil {
		return Neighbor{}, err
	}
	return *resp.(*Neighbor), nil
}

// MakeRoutingV1AutoLabelNeighborEndpoint creates  AutoLabelNeighbor endpoints for the service
func MakeRoutingV1AutoLabelNeighborEndpoint(s ServiceRoutingV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*api.Label)
		v, err := s.AutoLabelNeighbor(ctx, *req)
		return respRoutingV1AutoLabelNeighbor{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("RoutingV1:AutoLabelNeighbor")(f)
}

// AutoListNeighbor implementation on server Endpoint
func (e EndpointsRoutingV1Server) AutoListNeighbor(ctx context.Context, in api.ListWatchOptions) (NeighborList, error) {
	resp, err := e.AutoListNeighborEndpoint(ctx, in)
	if err != nil {
		return NeighborList{}, err
	}
	return *resp.(*NeighborList), nil
}

// MakeRoutingV1AutoListNeighborEndpoint creates  AutoListNeighbor endpoints for the service
func MakeRoutingV1AutoListNeighborEndpoint(s ServiceRoutingV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*api.ListWatchOptions)
		v, err := s.AutoListNeighbor(ctx, *req)
		return respRoutingV1AutoListNeighbor{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("RoutingV1:AutoListNeighbor")(f)
}

// AutoUpdateNeighbor implementation on server Endpoint
func (e EndpointsRoutingV1Server) AutoUpdateNeighbor(ctx context.Context, in Neighbor) (Neighbor, error) {
	resp, err := e.AutoUpdateNeighborEndpoint(ctx, in)
	if err != nil {
		return Neighbor{}, err
	}
	return *resp.(*Neighbor), nil
}

// MakeRoutingV1AutoUpdateNeighborEndpoint creates  AutoUpdateNeighbor endpoints for the service
func MakeRoutingV1AutoUpdateNeighborEndpoint(s ServiceRoutingV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Neighbor)
		v, err := s.AutoUpdateNeighbor(ctx, *req)
		return respRoutingV1AutoUpdateNeighbor{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("RoutingV1:AutoUpdateNeighbor")(f)
}

func (e EndpointsRoutingV1Server) AutoWatchSvcRoutingV1(in *api.AggWatchOptions, stream RoutingV1_AutoWatchSvcRoutingV1Server) error {
	return e.svcWatchHandlerRoutingV1(in, stream)
}

// MakeAutoWatchSvcRoutingV1Endpoint creates the Watch endpoint for the service
func MakeAutoWatchSvcRoutingV1Endpoint(s ServiceRoutingV1Server, logger log.Logger) func(options *api.AggWatchOptions, stream grpc.ServerStream) error {
	return func(options *api.AggWatchOptions, stream grpc.ServerStream) error {
		wstream := stream.(RoutingV1_AutoWatchSvcRoutingV1Server)
		return s.AutoWatchSvcRoutingV1(options, wstream)
	}
}

// AutoWatchNeighbor is the watch handler for Neighbor on the server side.
func (e EndpointsRoutingV1Server) AutoWatchNeighbor(in *api.ListWatchOptions, stream RoutingV1_AutoWatchNeighborServer) error {
	return e.watchHandlerNeighbor(in, stream)
}

// MakeAutoWatchNeighborEndpoint creates the Watch endpoint
func MakeAutoWatchNeighborEndpoint(s ServiceRoutingV1Server, logger log.Logger) func(options *api.ListWatchOptions, stream grpc.ServerStream) error {
	return func(options *api.ListWatchOptions, stream grpc.ServerStream) error {
		wstream := stream.(RoutingV1_AutoWatchNeighborServer)
		return s.AutoWatchNeighbor(options, wstream)
	}
}

// MakeRoutingV1ServerEndpoints creates server endpoints
func MakeRoutingV1ServerEndpoints(s ServiceRoutingV1Server, logger log.Logger) EndpointsRoutingV1Server {
	return EndpointsRoutingV1Server{
		svcWatchHandlerRoutingV1: MakeAutoWatchSvcRoutingV1Endpoint(s, logger),

		AutoAddNeighborEndpoint:    MakeRoutingV1AutoAddNeighborEndpoint(s, logger),
		AutoDeleteNeighborEndpoint: MakeRoutingV1AutoDeleteNeighborEndpoint(s, logger),
		AutoGetNeighborEndpoint:    MakeRoutingV1AutoGetNeighborEndpoint(s, logger),
		AutoLabelNeighborEndpoint:  MakeRoutingV1AutoLabelNeighborEndpoint(s, logger),
		AutoListNeighborEndpoint:   MakeRoutingV1AutoListNeighborEndpoint(s, logger),
		AutoUpdateNeighborEndpoint: MakeRoutingV1AutoUpdateNeighborEndpoint(s, logger),

		watchHandlerNeighbor: MakeAutoWatchNeighborEndpoint(s, logger),
	}
}

// LoggingRoutingV1MiddlewareClient adds middleware for the client
func LoggingRoutingV1MiddlewareClient(logger log.Logger) MiddlewareRoutingV1Client {
	return func(next ServiceRoutingV1Client) ServiceRoutingV1Client {
		return loggingRoutingV1MiddlewareClient{
			logger: logger,
			next:   next,
		}
	}
}

type loggingRoutingV1MiddlewareClient struct {
	logger log.Logger
	next   ServiceRoutingV1Client
}

// LoggingRoutingV1MiddlewareServer adds middleware for the client
func LoggingRoutingV1MiddlewareServer(logger log.Logger) MiddlewareRoutingV1Server {
	return func(next ServiceRoutingV1Server) ServiceRoutingV1Server {
		return loggingRoutingV1MiddlewareServer{
			logger: logger,
			next:   next,
		}
	}
}

type loggingRoutingV1MiddlewareServer struct {
	logger log.Logger
	next   ServiceRoutingV1Server
}

func (m loggingRoutingV1MiddlewareClient) AutoAddNeighbor(ctx context.Context, in *Neighbor) (resp *Neighbor, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoAddNeighbor", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.AutoAddNeighbor(ctx, in)
	return
}
func (m loggingRoutingV1MiddlewareClient) AutoDeleteNeighbor(ctx context.Context, in *Neighbor) (resp *Neighbor, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoDeleteNeighbor", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.AutoDeleteNeighbor(ctx, in)
	return
}
func (m loggingRoutingV1MiddlewareClient) AutoGetNeighbor(ctx context.Context, in *Neighbor) (resp *Neighbor, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoGetNeighbor", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.AutoGetNeighbor(ctx, in)
	return
}
func (m loggingRoutingV1MiddlewareClient) AutoLabelNeighbor(ctx context.Context, in *api.Label) (resp *Neighbor, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoLabelNeighbor", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.AutoLabelNeighbor(ctx, in)
	return
}
func (m loggingRoutingV1MiddlewareClient) AutoListNeighbor(ctx context.Context, in *api.ListWatchOptions) (resp *NeighborList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoListNeighbor", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.AutoListNeighbor(ctx, in)
	return
}
func (m loggingRoutingV1MiddlewareClient) AutoUpdateNeighbor(ctx context.Context, in *Neighbor) (resp *Neighbor, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoUpdateNeighbor", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.AutoUpdateNeighbor(ctx, in)
	return
}

func (m loggingRoutingV1MiddlewareClient) AutoWatchSvcRoutingV1(ctx context.Context, in *api.AggWatchOptions) (resp RoutingV1_AutoWatchSvcRoutingV1Client, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoWatchSvcRoutingV1", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.AutoWatchSvcRoutingV1(ctx, in)
	return
}

func (m loggingRoutingV1MiddlewareClient) AutoWatchNeighbor(ctx context.Context, in *api.ListWatchOptions) (resp RoutingV1_AutoWatchNeighborClient, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoWatchNeighbor", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.AutoWatchNeighbor(ctx, in)
	return
}

func (m loggingRoutingV1MiddlewareServer) AutoAddNeighbor(ctx context.Context, in Neighbor) (resp Neighbor, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoAddNeighbor", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.AutoAddNeighbor(ctx, in)
	return
}
func (m loggingRoutingV1MiddlewareServer) AutoDeleteNeighbor(ctx context.Context, in Neighbor) (resp Neighbor, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoDeleteNeighbor", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.AutoDeleteNeighbor(ctx, in)
	return
}
func (m loggingRoutingV1MiddlewareServer) AutoGetNeighbor(ctx context.Context, in Neighbor) (resp Neighbor, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoGetNeighbor", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.AutoGetNeighbor(ctx, in)
	return
}
func (m loggingRoutingV1MiddlewareServer) AutoLabelNeighbor(ctx context.Context, in api.Label) (resp Neighbor, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoLabelNeighbor", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.AutoLabelNeighbor(ctx, in)
	return
}
func (m loggingRoutingV1MiddlewareServer) AutoListNeighbor(ctx context.Context, in api.ListWatchOptions) (resp NeighborList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoListNeighbor", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.AutoListNeighbor(ctx, in)
	return
}
func (m loggingRoutingV1MiddlewareServer) AutoUpdateNeighbor(ctx context.Context, in Neighbor) (resp Neighbor, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "RoutingV1", "method", "AutoUpdateNeighbor", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.AutoUpdateNeighbor(ctx, in)
	return
}

func (m loggingRoutingV1MiddlewareServer) AutoWatchSvcRoutingV1(in *api.AggWatchOptions, stream RoutingV1_AutoWatchSvcRoutingV1Server) (err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(stream.Context(), "service", "RoutingV1", "method", "AutoWatchSvcRoutingV1", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	err = m.next.AutoWatchSvcRoutingV1(in, stream)
	return
}

func (m loggingRoutingV1MiddlewareServer) AutoWatchNeighbor(in *api.ListWatchOptions, stream RoutingV1_AutoWatchNeighborServer) (err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(stream.Context(), "service", "RoutingV1", "method", "AutoWatchNeighbor", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	err = m.next.AutoWatchNeighbor(in, stream)
	return
}

func (r *EndpointsRoutingV1RestClient) updateHTTPHeader(ctx context.Context, header *http.Header) {
	val, ok := loginctx.AuthzHeaderFromContext(ctx)
	if ok {
		header.Add("Authorization", val)
	}
	val, ok = loginctx.ExtRequestIDHeaderFromContext(ctx)
	if ok {
		header.Add("Pensando-Psm-External-Request-Id", val)
	}
}
func (r *EndpointsRoutingV1RestClient) getHTTPRequest(ctx context.Context, in interface{}, method, path string) (*http.Request, error) {
	target, err := url.Parse(r.instance)
	if err != nil {
		return nil, fmt.Errorf("invalid instance %s", r.instance)
	}
	target.Path = path
	req, err := http.NewRequest(method, target.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("could not create request (%s)", err)
	}
	r.updateHTTPHeader(ctx, &req.Header)
	if err = encodeHTTPRequest(ctx, req, in); err != nil {
		return nil, fmt.Errorf("could not encode request (%s)", err)
	}
	return req, nil
}

//
func makeURIRoutingV1AutoAddNeighborCreateOper(in *Neighbor) string {
	return ""

}

//
func makeURIRoutingV1AutoDeleteNeighborDeleteOper(in *Neighbor) string {
	return ""

}

//
func makeURIRoutingV1AutoGetNeighborGetOper(in *Neighbor) string {
	return ""

}

//
func makeURIRoutingV1AutoLabelNeighborLabelOper(in *api.Label) string {
	return ""

}

//
func makeURIRoutingV1AutoListNeighborListOper(in *api.ListWatchOptions) string {
	return fmt.Sprint("/routing/v1", "/neighbors")
}

//
func makeURIRoutingV1AutoUpdateNeighborUpdateOper(in *Neighbor) string {
	return ""

}

//
func makeURIRoutingV1AutoWatchNeighborWatchOper(in *api.ListWatchOptions) string {
	return ""

}

//
func makeURIRoutingV1AutoWatchSvcRoutingV1WatchOper(in *api.AggWatchOptions) string {
	return ""

}

// AutoAddNeighbor CRUD method for Neighbor
func (r *EndpointsRoutingV1RestClient) AutoAddNeighbor(ctx context.Context, in *Neighbor) (*Neighbor, error) {
	return nil, errors.New("not allowed")
}

// AutoUpdateNeighbor CRUD method for Neighbor
func (r *EndpointsRoutingV1RestClient) AutoUpdateNeighbor(ctx context.Context, in *Neighbor) (*Neighbor, error) {
	return nil, errors.New("not allowed")
}

// AutoLabelNeighbor label method for Neighbor
func (r *EndpointsRoutingV1RestClient) AutoLabelNeighbor(ctx context.Context, in *api.Label) (*Neighbor, error) {
	return nil, errors.New("not allowed")
}

// AutoGetNeighbor CRUD method for Neighbor
func (r *EndpointsRoutingV1RestClient) AutoGetNeighbor(ctx context.Context, in *Neighbor) (*Neighbor, error) {
	return nil, errors.New("not allowed")
}

// AutoDeleteNeighbor CRUD method for Neighbor
func (r *EndpointsRoutingV1RestClient) AutoDeleteNeighbor(ctx context.Context, in *Neighbor) (*Neighbor, error) {
	return nil, errors.New("not allowed")
}

// AutoListNeighbor CRUD method for Neighbor
func (r *EndpointsRoutingV1RestClient) AutoListNeighbor(ctx context.Context, options *api.ListWatchOptions) (*NeighborList, error) {
	path := makeURIRoutingV1AutoListNeighborListOper(options)
	if r.bufferId != "" {
		path = strings.Replace(path, "/configs", "/staging/"+r.bufferId, 1)
	}
	req, err := r.getHTTPRequest(ctx, options, "GET", path)
	if err != nil {
		return nil, err
	}
	resp, err := r.client.Do(req.WithContext(ctx))
	if err != nil {
		return nil, fmt.Errorf("request failed (%s)", err)
	}
	defer resp.Body.Close()
	ret, err := decodeHTTPrespRoutingV1AutoListNeighbor(ctx, resp)
	if err != nil {
		return nil, err
	}
	return ret.(*NeighborList), err
}

// AutoWatchNeighbor CRUD method for Neighbor
func (r *EndpointsRoutingV1RestClient) AutoWatchNeighbor(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	path := r.instance + makeURIRoutingV1AutoWatchNeighborWatchOper(options)
	path = strings.Replace(path, "http://", "ws://", 1)
	path = strings.Replace(path, "https://", "wss://", 1)
	params := apiutils.GetQueryStringFromListWatchOptions(options)
	if params != "" {
		path = path + "?" + params
	}
	header := http.Header{}
	r.updateHTTPHeader(ctx, &header)
	dialer := websocket.DefaultDialer
	dialer.TLSClientConfig = &tls.Config{InsecureSkipVerify: true}
	conn, hresp, err := dialer.Dial(path, header)
	if err != nil {
		return nil, fmt.Errorf("failed to connect web socket to [%s](%s)[%+v]", path, err, hresp)
	}
	bridgefn := func(lw *listerwatcher.WatcherClient) {
		for {
			in := &AutoMsgNeighborWatchHelper{}
			err := conn.ReadJSON(in)
			if err != nil {
				return
			}
			for _, e := range in.Events {
				ev := kvstore.WatchEvent{
					Type:   kvstore.WatchEventType(e.Type),
					Object: e.Object,
				}
				select {
				case lw.OutCh <- &ev:
				case <-ctx.Done():
					close(lw.OutCh)
					conn.WriteControl(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.CloseNormalClosure, "client closing"), time.Now().Add(3*time.Second))
					return
				}
			}
		}
	}
	lw := listerwatcher.NewWatcherClient(nil, bridgefn)
	lw.Run()
	go func() {
		<-ctx.Done()
		conn.WriteControl(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.CloseNormalClosure, "client closing"), time.Now().Add(3*time.Second))
	}()
	return lw, nil
}

// MakeRoutingV1RestClientEndpoints make REST client endpoints
func MakeRoutingV1RestClientEndpoints(instance string, httpClient *http.Client) (EndpointsRoutingV1RestClient, error) {
	if !strings.HasPrefix(instance, "https") {
		instance = "https://" + instance
	}

	return EndpointsRoutingV1RestClient{
		instance: instance,
		client:   httpClient,
	}, nil

}

// MakeRoutingV1StagedRestClientEndpoints makes staged REST client endpoints
func MakeRoutingV1StagedRestClientEndpoints(instance string, bufferId string, httpClient *http.Client) (EndpointsRoutingV1RestClient, error) {
	if !strings.HasPrefix(instance, "https") {
		instance = "https://" + instance
	}

	return EndpointsRoutingV1RestClient{
		instance: instance,
		bufferId: bufferId,
		client:   httpClient,
	}, nil
}
