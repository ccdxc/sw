// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package events is a auto generated package.
Input file: protos/events.proto
*/
package events

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

type grpcServerEventPolicyV1 struct {
	Endpoints EndpointsEventPolicyV1Server

	AutoAddEventPolicyHdlr    grpctransport.Handler
	AutoDeleteEventPolicyHdlr grpctransport.Handler
	AutoGetEventPolicyHdlr    grpctransport.Handler
	AutoListEventPolicyHdlr   grpctransport.Handler
	AutoUpdateEventPolicyHdlr grpctransport.Handler
}

// MakeGRPCServerEventPolicyV1 creates a GRPC server for EventPolicyV1 service
func MakeGRPCServerEventPolicyV1(ctx context.Context, endpoints EndpointsEventPolicyV1Server, logger log.Logger) EventPolicyV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerEventPolicyV1{
		Endpoints: endpoints,
		AutoAddEventPolicyHdlr: grpctransport.NewServer(
			endpoints.AutoAddEventPolicyEndpoint,
			DecodeGrpcReqEventPolicy,
			EncodeGrpcRespEventPolicy,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoAddEventPolicy", logger)))...,
		),

		AutoDeleteEventPolicyHdlr: grpctransport.NewServer(
			endpoints.AutoDeleteEventPolicyEndpoint,
			DecodeGrpcReqEventPolicy,
			EncodeGrpcRespEventPolicy,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoDeleteEventPolicy", logger)))...,
		),

		AutoGetEventPolicyHdlr: grpctransport.NewServer(
			endpoints.AutoGetEventPolicyEndpoint,
			DecodeGrpcReqEventPolicy,
			EncodeGrpcRespEventPolicy,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoGetEventPolicy", logger)))...,
		),

		AutoListEventPolicyHdlr: grpctransport.NewServer(
			endpoints.AutoListEventPolicyEndpoint,
			DecodeGrpcReqListWatchOptions,
			EncodeGrpcRespEventPolicyList,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoListEventPolicy", logger)))...,
		),

		AutoUpdateEventPolicyHdlr: grpctransport.NewServer(
			endpoints.AutoUpdateEventPolicyEndpoint,
			DecodeGrpcReqEventPolicy,
			EncodeGrpcRespEventPolicy,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("AutoUpdateEventPolicy", logger)))...,
		),
	}
}

func (s *grpcServerEventPolicyV1) AutoAddEventPolicy(ctx oldcontext.Context, req *EventPolicy) (*EventPolicy, error) {
	_, resp, err := s.AutoAddEventPolicyHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respEventPolicyV1AutoAddEventPolicy).V
	return &r, resp.(respEventPolicyV1AutoAddEventPolicy).Err
}

func decodeHTTPrespEventPolicyV1AutoAddEventPolicy(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp EventPolicy
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerEventPolicyV1) AutoDeleteEventPolicy(ctx oldcontext.Context, req *EventPolicy) (*EventPolicy, error) {
	_, resp, err := s.AutoDeleteEventPolicyHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respEventPolicyV1AutoDeleteEventPolicy).V
	return &r, resp.(respEventPolicyV1AutoDeleteEventPolicy).Err
}

func decodeHTTPrespEventPolicyV1AutoDeleteEventPolicy(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp EventPolicy
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerEventPolicyV1) AutoGetEventPolicy(ctx oldcontext.Context, req *EventPolicy) (*EventPolicy, error) {
	_, resp, err := s.AutoGetEventPolicyHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respEventPolicyV1AutoGetEventPolicy).V
	return &r, resp.(respEventPolicyV1AutoGetEventPolicy).Err
}

func decodeHTTPrespEventPolicyV1AutoGetEventPolicy(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp EventPolicy
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerEventPolicyV1) AutoListEventPolicy(ctx oldcontext.Context, req *api.ListWatchOptions) (*EventPolicyList, error) {
	_, resp, err := s.AutoListEventPolicyHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respEventPolicyV1AutoListEventPolicy).V
	return &r, resp.(respEventPolicyV1AutoListEventPolicy).Err
}

func decodeHTTPrespEventPolicyV1AutoListEventPolicy(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp EventPolicyList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerEventPolicyV1) AutoUpdateEventPolicy(ctx oldcontext.Context, req *EventPolicy) (*EventPolicy, error) {
	_, resp, err := s.AutoUpdateEventPolicyHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respEventPolicyV1AutoUpdateEventPolicy).V
	return &r, resp.(respEventPolicyV1AutoUpdateEventPolicy).Err
}

func decodeHTTPrespEventPolicyV1AutoUpdateEventPolicy(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp EventPolicy
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerEventPolicyV1) AutoWatchEventPolicy(in *api.ListWatchOptions, stream EventPolicyV1_AutoWatchEventPolicyServer) error {
	return s.Endpoints.AutoWatchEventPolicy(in, stream)
}

