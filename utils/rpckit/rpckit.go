// gRPC wrapper library

package rpckit

import (
	"crypto/tls"
	"crypto/x509"
	"errors"
	"io/ioutil"
	"net"
	"time"

	log "github.com/Sirupsen/logrus"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
)

// Middleware is RPC middleware interface
type Middleware interface {
	ReqInterceptor(ctx context.Context, role string, method string, req interface{}) context.Context
	RespInterceptor(ctx context.Context, role string, method string, req, reply interface{}, err error) context.Context
}

// RPCServer contains RPC server state
type RPCServer struct {
	GrpcServer  *grpc.Server      // gRPC server.
	listenURL   string            // URL where this server is listening
	certFile    string            // certificate filename
	keyFile     string            // private key file name
	caFile      string            // Root CA file
	listener    net.Listener      // listener
	stats       *statsMiddleware  // Stats middleware for the server instance
	tracer      *tracerMiddleware // Tracer middleware for the server
	middlewares []Middleware      // list of middlewares
}

// RPCClient contains RPC client definitions
type RPCClient struct {
	ClientConn  *grpc.ClientConn  // gRPC connection
	remoteURL   string            // URL we are connecting to
	certFile    string            // certificate file name
	keyFile     string            // private key file name
	caFile      string            // Root CA file
	stats       *statsMiddleware  // Stats middleware for the client instance
	tracer      *tracerMiddleware // Tracer middleware for the client
	middlewares []Middleware      // list of middlewares
}

// global middlewares that will intercept all client/server calls
var globalMiddlewares []Middleware

