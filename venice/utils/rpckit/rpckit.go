// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// gRPC wrapper library that provides additional functionality for Pensando
// cluster clients and server (TLS, service-discovery, load-balancing, etc.)

package rpckit

import (
	"errors"
	"expvar"
	"fmt"
	"net"
	"sync"
	"time"

	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/utils/log"
)

// Singleton stats
var once sync.Once
var singletonMap *expvar.Map

func init() {
	once.Do(func() {
		singletonMap = expvar.NewMap("rpcstats")
	})
}

// Stats returns the map that keeps track of rpcstats.
func Stats() *expvar.Map {
	return singletonMap
}

// These are options that are common to both client and server
type options struct {
	stats        *statsMiddleware  // Stats middleware for the server instance
	tracer       *tracerMiddleware // Tracer middleware for the server
	middlewares  []Middleware      // list of middlewares
	enableTracer bool              // option to enable tracer middleware
	enableLogger bool              // option to enable logging middleware
	enableStats  bool              // option to enable Stats middleware
	tlsProvider  TLSProvider       // provides TLS parameters for all RPC clients and servers
	balancer     grpc.Balancer     // Load balance RPCs between available servers (client option)
}

// RPCServer contains RPC server state
type RPCServer struct {
	sync.Mutex
	GrpcServer *grpc.Server // gRPC server.
	mysvcName  string       // my service name
	listenURL  string       // URL where this server is listening
	listener   net.Listener // listener
	DoneCh     chan error   // Error Channel
	once       sync.Once
	options
}

