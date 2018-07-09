// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package workload is a auto generated package.
Input file: svc_workload.proto
*/
package workload

import (
	"context"
	"encoding/json"
	"net/http"

	grpctransport "github.com/go-kit/kit/transport/grpc"
	oldcontext "golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/trace"
)

// Dummy definitions to suppress nonused warnings
var _ api.ObjectMeta

type grpcServerWorkloadV1 struct {
	Endpoints EndpointsWorkloadV1Server

	AutoAddEndpointHdlr    grpctransport.Handler
	AutoAddWorkloadHdlr    grpctransport.Handler
	AutoDeleteEndpointHdlr grpctransport.Handler
	AutoDeleteWorkloadHdlr grpctransport.Handler
	AutoGetEndpointHdlr    grpctransport.Handler
	AutoGetWorkloadHdlr    grpctransport.Handler
	AutoListEndpointHdlr   grpctransport.Handler
	AutoListWorkloadHdlr   grpctransport.Handler
	AutoUpdateEndpointHdlr grpctransport.Handler
	AutoUpdateWorkloadHdlr grpctransport.Handler
}

// MakeGRPCServerWorkloadV1 creates a GRPC server for WorkloadV1 service
func MakeGRPCServerWorkloadV1(ctx context.Context, endpoints EndpointsWorkloadV1Server, logger log.Logger) WorkloadV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerWorkloadV1{
		Endpoints: endpoints,
		AutoAddEndpointHdlr: grpctransport.NewServer(
			endpoints.AutoAddEndpointEndpoint,
			DecodeGrpcReqEndpoint,
			EncodeGrpcRespEndpoint,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoAddEndpoint", logger)))...,
		),

		AutoAddWorkloadHdlr: grpctransport.NewServer(
			endpoints.AutoAddWorkloadEndpoint,
			DecodeGrpcReqWorkload,
			EncodeGrpcRespWorkload,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoAddWorkload", logger)))...,
		),

		AutoDeleteEndpointHdlr: grpctransport.NewServer(
			endpoints.AutoDeleteEndpointEndpoint,
			DecodeGrpcReqEndpoint,
			EncodeGrpcRespEndpoint,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoDeleteEndpoint", logger)))...,
		),

		AutoDeleteWorkloadHdlr: grpctransport.NewServer(
			endpoints.AutoDeleteWorkloadEndpoint,
			DecodeGrpcReqWorkload,
			EncodeGrpcRespWorkload,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoDeleteWorkload", logger)))...,
		),

		AutoGetEndpointHdlr: grpctransport.NewServer(
			endpoints.AutoGetEndpointEndpoint,
			DecodeGrpcReqEndpoint,
			EncodeGrpcRespEndpoint,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoGetEndpoint", logger)))...,
		),

		AutoGetWorkloadHdlr: grpctransport.NewServer(
			endpoints.AutoGetWorkloadEndpoint,
			DecodeGrpcReqWorkload,
			EncodeGrpcRespWorkload,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoGetWorkload", logger)))...,
		),

		AutoListEndpointHdlr: grpctransport.NewServer(
			endpoints.AutoListEndpointEndpoint,
			DecodeGrpcReqListWatchOptions,
			EncodeGrpcRespEndpointList,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoListEndpoint", logger)))...,
		),

		AutoListWorkloadHdlr: grpctransport.NewServer(
			endpoints.AutoListWorkloadEndpoint,
			DecodeGrpcReqListWatchOptions,
			EncodeGrpcRespWorkloadList,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoListWorkload", logger)))...,
		),

		AutoUpdateEndpointHdlr: grpctransport.NewServer(
			endpoints.AutoUpdateEndpointEndpoint,
			DecodeGrpcReqEndpoint,
			EncodeGrpcRespEndpoint,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoUpdateEndpoint", logger)))...,
		),

		AutoUpdateWorkloadHdlr: grpctransport.NewServer(
			endpoints.AutoUpdateWorkloadEndpoint,
			DecodeGrpcReqWorkload,
			EncodeGrpcRespWorkload,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoUpdateWorkload", logger)))...,
		),
	}
}

func (s *grpcServerWorkloadV1) AutoAddEndpoint(ctx oldcontext.Context, req *Endpoint) (*Endpoint, error) {
	_, resp, err := s.AutoAddEndpointHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respWorkloadV1AutoAddEndpoint).V
	return &r, resp.(respWorkloadV1AutoAddEndpoint).Err
}

func decodeHTTPrespWorkloadV1AutoAddEndpoint(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp Endpoint
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerWorkloadV1) AutoAddWorkload(ctx oldcontext.Context, req *Workload) (*Workload, error) {
	_, resp, err := s.AutoAddWorkloadHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respWorkloadV1AutoAddWorkload).V
	return &r, resp.(respWorkloadV1AutoAddWorkload).Err
}

func decodeHTTPrespWorkloadV1AutoAddWorkload(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp Workload
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerWorkloadV1) AutoDeleteEndpoint(ctx oldcontext.Context, req *Endpoint) (*Endpoint, error) {
	_, resp, err := s.AutoDeleteEndpointHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respWorkloadV1AutoDeleteEndpoint).V
	return &r, resp.(respWorkloadV1AutoDeleteEndpoint).Err
}

