// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// gRPC wrapper library that provides additional functionality for Pensando
// cluster clients and server (TLS, service-discovery, load-balancing, etc.)

package rpckit

import (
	"errors"
	"fmt"
	"net"
	"time"

	"github.com/pensando/sw/utils/log"
	"google.golang.org/grpc"
)

// These are options that are common to both client and server
type options struct {
	stats        *statsMiddleware  // Stats middleware for the server instance
	tracer       *tracerMiddleware // Tracer middleware for the server
	middlewares  []Middleware      // list of middlewares
	enableTracer bool              // option to enable tracer middleware
	enableLogger bool              // option to enable logging middleware
	tlsProvider  TLSProvider       // provides TLS parameters for all RPC clients and servers
	balancer     grpc.Balancer     // Load balance RPCs between available servers (client option)
}

// RPCServer contains RPC server state
type RPCServer struct {
	GrpcServer *grpc.Server // gRPC server.
	listenURL  string       // URL where this server is listening
	listener   net.Listener // listener
	options
}

// RPCClient contains RPC client definitions
type RPCClient struct {
	ClientConn *grpc.ClientConn // gRPC connection
	srvName    string           // the server we are connecting to
	remoteURL  string           // URL we are connecting to
	options
}

// Option fills the optional params for RPCClient and RPCServer
type Option func(opt *options)

// WithTLSProvider passes a provider for gRPC TLS options
func WithTLSProvider(provider TLSProvider) Option {
	return func(o *options) {
		o.tlsProvider = provider
	}
}

// WithTracerEnabled specifies whether the tracer should be enabled
func WithTracerEnabled(enabled bool) Option {
	return func(o *options) {
		o.enableTracer = enabled
	}
}

// WithLoggerEnabled specifies whether the logger should be enabled
func WithLoggerEnabled(enabled bool) Option {
	return func(o *options) {
		o.enableLogger = enabled
	}
}

// WithMiddleware passes a provider for gRPC TLS options
func WithMiddleware(m Middleware) Option {
	return func(o *options) {
		o.middlewares = append(o.middlewares, m)
	}
}

// WithBalancer passes a balancer for gRPC Load balancing
func WithBalancer(b grpc.Balancer) Option {
	return func(o *options) {
		o.balancer = b
	}
}

func defaultOptions(srvName string) *options {
	return &options{
		stats:        newStatsMiddleware(),
		tracer:       newTracerMiddleware(srvName),
		enableTracer: false,
		enableLogger: true,
	}
}

// NewRPCServer returns a gRPC server listening on a local port
func NewRPCServer(srvName, listenURL string, opts ...Option) (*RPCServer, error) {
	var server *grpc.Server

	// some error checking
	if listenURL == "" {
		log.Errorf("Requires a URL to listen")
		return nil, errors.New("Requires a URL to listen")
	}

	// Start a listener
	lis, err := net.Listen("tcp", listenURL)
	if err != nil {
		log.Errorf("failed to listen to %s: Err %v", listenURL, err)
		return nil, err
	}
	// If started with ":0", listenURL needs to be updated.
	listenURL = lis.Addr().String()

	// Create the RPC server instance with default values
	rpcServer := &RPCServer{
		listenURL: listenURL,
		listener:  lis,
		options:   *defaultOptions(srvName),
	}

	// add custom options
	for _, o := range opts {
		if o != nil {
			o(&rpcServer.options)
		}
	}

	// add default middlewares
	rpcServer.middlewares = append(rpcServer.middlewares, rpcServer.stats)    // stats
	rpcServer.middlewares = append(rpcServer.middlewares, newLogMiddleware()) // logging
	if rpcServer.enableTracer {
		rpcServer.middlewares = append(rpcServer.middlewares, rpcServer.tracer) // tracing
	}

	grpcOpts := []grpc.ServerOption{
		grpc.UnaryInterceptor(rpcServerUnaryInterceptor(rpcServer)),
		grpc.StreamInterceptor(rpcServerStreamInterceptor(rpcServer)),
	}

	// get TLS options
	if rpcServer.tlsProvider != nil {
		tlsOptions, err := rpcServer.tlsProvider.GetServerOptions(srvName)
		if err != nil {
			log.Errorf("Failed to retrieve server TLS options. Server name: %s, Err %v", srvName, err)
			return nil, err
		}
		grpcOpts = append(grpcOpts, tlsOptions)
	}

	// start the server
	server = grpc.NewServer(grpcOpts...)
	// start service requests
	go server.Serve(lis)

	log.Infof("gRpc server Listening on %s", listenURL)

	// save the grpc server instance
	rpcServer.GrpcServer = server

	return rpcServer, nil
}

// GetRPCStats returns RPC stats for the server
func (srv *RPCServer) GetRPCStats() map[string]int64 {
	return srv.stats.rpcStats
}

