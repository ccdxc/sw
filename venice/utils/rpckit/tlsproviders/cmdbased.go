// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tlsproviders

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"net"
	"sync"
	"time"

	"github.com/pkg/errors"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"

	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/keymgr"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	defaultRetryInterval  = 500 * time.Millisecond
	defaultConnTimeout    = 3 * time.Second
	defaultConnMaxRetries = 5
)

// CMDBasedProvider is a TLS Provider which generates private keys locally and retrieves
// corresponding certificates from the Cluster Management Daemon (CMD)
type CMDBasedProvider struct {
	// the KeyMgr instance used to generate and store keys and certificates
	// KeyMgr is thread-safe
	keyMgr *keymgr.KeyMgr

	// the remote URL of the CMD endpoint
	cmdEndpointURL string

	// Client/Server Name. Used when asking for certificate
	endpointID string

	// When providing credentials for a client, use the same certificate for all connections
	clientCertificate *tls.Certificate

	// When providing credentials for servers, mint a new certificate for each server
	// so that we can put the correct server name in the subject
	serverCertificates map[string](*tls.Certificate)

	// Lock for serverCertificates map
	srvCertMapMutex sync.Mutex

	// Lock for clientCertificate
	clientCertMutex sync.Mutex

	// True when provider is ready to serve requests
	running bool

	// Lock for Connection
	runMutex sync.Mutex

	// CaTrustChain is used to form the bundles presented to the peer
	caTrustChain []*x509.Certificate

	// Additional trust roots allows cluster endpoints to trust entities outside the cluster
	trustRoots *x509.CertPool

	// User-provided options to control the behavior of the provider
	cmdProviderOptions
}

type cmdProviderOptions struct {
	// A gRPC load-balancer to be used when dialing the CMD endpoint
	// At present it needs to be passed in explicitly.
	// In the future it will be instantiated automatically.
	balancer grpc.Balancer

	// The time the provider waits for the CMD endpoint to respond before throwing an error.
	// It is passed to the grpc layer using the WithTimeout() option
	connTimeout time.Duration

	// The time the provider waits to reconnect to CMD after an error
	connRetryInterval time.Duration

	// The maximum number of times the provider tries to contact CMD before giving up
	// and returning an error to the caller
	connMaxRetries int
}

// CMDProviderOption fills the optional params for CMDBasedProvider
type CMDProviderOption func(opt *cmdProviderOptions)

// WithBalancer passes a gRPC load-balancer to be used when dialing CMD
func WithBalancer(b grpc.Balancer) CMDProviderOption {
	return func(o *cmdProviderOptions) {
		o.balancer = b
	}
}

// WithConnTimeout specified the timeout for opening the connection to CMD
func WithConnTimeout(t time.Duration) CMDProviderOption {
	return func(o *cmdProviderOptions) {
		o.connTimeout = t
	}
}

// WithConnRetryInterval specifies the time to wait before retrying to connect to CMD
func WithConnRetryInterval(i time.Duration) CMDProviderOption {
	return func(o *cmdProviderOptions) {
		o.connRetryInterval = i
	}
}

// WithConnMaxRetries specifies the maximum number of connection attempts to CMD
func WithConnMaxRetries(n int) CMDProviderOption {
	return func(o *cmdProviderOptions) {
		o.connMaxRetries = n
	}
}

func defaultOptions() *cmdProviderOptions {
	return &cmdProviderOptions{
		connTimeout:       defaultConnTimeout,
		connRetryInterval: defaultRetryInterval,
		connMaxRetries:    defaultConnMaxRetries,
	}
}

func (p *CMDBasedProvider) getCmdDialOptions() []grpc.DialOption {
	// The CMD API is not authenticated.
	// We cannot use TLS because the API itself is meant to supply TLS certificates.
	//
	// A component may request multiple certificates back-to-back, for example because
	// it hosts multiple servers or because it needs to work both as a client and as a server.
	// Since we use a single balancer for all requests but a new connection for each,
	// the balancer may still be holding to connections that are not fully closed when a new
	// request comes along. The default gRPC behavior in this case is to fail the RPC immediately
	// and let the client retry later. In our case it is better to wait for the old connection to
	// terminate cleanly and for the new one to be established. This is accomplished by
	// setting the FailFast to false on all RPCs.
	// See https://github.com/grpc/grpc/blob/master/doc/wait-for-ready.md

	dialOptions := []grpc.DialOption{
		grpc.WithInsecure(),
		grpc.WithBlock(),
		grpc.WithTimeout(p.connTimeout),
		grpc.WithDefaultCallOptions(grpc.FailFast(false))}

	if p.balancer != nil {
		dialOptions = append(dialOptions, grpc.WithBalancer(p.balancer))
	}
	return dialOptions
}

