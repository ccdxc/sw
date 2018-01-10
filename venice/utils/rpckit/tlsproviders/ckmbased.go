// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tlsproviders

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"net"
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
	retryInterval = 500 * time.Millisecond
	maxRetries    = 5
)

// CKMBasedProvider is a TLS Provider which retrieves keys and certificates from the Cluster Key Manager (CKM)
// using the certificates API
type CKMBasedProvider struct {
	// the KeyMgr instance used to generate and store keys and certificates
	keyMgr *keymgr.KeyMgr

	// the remote URL of the CKM endpoint
	ckmEndpointURL string

	// conn is the gRPC client connection
	conn *grpc.ClientConn

	// the CKM gRPC client
	ckmClient certapi.CertificatesClient

	// When providing credentials for a client, use the same certificate for all connections
	clientCertificate *tls.Certificate

	// When providing credentials for servers, mint a new certificate for each server
	// so that we can put the correct server name in the subject
	serverCertificates map[string](*tls.Certificate)

	// CaTrustChain is used to form the bundles presented to the peer
	caTrustChain []*x509.Certificate

	// Additional trust roots allows cluster endpoints to trust entities outside the cluster
	trustRoots *x509.CertPool

	// User-provided options to control the behavior of the provider
	ckmProviderOptions
}

type ckmProviderOptions struct {
	// A gRPC load-balancer to be used when dialing the CKM endpoint
	// At present it needs to be passed in explicitly.
	// In the future it will be instantiated automatically.
	balancer grpc.Balancer
}

// CKMProviderOption fills the optional params for CKMBasedProvider
type CKMProviderOption func(opt *ckmProviderOptions)

// WithBalancer passes a gRPC load-balancer to be used when dialing CKM
func WithBalancer(b grpc.Balancer) CKMProviderOption {
	return func(o *ckmProviderOptions) {
		o.balancer = b
	}
}

func (p *CKMBasedProvider) getCkmDialOptions() []grpc.DialOption {
	// Right now the CKM API is not authenticated.
	// We cannot use TLS because the API itself is meant to supply TLS certificates
	dialOptions := []grpc.DialOption{grpc.WithInsecure(), grpc.WithBlock(), grpc.WithTimeout(time.Second * 3)}
	if p.balancer != nil {
		dialOptions = append(dialOptions, grpc.WithBalancer(p.balancer))
	}

	return dialOptions
}

func (p *CKMBasedProvider) fetchCaCertificates() error {
	// Fetch CA trust chain
	tcs, err := p.ckmClient.GetCaTrustChain(context.Background(), &certapi.Empty{})
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
	rootsResp, err := p.ckmClient.GetTrustRoots(context.Background(), &certapi.Empty{})
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

func (p *CKMBasedProvider) getTLSCertificate(subjAltName string) (*tls.Certificate, error) {
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
	csrResp, err := p.ckmClient.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: csr.Raw})
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

// NewCKMBasedProvider instantiates a new CKM-based TLS provider
func NewCKMBasedProvider(ckmEpNameOrURL string, km *keymgr.KeyMgr, opts ...CKMProviderOption) (*CKMBasedProvider, error) {
	if ckmEpNameOrURL == "" {
		return nil, fmt.Errorf("Requires CKM endpoint name or URL in form hostname:port")
	}
	if km == nil {
		return nil, fmt.Errorf("Requires valid instance of KeyMgr")
	}

	provider := &CKMBasedProvider{
		keyMgr:             km,
		ckmEndpointURL:     ckmEpNameOrURL,
		serverCertificates: make(map[string](*tls.Certificate)),
		trustRoots:         x509.NewCertPool(),
	}

	// add custom options
	for _, o := range opts {
		if o != nil {
			o(&provider.ckmProviderOptions)
		}
	}
	_, _, err := net.SplitHostPort(ckmEpNameOrURL)
	if err != nil && provider.balancer == nil {
		return nil, fmt.Errorf("Require a balancer to resolve %v", ckmEpNameOrURL)
	}

	// Connect to CKM Endpoint and create RPC client
	var success bool
	var conn *grpc.ClientConn
	for i := 0; i < maxRetries; i++ {
		log.Infof("Connecting to CKM Endpoint: %v", provider.ckmEndpointURL)
		conn, err = grpc.Dial(provider.ckmEndpointURL, provider.getCkmDialOptions()...)
		if err == nil {
			success = true
			provider.ckmClient = certapi.NewCertificatesClient(conn)
			break
		}
		time.Sleep(retryInterval)
	}
	if !success {
		return nil, errors.Wrapf(err, "Failed to dial CKM Endpoint %s", provider.ckmEndpointURL)
	}
	provider.conn = conn

	err = provider.fetchCaCertificates()
	if err != nil {
		log.Fatalf("Error fetching trust roots from %s: %v", ckmEpNameOrURL, err)
	}

	return provider, nil
}

