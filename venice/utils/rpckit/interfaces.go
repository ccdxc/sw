// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// Interfaces for rpckit Middleware and Providers

package rpckit

import (
	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

// Middleware is RPC middleware interface
type Middleware interface {
	ReqInterceptor(ctx context.Context, role, mysvcName, method string, req interface{}) context.Context
	RespInterceptor(ctx context.Context, role, mysvcName, method string, req, reply interface{}, err error) context.Context
}

// TLSProvider is the interface for components that provide TLS credentials for
// rpckit clients and servers
type TLSProvider interface {
	GetServerOptions(serverName string) (grpc.ServerOption, error)
	GetDialOptions(serverName string) (grpc.DialOption, error)
	Close()
}