// RPCClient contains RPC client definitions
type RPCClient struct {
	ClientConn *grpc.ClientConn // gRPC connection
	mysvcName  string           // my service name
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

// WithStatsEnabled specifies whether stats collection should be enabled
func WithStatsEnabled(enabled bool) Option {
	return func(o *options) {
		o.enableStats = enabled
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

func defaultOptions(mysvcName, role string) *options {
	return &options{
		stats:        newStatsMiddleware(mysvcName, role),
		tracer:       newTracerMiddleware(mysvcName),
		enableTracer: false,
		enableLogger: true,
		enableStats:  true,
	}
}

type tcpKeepAliveListener struct {
	*net.TCPListener
}

func (ln tcpKeepAliveListener) Accept() (c net.Conn, err error) {
	tc, err := ln.AcceptTCP()
	if err != nil {
		return
	}
	tc.SetKeepAlive(true)
	tc.SetKeepAlivePeriod(3 * time.Minute)
	return tc, nil
}

// NewRPCServer returns a gRPC server listening on a local port
func NewRPCServer(mysvcName, listenURL string, opts ...Option) (*RPCServer, error) {
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

	lis = tcpKeepAliveListener{lis.(*net.TCPListener)}

	// Create the RPC server instance with default values
	rpcServer := &RPCServer{
		mysvcName: mysvcName,
		listenURL: listenURL,
		listener:  ListenWrapper(lis),
		DoneCh:    make(chan error),
		options:   *defaultOptions(mysvcName, RoleServer),
	}

	// add custom options
	for _, o := range opts {
		if o != nil {
			o(&rpcServer.options)
		}
	}

	// add default middlewares
	if rpcServer.enableStats {
		rpcServer.middlewares = append(rpcServer.middlewares, rpcServer.stats) // stats
	}
	if rpcServer.enableLogger {
		rpcServer.middlewares = append(rpcServer.middlewares, newLogMiddleware()) // logging
	}
	if rpcServer.enableTracer {
		rpcServer.middlewares = append(rpcServer.middlewares, rpcServer.tracer) // tracing
	}

	grpcOpts := []grpc.ServerOption{
		grpc.UnaryInterceptor(rpcServerUnaryInterceptor(rpcServer)),
		grpc.StreamInterceptor(rpcServerStreamInterceptor(rpcServer)),
	}

	// get TLS options
	if rpcServer.tlsProvider != nil {
		tlsOptions, err := rpcServer.tlsProvider.GetServerOptions(mysvcName)
		if err != nil {
			log.Errorf("Failed to retrieve server TLS options. Server name: %s, Err %v", mysvcName, err)
			return nil, err
		}
		grpcOpts = append(grpcOpts, tlsOptions)
	}

	// save the grpc server instance
	server = grpc.NewServer(grpcOpts...)
	if server == nil {
		log.Fatalf("Error creating grpc server")
	}
	rpcServer.GrpcServer = server
	return rpcServer, nil
}

// GetListenURL returns the listen URL for the server (for testing).
func (srv *RPCServer) GetListenURL() string {
	return srv.listenURL
}

// Stop stops grpc server and closes the listener
func (srv *RPCServer) Stop() error {
	srv.Lock()
	defer srv.Unlock()

	// stop the server
	if srv.GrpcServer != nil {
		log.Infof("Stopping grpc server %s listening on %s", srv.mysvcName, srv.listenURL)
		srv.GrpcServer.Stop()
		srv.GrpcServer = nil
	}

	// close the socket listener
	return srv.listener.Close()
}

func (srv *RPCServer) run() {
	// start service requests
	go func() {
		srv.Lock()
		grpcServer := srv.GrpcServer
		listener := srv.listener
		if grpcServer != nil && listener != nil {
			log.Infof("gRpc server %s Listening on %s", srv.mysvcName, srv.listenURL)
			srv.Unlock()
			srv.DoneCh <- grpcServer.Serve(listener)
			return
		}
		srv.Unlock()
	}()
}

// Start serving on the RPC server.
func (srv *RPCServer) Start() {
	srv.once.Do(srv.run)
}

// NewRPCClient returns an RPC client to a remote server
//
// mysvcName   - identifier of the client, used in logging
// remoteURL - either <host:port> or <service-name>. If <service-name> is
//             specified, a balancer should be involved to resolve the name.
//             At this time, the balancer is explicitly passed. At a later
//             time, there will be an implicit balancer created.
func NewRPCClient(mysvcName, remoteURL string, opts ...Option) (*RPCClient, error) {
	grpcOpts := make([]grpc.DialOption, 0)

	// some error checking
	if remoteURL == "" {
		log.Errorf("Requires a remote URL to dial")
		return nil, errors.New("Requires a remote URL to dial")
	}

	// create RPC client instance
	rpcClient := &RPCClient{
		mysvcName: mysvcName,
		remoteURL: remoteURL,
		options:   *defaultOptions(mysvcName, RoleClient),
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
	if rpcClient.enableStats {
		rpcClient.middlewares = append(rpcClient.middlewares, rpcClient.stats) //stats
	}
	if rpcClient.enableLogger {
		rpcClient.middlewares = append(rpcClient.middlewares, newLogMiddleware()) // logging
	}
	if rpcClient.enableTracer {
		rpcClient.middlewares = append(rpcClient.middlewares, rpcClient.tracer) // tracing
	}

	// Get credentials
	if rpcClient.tlsProvider != nil {
		tlsOpt, terr := rpcClient.tlsProvider.GetDialOptions(mysvcName)
		if terr != nil {
			log.Errorf("Failed to get dial options for server %v. Err: %v", mysvcName, terr)
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

	log.Infof("Client %s Connected to %s", mysvcName, remoteURL)

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
		opts, err = c.tlsProvider.GetDialOptions(c.mysvcName)
		if err != nil {
			log.Errorf("Failed to get dial options for server %v. Err: %v", c.mysvcName, err)
			return err
		}
	} else {
		opts = grpc.WithInsecure()
	}

	// Set up a connection to the server.
	conn, err := grpc.Dial(c.remoteURL, opts, grpc.WithBlock(), grpc.WithTimeout(time.Second*5),
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

// Close closes client connection
func (c *RPCClient) Close() error {
	if c.ClientConn != nil {
		c.ClientConn.Close()
		c.ClientConn = nil
	}
	return nil
}