func decodeHTTPrespWorkloadV1AutoDeleteEndpoint(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp Endpoint
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerWorkloadV1) AutoDeleteWorkload(ctx oldcontext.Context, req *Workload) (*Workload, error) {
	_, resp, err := s.AutoDeleteWorkloadHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respWorkloadV1AutoDeleteWorkload).V
	return &r, resp.(respWorkloadV1AutoDeleteWorkload).Err
}

func decodeHTTPrespWorkloadV1AutoDeleteWorkload(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp Workload
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerWorkloadV1) AutoGetEndpoint(ctx oldcontext.Context, req *Endpoint) (*Endpoint, error) {
	_, resp, err := s.AutoGetEndpointHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respWorkloadV1AutoGetEndpoint).V
	return &r, resp.(respWorkloadV1AutoGetEndpoint).Err
}

func decodeHTTPrespWorkloadV1AutoGetEndpoint(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp Endpoint
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerWorkloadV1) AutoGetWorkload(ctx oldcontext.Context, req *Workload) (*Workload, error) {
	_, resp, err := s.AutoGetWorkloadHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respWorkloadV1AutoGetWorkload).V
	return &r, resp.(respWorkloadV1AutoGetWorkload).Err
}

func decodeHTTPrespWorkloadV1AutoGetWorkload(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp Workload
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerWorkloadV1) AutoListEndpoint(ctx oldcontext.Context, req *api.ListWatchOptions) (*EndpointList, error) {
	_, resp, err := s.AutoListEndpointHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respWorkloadV1AutoListEndpoint).V
	return &r, resp.(respWorkloadV1AutoListEndpoint).Err
}

func decodeHTTPrespWorkloadV1AutoListEndpoint(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp EndpointList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerWorkloadV1) AutoListWorkload(ctx oldcontext.Context, req *api.ListWatchOptions) (*WorkloadList, error) {
	_, resp, err := s.AutoListWorkloadHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respWorkloadV1AutoListWorkload).V
	return &r, resp.(respWorkloadV1AutoListWorkload).Err
}

func decodeHTTPrespWorkloadV1AutoListWorkload(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp WorkloadList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerWorkloadV1) AutoUpdateEndpoint(ctx oldcontext.Context, req *Endpoint) (*Endpoint, error) {
	_, resp, err := s.AutoUpdateEndpointHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respWorkloadV1AutoUpdateEndpoint).V
	return &r, resp.(respWorkloadV1AutoUpdateEndpoint).Err
}

func decodeHTTPrespWorkloadV1AutoUpdateEndpoint(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp Endpoint
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerWorkloadV1) AutoUpdateWorkload(ctx oldcontext.Context, req *Workload) (*Workload, error) {
	_, resp, err := s.AutoUpdateWorkloadHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respWorkloadV1AutoUpdateWorkload).V
	return &r, resp.(respWorkloadV1AutoUpdateWorkload).Err
}

func decodeHTTPrespWorkloadV1AutoUpdateWorkload(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp Workload
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerWorkloadV1) AutoWatchSvcWorkloadV1(in *api.ListWatchOptions, stream WorkloadV1_AutoWatchSvcWorkloadV1Server) error {
	return s.Endpoints.AutoWatchSvcWorkloadV1(in, stream)
}

func (s *grpcServerWorkloadV1) AutoWatchEndpoint(in *api.ListWatchOptions, stream WorkloadV1_AutoWatchEndpointServer) error {
	return s.Endpoints.AutoWatchEndpoint(in, stream)
}

func (s *grpcServerWorkloadV1) AutoWatchWorkload(in *api.ListWatchOptions, stream WorkloadV1_AutoWatchWorkloadServer) error {
	return s.Endpoints.AutoWatchWorkload(in, stream)
}

func encodeHTTPEndpointList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPEndpointList(_ context.Context, r *http.Request) (interface{}, error) {
	var req EndpointList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqEndpointList encodes GRPC request
func EncodeGrpcReqEndpointList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EndpointList)
	return req, nil
}

// DecodeGrpcReqEndpointList decodes GRPC request
func DecodeGrpcReqEndpointList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EndpointList)
	return req, nil
}

// EncodeGrpcRespEndpointList endodes the GRPC response
func EncodeGrpcRespEndpointList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespEndpointList decodes the GRPC response
func DecodeGrpcRespEndpointList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPWorkloadList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPWorkloadList(_ context.Context, r *http.Request) (interface{}, error) {
	var req WorkloadList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqWorkloadList encodes GRPC request
func EncodeGrpcReqWorkloadList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*WorkloadList)
	return req, nil
}

// DecodeGrpcReqWorkloadList decodes GRPC request
func DecodeGrpcReqWorkloadList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*WorkloadList)
	return req, nil
}

// EncodeGrpcRespWorkloadList endodes the GRPC response
func EncodeGrpcRespWorkloadList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespWorkloadList decodes the GRPC response
func DecodeGrpcRespWorkloadList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}