// NewRPCServer returns a gRPC server listening on a local port
func NewRPCServer(srvName, listenURL, certFile, keyFile, caFile string, middlewares ...Middleware) (*RPCServer, error) {
	var server *grpc.Server

	// some error checking
	if listenURL == "" {
		log.Errorf("Requires a URL to listen")
		return nil, errors.New("Requires a URL to listen")
	}
	// Make sure both certFile and keyFile are specified or none
	if (certFile == "" && keyFile != "") || (certFile != "" && keyFile == "") {
		log.Errorf("Requires both cert file and key file")
		return nil, errors.New("Requires both cert file and key file")
	}

	// Start a listener
	lis, err := net.Listen("tcp", listenURL)
	if err != nil {
		log.Errorf("failed to listen to %s: Err %v", listenURL, err)
		return nil, err
	}

	// Create the RPC server instance
	rpcServer := &RPCServer{
		listenURL:   listenURL,
		listener:    lis,
		certFile:    certFile,
		keyFile:     keyFile,
		caFile:      caFile,
		stats:       newStatsMiddleware(),
		tracer:      newTracerMiddleware(srvName),
		middlewares: append(globalMiddlewares, middlewares...),
	}
	// add the stats middleware
	rpcServer.middlewares = append(rpcServer.middlewares, rpcServer.stats)
	rpcServer.middlewares = append(rpcServer.middlewares, rpcServer.tracer)

	// start new grpc server
	if certFile != "" && keyFile != "" && caFile != "" {
		certificate, err := tls.LoadX509KeyPair(certFile, keyFile)
		if err != nil {
			log.Errorf("Error loading tls keypairs(%s/%s). Err: %v", certFile, keyFile, err)
			return nil, err
		}

		certPool := x509.NewCertPool()
		bs, err := ioutil.ReadFile(caFile)
		if err != nil {
			log.Errorf("failed to read client ca cert: %s", err.Error())
			return nil, err
		}

		ok := certPool.AppendCertsFromPEM(bs)
		if !ok {
			log.Fatalf("failed to append client certs: %s", err.Error())
		}

		tlsConfig := &tls.Config{
			ClientAuth:   tls.RequireAndVerifyClientCert,
			Certificates: []tls.Certificate{certificate},
			ClientCAs:    certPool,
		}

		// start the server with TLS credentials
		server = grpc.NewServer(grpc.Creds(credentials.NewTLS(tlsConfig)),
			grpc.UnaryInterceptor(rpcServerUnaryInterceptor(rpcServer)),
			grpc.StreamInterceptor(rpcServerStreamInterceptor(rpcServer)))
	} else {
		server = grpc.NewServer(grpc.UnaryInterceptor(rpcServerUnaryInterceptor(rpcServer)),
			grpc.StreamInterceptor(rpcServerStreamInterceptor(rpcServer)))
	}

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

// Stop stops grpc server and closes the listener
func (srv *RPCServer) Stop() error {
	// stop the server
	srv.GrpcServer.Stop()

	// close the socket listener
	return srv.listener.Close()
}

// NewRPCClient returns an RPC client to a remote server
func NewRPCClient(srvName, remoteURL, certFile, keyFile, caFile string, middlewares ...Middleware) (*RPCClient, error) {
	var opts grpc.DialOption

	// create RPC client instance
	rpcClient := &RPCClient{
		remoteURL:   remoteURL,
		certFile:    certFile,
		keyFile:     keyFile,
		caFile:      caFile,
		stats:       newStatsMiddleware(),
		tracer:      newTracerMiddleware(srvName),
		middlewares: append(globalMiddlewares, middlewares...),
	}
	// add stats & tracer middleware
	rpcClient.middlewares = append(rpcClient.middlewares, rpcClient.stats)
	rpcClient.middlewares = append(rpcClient.middlewares, rpcClient.tracer)

	// Get credentials
	if certFile != "" {
		certificate, err := tls.LoadX509KeyPair(certFile, keyFile)
		if err != nil {
			log.Errorf("failed to read cert and key files(%s/%s): Err: %v", certFile, keyFile, err)
			return nil, err
		}

		certPool := x509.NewCertPool()
		bs, err := ioutil.ReadFile(caFile)
		if err != nil {
			log.Errorf("failed to read ca cert: %s", err.Error())
			return nil, err
		}

		ok := certPool.AppendCertsFromPEM(bs)
		if !ok {
			log.Fatalf("failed to append certs: %s", err.Error())
		}

		transportCreds := credentials.NewTLS(&tls.Config{
			ServerName:   "grpc.local",
			Certificates: []tls.Certificate{certificate},
			RootCAs:      certPool,
		})

		opts = grpc.WithTransportCredentials(transportCreds)
	} else {
		opts = grpc.WithInsecure()
	}

	// Set up a connection to the server.
	conn, err := grpc.Dial(remoteURL, grpc.WithBlock(), grpc.WithTimeout(time.Second*3), opts,
		grpc.WithUnaryInterceptor(rpcClientUnaryInterceptor(rpcClient)),
		grpc.WithStreamInterceptor(rpcClientStreamInterceptor(rpcClient)))
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

	// Get credentials
	if c.certFile != "" {
		certificate, err := tls.LoadX509KeyPair(c.certFile, c.keyFile)
		if err != nil {
			log.Fatalf("failed to read cert and key files(%s/%s): Err: %v", c.certFile, c.keyFile, err)
		}

		certPool := x509.NewCertPool()
		bs, err := ioutil.ReadFile(c.caFile)
		if err != nil {
			log.Fatalf("failed to read ca cert: %s", err.Error())
		}

		ok := certPool.AppendCertsFromPEM(bs)
		if !ok {
			log.Fatalf("failed to append certs: %s", err.Error())
		}

		transportCreds := credentials.NewTLS(&tls.Config{
			ServerName:   "grpc.local",
			Certificates: []tls.Certificate{certificate},
			RootCAs:      certPool,
		})

		opts = grpc.WithTransportCredentials(transportCreds)
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

// SetGlobalMiddlewares sets the global middlewares, useful for testing
func SetGlobalMiddlewares(middlewares []Middleware) {
	globalMiddlewares = middlewares
}

// Library initialization
func init() {
	// initialize global middlewares
	globalMiddlewares = []Middleware{newLogMiddleware()}
}
