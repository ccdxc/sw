// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package tokenauth is a auto generated package.
Input file: svc_tokenauth.proto
*/
package tokenauth

import (
	"context"

	"github.com/pensando/sw/api"
)

// Dummy definitions to suppress nonused warnings
var _ api.ObjectMeta

// ServiceTokenAuthV1Client  is the client interface for the service.
type ServiceTokenAuthV1Client interface {
	AutoWatchSvcTokenAuthV1(ctx context.Context, in *api.AggWatchOptions) (TokenAuthV1_AutoWatchSvcTokenAuthV1Client, error)

	GenerateNodeToken(ctx context.Context, t *NodeTokenRequest) (*NodeTokenResponse, error)
}

// ServiceTokenAuthV1Server is the server interface for the service.
type ServiceTokenAuthV1Server interface {
	AutoWatchSvcTokenAuthV1(in *api.AggWatchOptions, stream TokenAuthV1_AutoWatchSvcTokenAuthV1Server) error

	GenerateNodeToken(ctx context.Context, t NodeTokenRequest) (NodeTokenResponse, error)
}
