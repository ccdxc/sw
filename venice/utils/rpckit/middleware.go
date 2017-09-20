// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpckit

import (
	"fmt"
	"sync"

	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/venice/utils/log"
)

// middleware callback roles
const (
	RoleClient = "Client"
	RoleServer = "Server"
)

// rpcServerUnaryInterceptor returns an intercept handler for unary rpc calls
func rpcServerUnaryInterceptor(rpcServer *RPCServer) grpc.UnaryServerInterceptor {
	return func(ctx context.Context, req interface{}, info *grpc.UnaryServerInfo,
		handler grpc.UnaryHandler) (interface{}, error) {

		// call all the request middlewares
		for _, m := range rpcServer.middlewares {
			ctx = m.ReqInterceptor(ctx, RoleServer, rpcServer.mysvcName, info.FullMethod, req)
		}

		// finally call the handler
		resp, err := handler(ctx, req)

		// call all response middlewares
		for _, m := range rpcServer.middlewares {
			ctx = m.RespInterceptor(ctx, RoleServer, rpcServer.mysvcName, info.FullMethod, req, resp, err)
		}

		return resp, err
	}
}

// rpcServerStreamInterceptor  is the server intercept handler for stream RPCs
func rpcServerStreamInterceptor(rpcServer *RPCServer) grpc.StreamServerInterceptor {
	return func(srv interface{}, ss grpc.ServerStream, info *grpc.StreamServerInfo, handler grpc.StreamHandler) error {
		// call all the request middlewares
		ctx := ss.Context()
		for _, m := range rpcServer.middlewares {
			ctx = m.ReqInterceptor(ctx, RoleServer, rpcServer.mysvcName, info.FullMethod, struct{}{})
		}

		return handler(srv, ss)
	}
}

// rpcClientUnaryInterceptor intercepts the client rpc calls
func rpcClientUnaryInterceptor(rpcClient *RPCClient) grpc.UnaryClientInterceptor {
	return func(ctx context.Context, method string, req, reply interface{}, cc *grpc.ClientConn, invoker grpc.UnaryInvoker, opts ...grpc.CallOption) error {
		// call all the request middlewares
		for _, m := range rpcClient.middlewares {
			ctx = m.ReqInterceptor(ctx, RoleClient, rpcClient.mysvcName, method, req)
		}

		// finally, call the invoker
		err := invoker(ctx, method, req, reply, cc, opts...)

		// call all the response middlewares
		for _, m := range rpcClient.middlewares {
			ctx = m.RespInterceptor(ctx, RoleClient, rpcClient.mysvcName, method, req, reply, err)
		}

		return err
	}
}

// rpcClientStreamInterceptor intercepts client rpc calls
// FIXME: to be implemented
func rpcClientStreamInterceptor(rpcClient *RPCClient) grpc.StreamClientInterceptor {
	return func(ctx context.Context, desc *grpc.StreamDesc, cc *grpc.ClientConn, method string, streamer grpc.Streamer, opts ...grpc.CallOption) (grpc.ClientStream, error) {
		return streamer(ctx, desc, cc, method, opts...)
	}
}

// Logging middleware
type logMiddleware struct {
	// empty struct
}

// newLogMiddleware returns a Logging middleware
func newLogMiddleware() *logMiddleware {
	return &logMiddleware{}
}

// ReqInterceptor implements request interception
func (l *logMiddleware) ReqInterceptor(ctx context.Context, role, mysvcName, method string, req interface{}) context.Context {
	switch role {
	case RoleClient:
		log.Infof("Client %s Making RPC request: %s() Req: {%+v}", mysvcName, method, req)
	case RoleServer:
		log.Infof("Server %s received RPC: %s() Req: {%+v}", mysvcName, method, req)
	}

	return ctx
}

// RespInterceptor implements response interception
func (l *logMiddleware) RespInterceptor(ctx context.Context, role, mysvcName, method string, req, reply interface{}, err error) context.Context {
	switch role {
	case RoleClient:
		log.Infof("Client %s received RPC response: %s() Resp: {%+v}, error: %v", mysvcName, method, reply, err)
	case RoleServer:
		log.Infof("Server %s returning RPC response: %s() Resp: {%+v}, error: %v", mysvcName, method, reply, err)
	}

	return ctx
}

// Stats middleware
type statsMiddleware struct {
	lock           sync.Mutex                  // RW lock for stats object
	rpcStats       map[string]int64            // stats for each RPC call
	rpcClientStats map[string]map[string]int64 // stats for each RPC call, by client (valid only on server)
}

// newStatsMiddleware returns a new stats middleware
func newStatsMiddleware(svcName string) *statsMiddleware {
	return &statsMiddleware{
		rpcStats:       make(map[string]int64),
		rpcClientStats: make(map[string]map[string]int64),
	}
}

// ReqInterceptor implements request interception
func (s *statsMiddleware) ReqInterceptor(ctx context.Context, role, mysvcName, method string, req interface{}) context.Context {
	// lock the object
	s.lock.Lock()
	defer s.lock.Unlock()

	switch role {
	case RoleClient:
		// get existing metadata
		md, ok := metadata.FromContext(ctx)
		if !ok {
			md = metadata.New(nil)
		} else {
			md = md.Copy()
		}
		newMd := metadata.Join(md, metadata.Pairs("cname", mysvcName))
		ctx = metadata.NewContext(ctx, newMd)
	case RoleServer:
		// get grpc metadata
		md, ok := metadata.FromContext(ctx)
		if !ok {
			break
		}
		clientNames, ok := md["cname"]
		if !ok {
			break
		}
		methodStats, ok := s.rpcClientStats[method]
		if !ok {
			methodStats = make(map[string]int64)
			s.rpcClientStats[method] = methodStats
		}
		methodStats[clientNames[0]]++
	}

	// increment stats
	s.rpcStats[fmt.Sprintf("%s-%s:Req", role, method)]++

	return ctx
}

// RespInterceptor handles responses
func (s *statsMiddleware) RespInterceptor(ctx context.Context, role, mysvcName, method string, req, reply interface{}, err error) context.Context {
	// lock the object
	s.lock.Lock()
	defer s.lock.Unlock()

	// increment response stats
	s.rpcStats[fmt.Sprintf("%s-%s:Resp", role, method)]++

	// count RPC error responses
	if err != nil {
		s.rpcStats[fmt.Sprintf("%s-%s:Resp:Error", role, method)]++
	}

	return ctx
}
