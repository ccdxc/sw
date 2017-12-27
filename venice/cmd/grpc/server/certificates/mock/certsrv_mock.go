// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package mock

import (
	"bytes"
	"crypto"
	"crypto/x509"
	"net"
	"sync/atomic"

	"github.com/pkg/errors"
	context "golang.org/x/net/context"
	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
)

// CertSrv is a mock instance of the CMD certificates server
type CertSrv struct {
	RPCServer   *grpc.Server
	listenURL   string
	privateKey  crypto.PrivateKey
	certificate *x509.Certificate
	trustChain  []*x509.Certificate
	trustRoots  []*x509.Certificate
	// counters
	rpcSuccess uint64
	rpcError   uint64
}

func (c *CertSrv) incrementRPCSuccess() {
	atomic.AddUint64(&c.rpcSuccess, 1)
}

func (c *CertSrv) incrementRPCError() {
	atomic.AddUint64(&c.rpcError, 1)
}

// SignCertificateRequest is the handler for the RPC method with the same name
// It receives a CSR and returns a signed certificate
func (c *CertSrv) SignCertificateRequest(ctx context.Context, req *certapi.CertificateSignReq) (*certapi.CertificateSignResp, error) {
	var err error

	// make sure counters are updated on return
	defer func() {
		if err != nil {
			c.incrementRPCError()
		} else {
			c.incrementRPCSuccess()
		}
	}()

	// Only check that CSR is syntactically valid
	csr, err := x509.ParseCertificateRequest(req.GetCsr())
	if err != nil {
		log.Errorf("Received invalid certificate request, error: %v", err)
		return nil, errors.Wrapf(err, "Invalid certificate request")
	}

	err = csr.CheckSignature()
	if err != nil {
		log.Errorf("Received CSR with invalid signature, error: %v", err)
		return nil, errors.Wrapf(err, "CSR has invalid signature")
	}

	cert, err := certs.SignCSRwithCA(7 /* days */, csr, c.certificate, c.privateKey)
	if err != nil {
		log.Errorf("Error signing CSR: %v", err)
		return nil, errors.Wrapf(err, "Error signing CSR")
	}

	return &certapi.CertificateSignResp{
		Certificate: &certapi.Certificate{Certificate: cert.Raw},
	}, nil
}

// GetTrustRoots is the handler for the RPC method that returns trust roots
// It returns an array of trusted certificates
func (c *CertSrv) GetTrustRoots(ctx context.Context, empty *certapi.Empty) (*certapi.TrustRoots, error) {
	certs := make([]*certapi.Certificate, 0)
	for _, c := range c.trustRoots {
		certs = append(certs, &certapi.Certificate{Certificate: c.Raw})
	}
	c.incrementRPCSuccess()
	return &certapi.TrustRoots{TrustRoots: certs}, nil
}

// GetCaTrustChain is the handler for the RPC method that returns the CA trust chain
func (c *CertSrv) GetCaTrustChain(ctx context.Context, empty *certapi.Empty) (*certapi.CaTrustChain, error) {
	certs := make([]*certapi.Certificate, 0)
	for _, c := range c.trustChain {
		certs = append(certs, &certapi.Certificate{Certificate: c.Raw})
	}
	c.incrementRPCSuccess()
	return &certapi.CaTrustChain{Certificates: certs}, nil
}

// NewCertSrv returns a mock instance of the CMD certificates server that uses passed-in keys and certs
func NewCertSrv(listenURL, certPath, keyPath, rootsPath string) (*CertSrv, error) {
	if listenURL == "" {
		return nil, errors.New("ListenURL parameter is required")
	}

	key, err := certs.ReadPrivateKey(keyPath)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading private key from %s", keyPath)
	}

	caCert, err := certs.ReadCertificate(certPath)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading certificate from %s", keyPath)
	}

	// create the controller instance
	ctrler := &CertSrv{
		privateKey:  key,
		certificate: caCert,
	}

	trustRoots, err := certs.ReadCertificates(rootsPath)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading trust roots from %s", rootsPath)
	}

	if certs.IsSelfSigned(caCert) {
		ctrler.trustRoots = append(ctrler.trustRoots, caCert)
		ctrler.trustChain = append(ctrler.trustChain, caCert)
	}

	for _, c := range trustRoots {
		ctrler.trustRoots = append(ctrler.trustRoots, c)
		// assume at most 2-level hierarchy
		if !certs.IsSelfSigned(caCert) && bytes.Equal(c.RawSubject, caCert.RawIssuer) {
			ctrler.trustChain = append(ctrler.trustChain, caCert, c)
		}
	}
	if len(ctrler.trustChain) < 1 {
		return nil, errors.Wrapf(err, "Could not establish CA trust chain")
	}

	// Do not use rpckit to avoid circular dependencies
	listener, err := net.Listen("tcp", listenURL)
	if err != nil {
		return nil, errors.Wrapf(err, "Could start listening at %s", listenURL)
	}
	server := grpc.NewServer()
	certapi.RegisterCertificatesServer(server, ctrler)
	ctrler.RPCServer = server
	ctrler.listenURL = listener.Addr().String()
	go server.Serve(listener)

	return ctrler, err
}

// Returns the URL the server is listening on
func (c *CertSrv) GetListenURL() string {
	return c.listenURL
}

// Stop stops the gRPC server and performs cleanup
func (c *CertSrv) Stop() error {
	c.RPCServer.Stop()
	return nil
}

func (c *CertSrv) GetRPCSuccessCount() uint64 {
	return atomic.LoadUint64(&c.rpcSuccess)
}

func (c *CertSrv) GetRPCErrorCount() uint64 {
	return atomic.LoadUint64(&c.rpcError)
}

func (c *CertSrv) ClearRPCCounts() {
	atomic.StoreUint64(&c.rpcSuccess, 0)
	atomic.StoreUint64(&c.rpcError, 0)
}