// Connect to CMD Endpoint
func (p *CMDBasedProvider) openCmdConnection() (*grpc.ClientConn, error) {
	var err error
	var conn *grpc.ClientConn
	for i := 0; i < p.connMaxRetries; i++ {
		conn, err = grpc.Dial(p.cmdEndpointURL, p.getCmdDialOptions()...)
		if err == nil {
			return conn, nil
		}
		time.Sleep(p.connRetryInterval)
	}
	return nil, err
}

func (p *CMDBasedProvider) fetchCaCertificates() error {
	conn, err := p.openCmdConnection()
	if err != nil {
		log.Errorf("Error opening CMD Connection, URL: %v, balancer: %+v", p.cmdEndpointURL, p.balancer)
		return err
	}
	defer conn.Close()
	cmdClient := certapi.NewCertificatesClient(conn)

	// Fetch CA trust chain
	tcs, err := cmdClient.GetCaTrustChain(context.Background(), &certapi.Empty{})
	if err != nil {
		return errors.Wrap(err, "Error fetching CA trust chain")
	}
	for _, c := range tcs.GetCertificates() {
		c, err := x509.ParseCertificate(c.GetCertificate())
		if err != nil {
			return errors.Wrapf(err, "Error parsing intermediate certificate: %+v", c)
		}
		p.caTrustChain = append(p.caTrustChain, c)
	}

	// Fetch additional trust roots
	rootsResp, err := cmdClient.GetTrustRoots(context.Background(), &certapi.Empty{})
	if err != nil {
		return errors.Wrap(err, "Error fetching trust roots")
	}

	for _, r := range rootsResp.GetTrustRoots() {
		c, err := x509.ParseCertificate(r.GetCertificate())
		if err != nil {
			return errors.Wrap(err, "Received malformed trust roots")
		}
		p.trustRoots.AddCert(c)
	}

	return nil
}

func (p *CMDBasedProvider) getTLSCertificate(subjAltName string) (*tls.Certificate, error) {
	conn, err := p.openCmdConnection()
	if err != nil {
		log.Errorf("Error opening CMD Connection, URL: %v, balancer: %+v", p.cmdEndpointURL, p.balancer)
		return nil, err
	}
	defer conn.Close()
	cmdClient := certapi.NewCertificatesClient(conn)

	privateKey, err := p.keyMgr.GetObject(subjAltName, keymgr.ObjectTypeKeyPair)
	if err != nil {
		return nil, errors.Wrap(err, "Error reading key pair from keymgr")
	}
	if privateKey == nil {
		privateKey, err = p.keyMgr.CreateKeyPair(subjAltName, keymgr.ECDSA256)
		if err != nil {
			return nil, errors.Wrap(err, "Error generating private key")
		}
	}
	csr, err := certs.CreateCSR(privateKey, []string{subjAltName}, nil)
	if err != nil {
		return nil, errors.Wrap(err, "Error generating CSR")
	}

	// Get the CSR signed
	csrResp, err := cmdClient.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: csr.Raw})
	if err != nil {
		return nil, errors.Wrap(err, "Error issuing sign request")
	}

	// Parse certificate and create bundle
	cert, err := x509.ParseCertificate(csrResp.GetCertificate().GetCertificate())
	if err != nil {
		return nil, errors.Wrapf(err, "Error parsing certificate: %+v", csrResp.GetCertificate())
	}
	bundle := [][]byte{cert.Raw}
	for _, c := range p.caTrustChain {
		bundle = append(bundle, c.Raw)
	}

	return &tls.Certificate{
		PrivateKey:  privateKey,
		Certificate: bundle,
	}, nil
}

// NewCMDBasedProvider instantiates a new CMD-based TLS provider
func NewCMDBasedProvider(cmdEpNameOrURL, endpointID string, km *keymgr.KeyMgr, opts ...CMDProviderOption) (*CMDBasedProvider, error) {
	if cmdEpNameOrURL == "" {
		return nil, fmt.Errorf("Requires CMD endpoint name or URL in form hostname:port")
	}
	if endpointID == "" {
		return nil, fmt.Errorf("endpointID is required")
	}
	if km == nil {
		return nil, fmt.Errorf("Requires valid instance of KeyMgr")
	}

	provider := &CMDBasedProvider{
		keyMgr:             km,
		cmdEndpointURL:     cmdEpNameOrURL,
		endpointID:         endpointID,
		serverCertificates: make(map[string](*tls.Certificate)),
		trustRoots:         x509.NewCertPool(),
		cmdProviderOptions: *defaultOptions(),
	}

	// add custom options
	for _, o := range opts {
		if o != nil {
			o(&provider.cmdProviderOptions)
		}
	}
	_, _, err := net.SplitHostPort(cmdEpNameOrURL)
	if err != nil && provider.balancer == nil {
		return nil, fmt.Errorf("Require a balancer to resolve %v", cmdEpNameOrURL)
	}

	err = provider.fetchCaCertificates()
	if err != nil {
		log.Errorf("Error fetching trust roots from %s: %v", cmdEpNameOrURL, err)
		return nil, err
	}

	provider.running = true
	return provider, nil
}

