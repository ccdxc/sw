// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package certsproxy

import (
	"context"
	"crypto/x509"
	"fmt"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// CertsProxy is a component that receives certificate-signing requests and
// forwards them to a remote endpoint. The API that is exposed locally is the CMD API,
// so that rpckit does not need to be aware of the presence of the proxy.
// The local API is unauthenticated, whereas the connection towards the CMD endpoint
// uses the supplied TLS provider
type CertsProxy struct {
	// The URL where the proxy listens for requests
	listenURL string
	// the URL where where requests are forwarded to
	remoteURL string
	// the gRPC endpoint that exposes CMD APIs to clients
	rpcServer *rpckit.RPCServer
	// rpc client options
	rpcClientOptions []rpckit.Option
}

var (
	connDialTimeout   = 30 * time.Second
	connRetryInterval = 30 * time.Second
	connMaxRetries    = 5
)

// Open a connection to CMD Endpoint with retries
func (c *CertsProxy) getRPCClient() (*rpckit.RPCClient, error) {
	// We need to set FailFast=false because we create a new connection for each RPC.
	// With FailFast=true, if the connection is not ready, the RPC fails immediately,
	// but connection establishment proceeds in the background until canceled.
	// Connection establishment is expensive because of the TLS handshake, so we want
	// the RPC layer to wait for the connection to be ready and not produce spurious
	// failures.
	// We also want to use a fairly long timeout because if we cancel the connection
	// the goroutine that performs the handshake keeps working in the background.
	// That slows us down, making it more likely that we will timeout again (producing
	// more work) and never get to complete the handshake.
	rpcOptions := []rpckit.Option{rpckit.WithFailFastDialOption(false), rpckit.WithDialTimeout(connDialTimeout)}
	rpcOptions = append(rpcOptions, c.rpcClientOptions...)
	for i := 0; i <= connMaxRetries; i++ {
		rpcClient, err := rpckit.NewRPCClient("certsproxy", c.remoteURL, rpcOptions...)
		if err == nil {
			return rpcClient, nil
		}
		log.Infof("Failed to open CMD Connection to: %v, result: %v, retrying in %v", c.remoteURL, err, connRetryInterval)
		time.Sleep(connRetryInterval)
	}
	return nil, fmt.Errorf("Error opening CMD connection")
}

// SignCertificateRequest checks a CSR submitted by a client, signs it and returns a certificate
func (c *CertsProxy) SignCertificateRequest(ctx context.Context, req *certapi.CertificateSignReq) (*certapi.CertificateSignResp, error) {
	csr, err := x509.ParseCertificateRequest(req.GetCsr())
	if err != nil {
		log.Errorf("Received invalid certificate request, error: %v", err)
		return &certapi.CertificateSignResp{}, fmt.Errorf("Invalid CSR: %v", err)
	}
	err = csr.CheckSignature()
	if err != nil {
		log.Errorf("Received CSR with invalid signature, error: %v", err)
		return &certapi.CertificateSignResp{}, fmt.Errorf("CSR has invalid signature: %v", err)
	}
	rpcClient, err := c.getRPCClient()
	if err != nil {
		return &certapi.CertificateSignResp{}, fmt.Errorf("Unable to connect to remote server: %v", err)
	}
	defer rpcClient.Close()
	cmdClient := certapi.NewCertificatesClient(rpcClient.ClientConn)
	result, err := cmdClient.SignCertificateRequest(ctx, &certapi.CertificateSignReq{Csr: csr.Raw})
	if err != nil {
		log.Errorf("Remote server returned RPC error: %v", err)
		return &certapi.CertificateSignResp{}, fmt.Errorf("Remote server returned RPC error: %v", err)
	}
	return result, nil
}

// GetCaTrustChain returns the trust chain from the CA certificate to the root of trust
func (c *CertsProxy) GetCaTrustChain(ctx context.Context, empty *certapi.Empty) (*certapi.CaTrustChain, error) {
	rpcClient, err := c.getRPCClient()
	if err != nil {
		return &certapi.CaTrustChain{}, fmt.Errorf("Unable to connect to remote server: %v", err)
	}
	defer rpcClient.Close()
	cmdClient := certapi.NewCertificatesClient(rpcClient.ClientConn)
	return cmdClient.GetCaTrustChain(ctx, empty)
}

// GetTrustRoots returns the trust roots that should be used by client when verifying trust chains
func (c *CertsProxy) GetTrustRoots(ctx context.Context, empty *certapi.Empty) (*certapi.TrustRoots, error) {
	rpcClient, err := c.getRPCClient()
	if err != nil {
		return &certapi.TrustRoots{}, fmt.Errorf("Unable to connect to remote server: %v", err)
	}
	defer rpcClient.Close()
	cmdClient := certapi.NewCertificatesClient(rpcClient.ClientConn)
	return cmdClient.GetTrustRoots(ctx, empty)
}

// GetListenURL returns the listen URL for the server
func (c *CertsProxy) GetListenURL() string {
	return c.rpcServer.GetListenURL()
}

// GetRemoteURL returns the URL of the remote endpoint
func (c *CertsProxy) GetRemoteURL() string {
	return c.remoteURL
}

// Start starts listening for requests
func (c *CertsProxy) Start() {
	c.rpcServer.Start()
}

// Stop stops the rpc server
func (c *CertsProxy) Stop() error {
	return c.rpcServer.Stop()
}

// NewCertsProxy returns a new CertsProxy instance
func NewCertsProxy(listenURL, remoteURL string, clientOptions ...rpckit.Option) (*CertsProxy, error) {
	if listenURL == "" {
		return nil, errors.New("listen URL required")
	}

	if remoteURL == "" {
		return nil, errors.New("remote server URL required")
	}

	certsProxy := &CertsProxy{
		listenURL:        listenURL,
		remoteURL:        remoteURL,
		rpcClientOptions: clientOptions,
	}

	rpcServer, err := rpckit.NewRPCServer("certsproxy", listenURL, rpckit.WithTLSProvider(nil))
	if err != nil {
		return nil, errors.Wrap(err, "Error instantiating RPC server")
	}

	certsProxy.rpcServer = rpcServer
	certapi.RegisterCertificatesServer(rpcServer.GrpcServer, certsProxy)

	return certsProxy, err
}
