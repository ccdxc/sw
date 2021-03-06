// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package telemetry_query is a auto generated package.
Input file: svc_telemetry_query.proto
*/
package telemetry_query

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
	"github.com/pensando/sw/api/listerwatcher"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/trace"
)

// Dummy definitions to suppress nonused warnings
var _ api.ObjectMeta
var _ grpc.ServerStream
var _ fmt.Formatter
var _ *listerwatcher.WatcherClient

// MiddlewareTelemetryV1Client add middleware to the client
type MiddlewareTelemetryV1Client func(ServiceTelemetryV1Client) ServiceTelemetryV1Client

// EndpointsTelemetryV1Client is the endpoints for the client
type EndpointsTelemetryV1Client struct {
	Client                          TelemetryV1Client
	AutoWatchSvcTelemetryV1Endpoint endpoint.Endpoint

	FwlogsEndpoint  endpoint.Endpoint
	MetricsEndpoint endpoint.Endpoint
}

// EndpointsTelemetryV1RestClient is the REST client
type EndpointsTelemetryV1RestClient struct {
	logger   log.Logger
	client   *http.Client
	instance string
	bufferId string

	AutoWatchSvcTelemetryV1Endpoint endpoint.Endpoint
	FwlogsEndpoint                  endpoint.Endpoint
	MetricsEndpoint                 endpoint.Endpoint
}

// MiddlewareTelemetryV1Server adds middle ware to the server
type MiddlewareTelemetryV1Server func(ServiceTelemetryV1Server) ServiceTelemetryV1Server

// EndpointsTelemetryV1Server is the server endpoints
type EndpointsTelemetryV1Server struct {
	svcWatchHandlerTelemetryV1 func(options *api.AggWatchOptions, stream grpc.ServerStream) error

	FwlogsEndpoint  endpoint.Endpoint
	MetricsEndpoint endpoint.Endpoint
}

// Fwlogs is endpoint for Fwlogs
func (e EndpointsTelemetryV1Client) Fwlogs(ctx context.Context, in *FwlogsQueryList) (*FwlogsQueryResponse, error) {
	resp, err := e.FwlogsEndpoint(ctx, in)
	if err != nil {
		return &FwlogsQueryResponse{}, err
	}
	return resp.(*FwlogsQueryResponse), nil
}

type respTelemetryV1Fwlogs struct {
	V   FwlogsQueryResponse
	Err error
}

// Metrics is endpoint for Metrics
func (e EndpointsTelemetryV1Client) Metrics(ctx context.Context, in *MetricsQueryList) (*MetricsQueryResponse, error) {
	resp, err := e.MetricsEndpoint(ctx, in)
	if err != nil {
		return &MetricsQueryResponse{}, err
	}
	return resp.(*MetricsQueryResponse), nil
}

type respTelemetryV1Metrics struct {
	V   MetricsQueryResponse
	Err error
}

func (e EndpointsTelemetryV1Client) AutoWatchSvcTelemetryV1(ctx context.Context, in *api.AggWatchOptions) (TelemetryV1_AutoWatchSvcTelemetryV1Client, error) {
	return e.Client.AutoWatchSvcTelemetryV1(ctx, in)
}

// Fwlogs implementation on server Endpoint
func (e EndpointsTelemetryV1Server) Fwlogs(ctx context.Context, in FwlogsQueryList) (FwlogsQueryResponse, error) {
	resp, err := e.FwlogsEndpoint(ctx, in)
	if err != nil {
		return FwlogsQueryResponse{}, err
	}
	return *resp.(*FwlogsQueryResponse), nil
}

// MakeTelemetryV1FwlogsEndpoint creates  Fwlogs endpoints for the service
func MakeTelemetryV1FwlogsEndpoint(s ServiceTelemetryV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*FwlogsQueryList)
		v, err := s.Fwlogs(ctx, *req)
		return respTelemetryV1Fwlogs{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("TelemetryV1:Fwlogs")(f)
}

// Metrics implementation on server Endpoint
func (e EndpointsTelemetryV1Server) Metrics(ctx context.Context, in MetricsQueryList) (MetricsQueryResponse, error) {
	resp, err := e.MetricsEndpoint(ctx, in)
	if err != nil {
		return MetricsQueryResponse{}, err
	}
	return *resp.(*MetricsQueryResponse), nil
}

// MakeTelemetryV1MetricsEndpoint creates  Metrics endpoints for the service
func MakeTelemetryV1MetricsEndpoint(s ServiceTelemetryV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*MetricsQueryList)
		v, err := s.Metrics(ctx, *req)
		return respTelemetryV1Metrics{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("TelemetryV1:Metrics")(f)
}

func (e EndpointsTelemetryV1Server) AutoWatchSvcTelemetryV1(in *api.AggWatchOptions, stream TelemetryV1_AutoWatchSvcTelemetryV1Server) error {
	return e.svcWatchHandlerTelemetryV1(in, stream)
}

// MakeAutoWatchSvcTelemetryV1Endpoint creates the Watch endpoint for the service
func MakeAutoWatchSvcTelemetryV1Endpoint(s ServiceTelemetryV1Server, logger log.Logger) func(options *api.AggWatchOptions, stream grpc.ServerStream) error {
	return func(options *api.AggWatchOptions, stream grpc.ServerStream) error {
		wstream := stream.(TelemetryV1_AutoWatchSvcTelemetryV1Server)
		return s.AutoWatchSvcTelemetryV1(options, wstream)
	}
}

