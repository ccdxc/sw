package rpckit

import (
	"fmt"

	log "github.com/Sirupsen/logrus"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

// rpcServerUnaryInterceptor returns an intercept handler for unary rpc calls
func rpcServerUnaryInterceptor(rpcServer *RPCServer) grpc.UnaryServerInterceptor {
	return func(ctx context.Context, req interface{}, info *grpc.UnaryServerInfo,
		handler grpc.UnaryHandler) (interface{}, error) {

		// call all the request middlewares
		for _, m := range rpcServer.middlewares {
			m.ReqInterceptor(ctx, "Server", info.FullMethod, req)
		}

		// finally call the handler
		resp, err := handler(ctx, req)

		// call all response middlewares
		for _, m := range rpcServer.middlewares {
			m.RespInterceptor(ctx, "Server", info.FullMethod, req, resp, err)
		}

		return resp, err
	}
}

// rpcServerStreamInterceptor  is the server intercept handler for stream RPCs
// FIXME: to be implemented
func rpcServerStreamInterceptor(rpcServer *RPCServer) grpc.StreamServerInterceptor {
	return func(srv interface{}, ss grpc.ServerStream, info *grpc.StreamServerInfo, handler grpc.StreamHandler) error {
		return handler(srv, ss)
	}
}

// rpcClientUnaryInterceptor intercepts the client rpc calls
func rpcClientUnaryInterceptor(rpcClient *RPCClient) grpc.UnaryClientInterceptor {
	return func(ctx context.Context, method string, req, reply interface{}, cc *grpc.ClientConn, invoker grpc.UnaryInvoker, opts ...grpc.CallOption) error {
		// call all the request middlewares
		for _, m := range rpcClient.middlewares {
			m.ReqInterceptor(ctx, "Client", method, req)
		}

		// finally, call the invoker
		err := invoker(ctx, method, req, reply, cc, opts...)

		// call all the response middlewares
		for _, m := range rpcClient.middlewares {
			m.RespInterceptor(ctx, "Client", method, req, reply, err)
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
func (l *logMiddleware) ReqInterceptor(ctx context.Context, role string, method string, req interface{}) {
	switch role {
	case "Client":
		log.Infof("Client Making RPC request: %s() Req: {%+v}", method, req)
	case "Server":
		log.Infof("Server received RPC: %s() Req: {%+v}", method, req)
	}
}

// RespInterceptor implements response interception
func (l *logMiddleware) RespInterceptor(ctx context.Context, role string, method string, req, reply interface{}, err error) {
	switch role {
	case "Client":
		log.Infof("Client received RPC response: %s() Resp: {%+v}, error: %v", method, reply, err)
	case "Server":
		log.Infof("Server returning RPC response: %s() Resp: {%+v}, error: %v", method, reply, err)
	}
}

// Stats middleware
type statsMiddleware struct {
	rpcStats map[string]int64 // stats for each RPC call
}

// newStatsMiddleware returns a new stats middleware
func newStatsMiddleware() *statsMiddleware {
	return &statsMiddleware{
		rpcStats: make(map[string]int64),
	}
}

// ReqInterceptor implements request interception
func (s *statsMiddleware) ReqInterceptor(ctx context.Context, role string, method string, req interface{}) {
	s.rpcStats[fmt.Sprintf("%s-%s:Req", role, method)]++
}

// RespInterceptor handles responses
func (s *statsMiddleware) RespInterceptor(ctx context.Context, role string, method string, req, reply interface{}, err error) {
	s.rpcStats[fmt.Sprintf("%s-%s:Resp", role, method)]++

	// count RPC error responses
	if err != nil {
		s.rpcStats[fmt.Sprintf("%s-%s:Resp:Error", role, method)]++
	}
}