// GetListenURL returns the listen URL for the server (for testing).
func (srv *RPCServer) GetListenURL() string {
	return srv.listenURL
}

// Stop stops grpc server and closes the listener
func (srv *RPCServer) Stop() error {
	// stop the server
	srv.GrpcServer.Stop()

	// close the socket listener
	return srv.listener.Close()
}

// NewRPCClient returns an RPC client to a remote server
//
// srvName   - identifier of the client, used in logging
// remoteURL - either <host:port> or <service-name>. If <service-name> is
//             specified, a balancer should be involved to resolve the name.
//             At this time, the balancer is explicitly passed. At a later
//             time, there will be an implicit balancer created.
func NewRPCClient(srvName, remoteURL string, opts ...Option) (*RPCClient, error) {
	grpcOpts := make([]grpc.DialOption, 0)

	// some error checking
	if remoteURL == "" {
		log.Errorf("Requires a remote URL to dial")
		return nil, errors.New("Requires a remote URL to dial")
	}

	// create RPC client instance
	rpcClient := &RPCClient{
		srvName:   srvName,
		remoteURL: remoteURL,
		options:   *defaultOptions(srvName),
	}

	// add custom options
	for _, o := range opts {
		if o != nil {
			o(&rpcClient.options)
		}
	}

	serviceTarget := false // need for a balancer.
	_, _, err := net.SplitHostPort(remoteURL)
	if err != nil {
		// Not a URL, must provide a balancer.
		// TODO: Create a balancer that uses a resolver that points to VIP:<CMD port>
		if rpcClient.balancer == nil {
			return nil, fmt.Errorf("Requires a balancer to resolve %v", remoteURL)
		}
		serviceTarget = true
	}

	// add default middlewares
	rpcClient.middlewares = append(rpcClient.middlewares, rpcClient.stats)    //stats
	rpcClient.middlewares = append(rpcClient.middlewares, newLogMiddleware()) // logging
	if rpcClient.enableTracer {
		rpcClient.middlewares = append(rpcClient.middlewares, rpcClient.tracer) // tracing
	}

	// Get credentials
	if rpcClient.tlsProvider != nil {
		tlsOpt, err := rpcClient.tlsProvider.GetDialOptions(srvName)
		if err != nil {
			log.Errorf("Failed to get dial options for server %v. Err: %v", srvName, err)
			return nil, err
		}
		grpcOpts = append(grpcOpts, tlsOpt)
	} else {
		grpcOpts = append(grpcOpts, grpc.WithInsecure())
	}

	// For service targets, use the balancer.
	if serviceTarget {
		grpcOpts = append(grpcOpts, grpc.WithBalancer(rpcClient.balancer))
	}

	grpcOpts = append(grpcOpts, grpc.WithBlock(), grpc.WithTimeout(time.Second*3),
		grpc.WithUnaryInterceptor(rpcClientUnaryInterceptor(rpcClient)),
		grpc.WithStreamInterceptor(rpcClientStreamInterceptor(rpcClient)))

	// Set up a connection to the server.
	conn, err := grpc.Dial(remoteURL, grpcOpts...)
	if err != nil {
		log.Errorf("could not connect to %s. Err: %v", remoteURL, err)
		return nil, err
	}

	rpcClient.ClientConn = conn

	log.Infof("Connected to %s", remoteURL)

	return rpcClient, nil
}

// Reconnect connects back to the remote URL
func (c *RPCClient) Reconnect() error {
	var opts grpc.DialOption

	// close old connection
	if c.ClientConn != nil {
		err := c.ClientConn.Close()
		if err != nil {
			log.Warnf("Error closing old connection during reconnect. Err: %v", err)
		}
		c.ClientConn = nil
	}

	var err error
	// Get credentials
	if c.tlsProvider != nil {
		opts, err = c.tlsProvider.GetDialOptions(c.srvName)
		if err != nil {
			log.Errorf("Failed to get dial options for server %v. Err: %v", c.srvName, err)
			return err
		}
	} else {
		opts = grpc.WithInsecure()
	}

	// Set up a connection to the server.
	conn, err := grpc.Dial(c.remoteURL, opts, grpc.WithBlock(), grpc.WithTimeout(time.Second*3),
		grpc.WithUnaryInterceptor(rpcClientUnaryInterceptor(c)),
		grpc.WithStreamInterceptor(rpcClientStreamInterceptor(c)))
	if err != nil {
		log.Errorf("could not connect to %s. Err: %v", c.remoteURL, err)
		return err
	}

	// save new client
	c.ClientConn = conn

	return nil
}

// GetRPCStats returns RPC stats for the client
func (c *RPCClient) GetRPCStats() map[string]int64 {
	return c.stats.rpcStats
}

// Close closes client connection
func (c *RPCClient) Close() error {
	return c.ClientConn.Close()
}