// NewDefaultCMDBasedProvider instantiates a new CMD-based TLS provider using a keymgr with default backend
func NewDefaultCMDBasedProvider(cmdEpNameOrURL, endpointID string, opts ...CMDProviderOption) (*CMDBasedProvider, error) {
	workDir, err := ioutil.TempDir("", "tlsprovider-"+endpointID+"-")
	if err != nil {
		return nil, errors.Wrapf(err, "Error creating workdir for GoCrypto backend")
	}
	be, err := keymgr.NewGoCryptoBackend(workDir)
	if err != nil {
		return nil, errors.Wrapf(err, "Error instantiating GoCrypto backend")
	}
	km, err := keymgr.NewKeyMgr(be)
	if err != nil {
		be.Close()
		return nil, errors.Wrapf(err, "Error instantiating keymgr")
	}
	prov, err := NewCMDBasedProvider(cmdEpNameOrURL, endpointID, km, opts...)
	if err != nil {
		km.Close()
		return nil, errors.Wrapf(err, "Error instantiating keymgr")
	}
	return prov, nil
}

// getServerCertificate is the callback that returns server certificates
func (p *CMDBasedProvider) getServerCertificate(clientHelloInfo *tls.ClientHelloInfo) (*tls.Certificate, error) {
	// FIXME: we should not mint certificates based on what client is asking but
	// have server explicitly declare which names are allowed and provide default
	// if client is asking for something we don't have

	p.runMutex.Lock()
	defer p.runMutex.Unlock()
	if !p.running {
		return nil, fmt.Errorf("Shutting down")
	}

	serverName := clientHelloInfo.ServerName
	if serverName == "" {
		serverName = p.endpointID
	}

	p.srvCertMapMutex.Lock()
	tlsCert := p.serverCertificates[serverName]
	p.srvCertMapMutex.Unlock()
	if tlsCert == nil {
		var err error
		tlsCert, err = p.getTLSCertificate(serverName)
		if err != nil {
			return nil, fmt.Errorf("Error getting TLS certificate for server %s: %v", serverName, err)
		}
		p.srvCertMapMutex.Lock()
		p.serverCertificates[serverName] = tlsCert
		p.srvCertMapMutex.Unlock()
	}

	return tlsCert, nil
}

// GetServerOptions returns server options to be passed to grpc.NewServer()
func (p *CMDBasedProvider) GetServerOptions(serverName string) (grpc.ServerOption, error) {
	tlsConfig := getTLSServerConfig(serverName, nil, p.trustRoots)
	// Set callback to be invoked whenever a new connection is established
	// This enables certificate rotation
	tlsConfig.GetCertificate = p.getServerCertificate
	return grpc.Creds(credentials.NewTLS(tlsConfig)), nil
}

// GetDialOptions returns dial options to be passed to grpc.Dial()
func (p *CMDBasedProvider) GetDialOptions(serverName string) (grpc.DialOption, error) {
	// Golang 1.8.3 and subsequent support a GetClientCertificate option to allow a client to
	// pick a certificate based on the server's certificate request. However, with gRPC this
	// doesn't work, because gRPC currently copies tls.Config structures using a naive,
	// outdated implementation that discards GetClientCertificate
	// (see https://github.com/golang/go/issues/15771)
	// It is not a big problem because DialOptions are associated to a single outgoing connection
	// (as opposed to ServerOptions, which are used for all incoming connections) and rpckit
	// reinvokes GetDialOptions() if client performs a Reconnect() call on an existing RPCClient
	p.runMutex.Lock()
	defer p.runMutex.Unlock()
	if !p.running {
		return nil, fmt.Errorf("Shutting down")
	}
	p.clientCertMutex.Lock()
	defer p.clientCertMutex.Unlock()
	if p.clientCertificate == nil {
		cert, err := p.getTLSCertificate(p.endpointID)
		if err != nil {
			return nil, errors.Wrap(err, "Error retrieving client certificate")
		}
		p.clientCertificate = cert
	}

	tlsConfig := getTLSClientConfig(serverName, p.clientCertificate, p.trustRoots)
	return grpc.WithTransportCredentials(credentials.NewTLS(tlsConfig)), nil
}

// Close closes the client.
func (p *CMDBasedProvider) Close() {
	p.runMutex.Lock()
	p.running = false
	p.runMutex.Unlock()
	if p.keyMgr != nil {
		p.keyMgr.Close()
		p.keyMgr = nil
	}
}