func encodeHTTPEvent(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPEvent(_ context.Context, r *http.Request) (interface{}, error) {
	var req Event
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqEvent encodes GRPC request
func EncodeGrpcReqEvent(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Event)
	return req, nil
}

// DecodeGrpcReqEvent decodes GRPC request
func DecodeGrpcReqEvent(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Event)
	return req, nil
}

// EncodeGrpcRespEvent encodes GRC response
func EncodeGrpcRespEvent(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespEvent decodes GRPC response
func DecodeGrpcRespEvent(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPEventAttributes(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPEventAttributes(_ context.Context, r *http.Request) (interface{}, error) {
	var req EventAttributes
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqEventAttributes encodes GRPC request
func EncodeGrpcReqEventAttributes(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventAttributes)
	return req, nil
}

// DecodeGrpcReqEventAttributes decodes GRPC request
func DecodeGrpcReqEventAttributes(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventAttributes)
	return req, nil
}

// EncodeGrpcRespEventAttributes encodes GRC response
func EncodeGrpcRespEventAttributes(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespEventAttributes decodes GRPC response
func DecodeGrpcRespEventAttributes(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPEventExport(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPEventExport(_ context.Context, r *http.Request) (interface{}, error) {
	var req EventExport
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqEventExport encodes GRPC request
func EncodeGrpcReqEventExport(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventExport)
	return req, nil
}

// DecodeGrpcReqEventExport decodes GRPC request
func DecodeGrpcReqEventExport(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventExport)
	return req, nil
}

// EncodeGrpcRespEventExport encodes GRC response
func EncodeGrpcRespEventExport(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespEventExport decodes GRPC response
func DecodeGrpcRespEventExport(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPEventList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPEventList(_ context.Context, r *http.Request) (interface{}, error) {
	var req EventList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqEventList encodes GRPC request
func EncodeGrpcReqEventList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventList)
	return req, nil
}

// DecodeGrpcReqEventList decodes GRPC request
func DecodeGrpcReqEventList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventList)
	return req, nil
}

// EncodeGrpcRespEventList encodes GRC response
func EncodeGrpcRespEventList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespEventList decodes GRPC response
func DecodeGrpcRespEventList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPEventPolicy(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPEventPolicy(_ context.Context, r *http.Request) (interface{}, error) {
	var req EventPolicy
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqEventPolicy encodes GRPC request
func EncodeGrpcReqEventPolicy(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventPolicy)
	return req, nil
}

// DecodeGrpcReqEventPolicy decodes GRPC request
func DecodeGrpcReqEventPolicy(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventPolicy)
	return req, nil
}

// EncodeGrpcRespEventPolicy encodes GRC response
func EncodeGrpcRespEventPolicy(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespEventPolicy decodes GRPC response
func DecodeGrpcRespEventPolicy(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPEventPolicyList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPEventPolicyList(_ context.Context, r *http.Request) (interface{}, error) {
	var req EventPolicyList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqEventPolicyList encodes GRPC request
func EncodeGrpcReqEventPolicyList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventPolicyList)
	return req, nil
}

// DecodeGrpcReqEventPolicyList decodes GRPC request
func DecodeGrpcReqEventPolicyList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventPolicyList)
	return req, nil
}

// EncodeGrpcRespEventPolicyList endodes the GRPC response
func EncodeGrpcRespEventPolicyList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespEventPolicyList decodes the GRPC response
func DecodeGrpcRespEventPolicyList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPEventPolicySpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPEventPolicySpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req EventPolicySpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqEventPolicySpec encodes GRPC request
func EncodeGrpcReqEventPolicySpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventPolicySpec)
	return req, nil
}

// DecodeGrpcReqEventPolicySpec decodes GRPC request
func DecodeGrpcReqEventPolicySpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventPolicySpec)
	return req, nil
}

// EncodeGrpcRespEventPolicySpec encodes GRC response
func EncodeGrpcRespEventPolicySpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespEventPolicySpec decodes GRPC response
func DecodeGrpcRespEventPolicySpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPEventPolicyStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPEventPolicyStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req EventPolicyStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqEventPolicyStatus encodes GRPC request
func EncodeGrpcReqEventPolicyStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventPolicyStatus)
	return req, nil
}

// DecodeGrpcReqEventPolicyStatus decodes GRPC request
func DecodeGrpcReqEventPolicyStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventPolicyStatus)
	return req, nil
}

// EncodeGrpcRespEventPolicyStatus encodes GRC response
func EncodeGrpcRespEventPolicyStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespEventPolicyStatus decodes GRPC response
func DecodeGrpcRespEventPolicyStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPEventSource(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPEventSource(_ context.Context, r *http.Request) (interface{}, error) {
	var req EventSource
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqEventSource encodes GRPC request
func EncodeGrpcReqEventSource(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventSource)
	return req, nil
}

// DecodeGrpcReqEventSource decodes GRPC request
func DecodeGrpcReqEventSource(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EventSource)
	return req, nil
}

// EncodeGrpcRespEventSource encodes GRC response
func EncodeGrpcRespEventSource(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespEventSource decodes GRPC response
func DecodeGrpcRespEventSource(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}