// MakeTelemetryV1ServerEndpoints creates server endpoints
func MakeTelemetryV1ServerEndpoints(s ServiceTelemetryV1Server, logger log.Logger) EndpointsTelemetryV1Server {
	return EndpointsTelemetryV1Server{
		svcWatchHandlerTelemetryV1: MakeAutoWatchSvcTelemetryV1Endpoint(s, logger),

		FwlogsEndpoint:  MakeTelemetryV1FwlogsEndpoint(s, logger),
		MetricsEndpoint: MakeTelemetryV1MetricsEndpoint(s, logger),
	}
}

// LoggingTelemetryV1MiddlewareClient adds middleware for the client
func LoggingTelemetryV1MiddlewareClient(logger log.Logger) MiddlewareTelemetryV1Client {
	return func(next ServiceTelemetryV1Client) ServiceTelemetryV1Client {
		return loggingTelemetryV1MiddlewareClient{
			logger: logger,
			next:   next,
		}
	}
}

type loggingTelemetryV1MiddlewareClient struct {
	logger log.Logger
	next   ServiceTelemetryV1Client
}

// LoggingTelemetryV1MiddlewareServer adds middleware for the client
func LoggingTelemetryV1MiddlewareServer(logger log.Logger) MiddlewareTelemetryV1Server {
	return func(next ServiceTelemetryV1Server) ServiceTelemetryV1Server {
		return loggingTelemetryV1MiddlewareServer{
			logger: logger,
			next:   next,
		}
	}
}

type loggingTelemetryV1MiddlewareServer struct {
	logger log.Logger
	next   ServiceTelemetryV1Server
}

func (m loggingTelemetryV1MiddlewareClient) Fwlogs(ctx context.Context, in *FwlogsQueryList) (resp *FwlogsQueryResponse, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "TelemetryV1", "method", "Fwlogs", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.Fwlogs(ctx, in)
	return
}
func (m loggingTelemetryV1MiddlewareClient) Metrics(ctx context.Context, in *MetricsQueryList) (resp *MetricsQueryResponse, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "TelemetryV1", "method", "Metrics", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.Metrics(ctx, in)
	return
}

func (m loggingTelemetryV1MiddlewareClient) AutoWatchSvcTelemetryV1(ctx context.Context, in *api.AggWatchOptions) (resp TelemetryV1_AutoWatchSvcTelemetryV1Client, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "TelemetryV1", "method", "AutoWatchSvcTelemetryV1", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.AutoWatchSvcTelemetryV1(ctx, in)
	return
}

func (m loggingTelemetryV1MiddlewareServer) Fwlogs(ctx context.Context, in FwlogsQueryList) (resp FwlogsQueryResponse, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "TelemetryV1", "method", "Fwlogs", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.Fwlogs(ctx, in)
	return
}
func (m loggingTelemetryV1MiddlewareServer) Metrics(ctx context.Context, in MetricsQueryList) (resp MetricsQueryResponse, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "TelemetryV1", "method", "Metrics", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.Metrics(ctx, in)
	return
}

func (m loggingTelemetryV1MiddlewareServer) AutoWatchSvcTelemetryV1(in *api.AggWatchOptions, stream TelemetryV1_AutoWatchSvcTelemetryV1Server) (err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(stream.Context(), "service", "TelemetryV1", "method", "AutoWatchSvcTelemetryV1", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	err = m.next.AutoWatchSvcTelemetryV1(in, stream)
	return
}

func (r *EndpointsTelemetryV1RestClient) updateHTTPHeader(ctx context.Context, header *http.Header) {
	val, ok := loginctx.AuthzHeaderFromContext(ctx)
	if ok {
		header.Add("Authorization", val)
	}
	val, ok = loginctx.ExtRequestIDHeaderFromContext(ctx)
	if ok {
		header.Add("Pensando-Psm-External-Request-Id", val)
	}
}
func (r *EndpointsTelemetryV1RestClient) getHTTPRequest(ctx context.Context, in interface{}, method, path string) (*http.Request, error) {
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
func makeURITelemetryV1AutoWatchSvcTelemetryV1WatchOper(in *api.AggWatchOptions) string {
	return ""

}

func (r *EndpointsTelemetryV1RestClient) TelemetryV1FwlogsEndpoint(ctx context.Context, in *FwlogsQueryList) (*FwlogsQueryResponse, error) {
	return nil, errors.New("not allowed")
}

func (r *EndpointsTelemetryV1RestClient) TelemetryV1MetricsEndpoint(ctx context.Context, in *MetricsQueryList) (*MetricsQueryResponse, error) {
	return nil, errors.New("not allowed")
}

// MakeTelemetryV1RestClientEndpoints make REST client endpoints
func MakeTelemetryV1RestClientEndpoints(instance string, httpClient *http.Client) (EndpointsTelemetryV1RestClient, error) {
	if !strings.HasPrefix(instance, "https") {
		instance = "https://" + instance
	}

	return EndpointsTelemetryV1RestClient{
		instance: instance,
		client:   httpClient,
	}, nil

}

// MakeTelemetryV1StagedRestClientEndpoints makes staged REST client endpoints
func MakeTelemetryV1StagedRestClientEndpoints(instance string, bufferId string, httpClient *http.Client) (EndpointsTelemetryV1RestClient, error) {
	if !strings.HasPrefix(instance, "https") {
		instance = "https://" + instance
	}

	return EndpointsTelemetryV1RestClient{
		instance: instance,
		bufferId: bufferId,
		client:   httpClient,
	}, nil
}
