// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package tokenauth is a auto generated package.
Input file: svc_tokenauth.proto
*/
package tokenauth

import (
	"context"
	"encoding/json"
	"errors"
	"net/http"

	grpctransport "github.com/go-kit/kit/transport/grpc"
	oldcontext "golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/trace"
)

// Dummy definitions to suppress nonused warnings
var _ api.ObjectMeta

type grpcServerTokenAuthV1 struct {
	Endpoints EndpointsTokenAuthV1Server

	GenerateNodeTokenHdlr grpctransport.Handler
}

// MakeGRPCServerTokenAuthV1 creates a GRPC server for TokenAuthV1 service
func MakeGRPCServerTokenAuthV1(ctx context.Context, endpoints EndpointsTokenAuthV1Server, logger log.Logger) TokenAuthV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerTokenAuthV1{
		Endpoints: endpoints,
		GenerateNodeTokenHdlr: grpctransport.NewServer(
			endpoints.GenerateNodeTokenEndpoint,
			DecodeGrpcReqNodeTokenRequest,
			EncodeGrpcRespNodeTokenResponse,
			append(options, grpctransport.ServerBefore(trace.FromGRPCRequest("GenerateNodeToken", logger)))...,
		),
	}
}

func (s *grpcServerTokenAuthV1) GenerateNodeToken(ctx oldcontext.Context, req *NodeTokenRequest) (*NodeTokenResponse, error) {
	_, resp, err := s.GenerateNodeTokenHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respTokenAuthV1GenerateNodeToken).V
	return &r, resp.(respTokenAuthV1GenerateNodeToken).Err
}

func decodeHTTPrespTokenAuthV1GenerateNodeToken(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp NodeTokenResponse
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func (s *grpcServerTokenAuthV1) AutoWatchSvcTokenAuthV1(in *api.ListWatchOptions, stream TokenAuthV1_AutoWatchSvcTokenAuthV1Server) error {
	return errors.New("not implemented")
}
