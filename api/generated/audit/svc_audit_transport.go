// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package audit is a auto generated package.
Input file: svc_audit.proto
*/
package audit

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

type grpcServerAuditV1 struct {
	Endpoints EndpointsAuditV1Server

	GetEventHdlr grpctransport.Handler
}

// MakeGRPCServerAuditV1 creates a GRPC server for AuditV1 service
func MakeGRPCServerAuditV1(ctx context.Context, endpoints EndpointsAuditV1Server, logger log.Logger) AuditV1Server {
	return &grpcServerAuditV1{
		Endpoints: endpoints,
		GetEventHdlr: grpctransport.NewServer(
			endpoints.GetEventEndpoint,
			DecodeGrpcReqAuditEventRequest,
			EncodeGrpcRespAuditEvent,
			append([]grpctransport.ServerOption{grpctransport.ServerErrorLogger(logger), grpctransport.ServerBefore(recoverVersion)}, grpctransport.ServerBefore(trace.FromGRPCRequest("GetEvent", logger)))...,
		),
	}
}

func (s *grpcServerAuditV1) GetEvent(ctx oldcontext.Context, req *AuditEventRequest) (*AuditEvent, error) {
	_, resp, err := s.GetEventHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respAuditV1GetEvent).V
	return &r, resp.(respAuditV1GetEvent).Err
}

func decodeHTTPrespAuditV1GetEvent(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp AuditEvent
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerAuditV1) AutoWatchSvcAuditV1(in *api.AggWatchOptions, stream AuditV1_AutoWatchSvcAuditV1Server) error {
	return s.Endpoints.AutoWatchSvcAuditV1(in, stream)
}
