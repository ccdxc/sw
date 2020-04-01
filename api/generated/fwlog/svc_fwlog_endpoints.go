// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package fwlog is a auto generated package.
Input file: svc_fwlog.proto
*/
package fwlog

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

// MiddlewareFwLogV1Client add middleware to the client
type MiddlewareFwLogV1Client func(ServiceFwLogV1Client) ServiceFwLogV1Client

// EndpointsFwLogV1Client is the endpoints for the client
type EndpointsFwLogV1Client struct {
	Client                      FwLogV1Client
	AutoWatchSvcFwLogV1Endpoint endpoint.Endpoint

	GetLogsEndpoint endpoint.Endpoint
}

// EndpointsFwLogV1RestClient is the REST client
type EndpointsFwLogV1RestClient struct {
	logger   log.Logger
	client   *http.Client
	instance string
	bufferId string

	AutoWatchSvcFwLogV1Endpoint endpoint.Endpoint
	GetLogsEndpoint             endpoint.Endpoint
}

// MiddlewareFwLogV1Server adds middle ware to the server
type MiddlewareFwLogV1Server func(ServiceFwLogV1Server) ServiceFwLogV1Server

// EndpointsFwLogV1Server is the server endpoints
type EndpointsFwLogV1Server struct {
	svcWatchHandlerFwLogV1 func(options *api.ListWatchOptions, stream grpc.ServerStream) error

	GetLogsEndpoint endpoint.Endpoint
}

// GetLogs is endpoint for GetLogs
func (e EndpointsFwLogV1Client) GetLogs(ctx context.Context, in *FwLogQuery) (*FwLogList, error) {
	resp, err := e.GetLogsEndpoint(ctx, in)
	if err != nil {
		return &FwLogList{}, err
	}
	return resp.(*FwLogList), nil
}

type respFwLogV1GetLogs struct {
	V   FwLogList
	Err error
}

func (e EndpointsFwLogV1Client) AutoWatchSvcFwLogV1(ctx context.Context, in *api.ListWatchOptions) (FwLogV1_AutoWatchSvcFwLogV1Client, error) {
	return nil, errors.New("not implemented")
}

// GetLogs implementation on server Endpoint
func (e EndpointsFwLogV1Server) GetLogs(ctx context.Context, in FwLogQuery) (FwLogList, error) {
	resp, err := e.GetLogsEndpoint(ctx, in)
	if err != nil {
		return FwLogList{}, err
	}
	return *resp.(*FwLogList), nil
}

// MakeFwLogV1GetLogsEndpoint creates  GetLogs endpoints for the service
func MakeFwLogV1GetLogsEndpoint(s ServiceFwLogV1Server, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*FwLogQuery)
		v, err := s.GetLogs(ctx, *req)
		return respFwLogV1GetLogs{
			V:   v,
			Err: err,
		}, nil
	}
	return trace.ServerEndpoint("FwLogV1:GetLogs")(f)
}

// MakeAutoWatchSvcFwLogV1Endpoint creates the Watch endpoint for the service
func MakeAutoWatchSvcFwLogV1Endpoint(s ServiceFwLogV1Server, logger log.Logger) func(options *api.ListWatchOptions, stream grpc.ServerStream) error {
	return func(options *api.ListWatchOptions, stream grpc.ServerStream) error {
		return errors.New("not implemented")
	}
}

// MakeFwLogV1ServerEndpoints creates server endpoints
func MakeFwLogV1ServerEndpoints(s ServiceFwLogV1Server, logger log.Logger) EndpointsFwLogV1Server {
	return EndpointsFwLogV1Server{
		svcWatchHandlerFwLogV1: MakeAutoWatchSvcFwLogV1Endpoint(s, logger),

		GetLogsEndpoint: MakeFwLogV1GetLogsEndpoint(s, logger),
	}
}

// LoggingFwLogV1MiddlewareClient adds middleware for the client
func LoggingFwLogV1MiddlewareClient(logger log.Logger) MiddlewareFwLogV1Client {
	return func(next ServiceFwLogV1Client) ServiceFwLogV1Client {
		return loggingFwLogV1MiddlewareClient{
			logger: logger,
			next:   next,
		}
	}
}

type loggingFwLogV1MiddlewareClient struct {
	logger log.Logger
	next   ServiceFwLogV1Client
}

// LoggingFwLogV1MiddlewareServer adds middleware for the client
func LoggingFwLogV1MiddlewareServer(logger log.Logger) MiddlewareFwLogV1Server {
	return func(next ServiceFwLogV1Server) ServiceFwLogV1Server {
		return loggingFwLogV1MiddlewareServer{
			logger: logger,
			next:   next,
		}
	}
}

type loggingFwLogV1MiddlewareServer struct {
	logger log.Logger
	next   ServiceFwLogV1Server
}

func (m loggingFwLogV1MiddlewareClient) GetLogs(ctx context.Context, in *FwLogQuery) (resp *FwLogList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "FwLogV1", "method", "GetLogs", "result", rslt, "duration", time.Since(begin), "error", err)
	}(time.Now())
	resp, err = m.next.GetLogs(ctx, in)
	return
}

func (m loggingFwLogV1MiddlewareClient) AutoWatchSvcFwLogV1(ctx context.Context, in *api.ListWatchOptions) (FwLogV1_AutoWatchSvcFwLogV1Client, error) {
	return nil, errors.New("not implemented")
}

func (m loggingFwLogV1MiddlewareServer) GetLogs(ctx context.Context, in FwLogQuery) (resp FwLogList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "FwLogV1", "method", "GetLogs", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetLogs(ctx, in)
	return
}

func (m loggingFwLogV1MiddlewareServer) AutoWatchSvcFwLogV1(in *api.ListWatchOptions, stream FwLogV1_AutoWatchSvcFwLogV1Server) error {
	return errors.New("Not implemented")
}

func (r *EndpointsFwLogV1RestClient) updateHTTPHeader(ctx context.Context, header *http.Header) {
	val, ok := loginctx.AuthzHeaderFromContext(ctx)
	if ok {
		header.Add("Authorization", val)
	}
	val, ok = loginctx.ExtRequestIDHeaderFromContext(ctx)
	if ok {
		header.Add("Pensando-Psm-External-Request-Id", val)
	}
}
func (r *EndpointsFwLogV1RestClient) getHTTPRequest(ctx context.Context, in interface{}, method, path string) (*http.Request, error) {
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
func makeURIFwLogV1AutoWatchSvcFwLogV1WatchOper(in *api.ListWatchOptions) string {
	return ""

}

func (r *EndpointsFwLogV1RestClient) FwLogV1GetLogsEndpoint(ctx context.Context, in *FwLogQuery) (*FwLogList, error) {
	return nil, errors.New("not allowed")
}

// MakeFwLogV1RestClientEndpoints make REST client endpoints
func MakeFwLogV1RestClientEndpoints(instance string, httpClient *http.Client) (EndpointsFwLogV1RestClient, error) {
	if !strings.HasPrefix(instance, "https") {
		instance = "https://" + instance
	}

	return EndpointsFwLogV1RestClient{
		instance: instance,
		client:   httpClient,
	}, nil

}

// MakeFwLogV1StagedRestClientEndpoints makes staged REST client endpoints
func MakeFwLogV1StagedRestClientEndpoints(instance string, bufferId string, httpClient *http.Client) (EndpointsFwLogV1RestClient, error) {
	if !strings.HasPrefix(instance, "https") {
		instance = "https://" + instance
	}

	return EndpointsFwLogV1RestClient{
		instance: instance,
		bufferId: bufferId,
		client:   httpClient,
	}, nil
}
