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
	"google.golang.org/grpc/keepalive"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	defaultMaxMsgSize = 50 * 1024 * 1024
	// clientKeepaliveTime is the interval at which rpc clients send
	// keepalive messages to the server. This is enabled to protect watches
	// from hanging on a server failure. The default timeout on keepalives
	// is 20 seconds.
	clientKeepaliveTime = 5 * time.Second
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

// ClearStats clears all RPC statistics.
func ClearStats() {
	singletonMap = singletonMap.Init()
}

// These are options that are common to both client and server
type options struct {
	stats             *statsMiddleware  // Stats middleware for the server instance
	tracer            *tracerMiddleware // Tracer middleware for the server
	middlewares       []Middleware      // list of middlewares
	enableTracer      bool              // option to enable tracer middleware
	enableLogger      bool              // option to enable logging middleware
	enableStats       bool              // option to enable Stats middleware
	maxMsgSize        int               // Max message size allowed on the connections
	tlsProvider       TLSProvider       // provides TLS parameters for all RPC clients and servers
	customTLSProvider bool              // a flag that indicates that user has explicitly passed in a TLS provider
	balancer          grpc.Balancer     // Load balance RPCs between available servers (client option)
	remoteServerName  string            // The name of the server that client is connecting to (client option)
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
		o.customTLSProvider = true
	}
}

// WithTracerEnabled specifies whether the tracer should be enabled
func WithTracerEnabled(enabled bool) Option {
	return func(o *options) {
		o.enableTracer = enabled
	}
}

// WithMaxMsgSize specifies the max message size permitted on the transport
func WithMaxMsgSize(size int) Option {
	return func(o *options) {
		o.maxMsgSize = size
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

// WithRemoteServerName passes the name of the server that client is connecting to
// This name is expected to appear as a SAN in the server certificate if TLS is enabled.
// If it is not provided, remoteURL is used in its place.
func WithRemoteServerName(name string) Option {
	return func(o *options) {
		o.remoteServerName = name
	}
}

func defaultOptions(mysvcName, role string) *options {
	return &options{
		stats:        newStatsMiddleware(mysvcName, role),
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
		DoneCh:    make(chan error, 2),
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
		rpcServer.tracer = newTracerMiddleware(mysvcName)
		rpcServer.middlewares = append(rpcServer.middlewares, rpcServer.tracer) // tracing
	}

	grpcOpts := []grpc.ServerOption{
		grpc.UnaryInterceptor(rpcServerUnaryInterceptor(rpcServer)),
		grpc.StreamInterceptor(rpcServerStreamInterceptor(rpcServer)),
		grpc.KeepaliveEnforcementPolicy(keepalive.EnforcementPolicy{MinTime: clientKeepaliveTime}),
	}

	// Use default TLS provider unless user has passed in one
	if rpcServer.options.customTLSProvider == false {
		tlsProvider, err := GetDefaultTLSProvider(mysvcName)
		if err != nil {
			log.Errorf("Failed to instantiate TLS provider. Server name: %s, Err %v", mysvcName, err)
			return nil, err
		}
		rpcServer.tlsProvider = tlsProvider
	}
	log.Infof("Creating new server, name: %v listenURL: %v, TLS: %v", mysvcName, listenURL, rpcServer.tlsProvider != nil)

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
		log.Errorf("Error creating grpc server")
		return nil, fmt.Errorf("Error creating grpc server")
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

	// release resources held by TLS provider, if any
	if srv.tlsProvider != nil {
		srv.tlsProvider.Close()
		srv.tlsProvider = nil
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

	// set remoteServerName to remoteURL if it was not provided
	if rpcClient.options.remoteServerName == "" {
		rpcClient.options.remoteServerName = remoteURL
	}
	serverName := rpcClient.options.remoteServerName

	// add default middlewares
	if rpcClient.enableStats {
		rpcClient.middlewares = append(rpcClient.middlewares, rpcClient.stats) //stats
	}
	if rpcClient.enableLogger {
		rpcClient.middlewares = append(rpcClient.middlewares, newLogMiddleware()) // logging
	}
	if rpcClient.enableTracer {
		rpcClient.tracer = newTracerMiddleware(mysvcName)
		rpcClient.middlewares = append(rpcClient.middlewares, rpcClient.tracer) // tracing
	}

	// Use default TLS provider unless user has passed in one
	if rpcClient.options.customTLSProvider == false {
		tlsProvider, err := GetDefaultTLSProvider(mysvcName)
		if err != nil {
			log.Errorf("Failed to instantiate TLS provider. Server name: %s, Err %v", mysvcName, err)
			return nil, err
		}
		rpcClient.tlsProvider = tlsProvider
	}
	log.Infof("Service %v connecting to remoteURL: %v, TLS: %v", mysvcName, remoteURL, rpcClient.tlsProvider != nil)

	// Get credentials
	if rpcClient.tlsProvider != nil {
		tlsOpt, terr := rpcClient.tlsProvider.GetDialOptions(serverName)
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

	if rpcClient.maxMsgSize != 0 {
		grpcOpts = append(grpcOpts, grpc.WithMaxMsgSize(rpcClient.maxMsgSize))
	} else {
		grpcOpts = append(grpcOpts, grpc.WithMaxMsgSize(defaultMaxMsgSize))
	}

	grpcOpts = append(grpcOpts, grpc.WithBlock(), grpc.WithTimeout(time.Second*3),
		grpc.WithKeepaliveParams(keepalive.ClientParameters{Time: clientKeepaliveTime}),
		grpc.WithUnaryInterceptor(rpcClientUnaryInterceptor(rpcClient)),
		grpc.WithStreamInterceptor(rpcClientStreamInterceptor(rpcClient)))

	// Set up a connection to the server.
	conn, err := grpc.Dial(remoteURL, grpcOpts...)
	if err != nil {
		log.Errorf("Service %v could not connect to service %s, URL: %v, err: %v", mysvcName, serverName, remoteURL, err)
		return nil, err
	}

	rpcClient.ClientConn = conn

	log.Infof("Client %s Connected to %s at %s", mysvcName, serverName, remoteURL)

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
		opts, err = c.tlsProvider.GetDialOptions(c.options.remoteServerName)
		if err != nil {
			log.Errorf("Failed to get dial options for server %v. Err: %v", c.remoteURL, err)
			return err
		}
	} else {
		opts = grpc.WithInsecure()
	}

	// Set up a connection to the server.
	conn, err := grpc.Dial(c.remoteURL, opts, grpc.WithBlock(), grpc.WithTimeout(time.Second*3),
		grpc.WithKeepaliveParams(keepalive.ClientParameters{Time: clientKeepaliveTime}),
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
	// close the connection
	if c.ClientConn != nil {
		c.ClientConn.Close()
		c.ClientConn = nil
	}

	// do not invoke Close() on the TLS provider because it is shared

	if c.balancer != nil {
		c.balancer.Close()
		c.balancer = nil
	}

	return nil
}