// NewDefaultCKMBasedProvider instantiates a new CKM-based TLS provider using a keymgr with default backend
func NewDefaultCKMBasedProvider(ckmEpNameOrURL, endpointID string, opts ...CKMProviderOption) (*CKMBasedProvider, error) {
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
	prov, err := NewCKMBasedProvider(ckmEpNameOrURL, km, opts...)
	if err != nil {
		km.Close()
		return nil, errors.Wrapf(err, "Error instantiating keymgr")
	}
	return prov, nil
}

// getServerCertificate is the callback that returns server certificates
func (p *CKMBasedProvider) getServerCertificate(clientHelloInfo *tls.ClientHelloInfo) (*tls.Certificate, error) {
	serverName := clientHelloInfo.ServerName
	tlsCert := p.serverCertificates[serverName]
	if tlsCert == nil {
		var err error
		tlsCert, err = p.getTLSCertificate(serverName)
		if err != nil {
			return nil, fmt.Errorf("Error getting dial options for server %s: %v", serverName, err)
		}
		p.serverCertificates[serverName] = tlsCert
	}

	return tlsCert, nil
}

// GetServerOptions returns server options to be passed to grpc.NewServer()
func (p *CKMBasedProvider) GetServerOptions(serverName string) (grpc.ServerOption, error) {
	tlsConfig := getTLSServerConfig(serverName, nil, p.trustRoots)
	// Set callback to be invoked whenever a new connection is established
	// This enables certificate rotation
	tlsConfig.GetCertificate = p.getServerCertificate
	return grpc.Creds(credentials.NewTLS(tlsConfig)), nil
}

// GetDialOptions returns dial options to be passed to grpc.Dial()
func (p *CKMBasedProvider) GetDialOptions(serverName string) (grpc.DialOption, error) {
	// Golang 1.8.3 and subsequent support a GetClientCertificate option to allow a client to
	// pick a certificate based on the server's certificate request. However, with gRPC this
	// doesn't work, because gRPC currently copies tls.Config structures using a naive,
	// outdated implementation that discards GetClientCertificate
	// (see https://github.com/golang/go/issues/15771)
	// It is not a big problem because DialOptions are associated to a single outgoing connection
	// (as opposed to ServerOptions, which are used for all incoming connections) and rpckit
	// reinvokes GetDialOptions() if client performs a Reconnect() call on an existing RPCClient
	if p.clientCertificate == nil {
		// TODO: replace "client" with actual client name
		// note that most TLS implementations do not check client certificate subject
		cert, err := p.getTLSCertificate("client")
		if err != nil {
			return nil, errors.Wrap(err, "Error retrieving client certificate")
		}
		p.clientCertificate = cert
	}

	tlsConfig := getTLSClientConfig(serverName, p.clientCertificate, p.trustRoots)
	return grpc.WithTransportCredentials(credentials.NewTLS(tlsConfig)), nil
}

// Close closes the client.
func (p *CKMBasedProvider) Close() {
	if p.conn != nil {
		log.Infof("Closing client conn: %+v", p.conn)
		p.conn.Close()
		p.conn = nil
	}
	if p.keyMgr != nil {
		p.keyMgr.Close()
		p.keyMgr = nil
	}
}
