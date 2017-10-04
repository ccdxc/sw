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

	"github.com/pensando/sw/venice/ctrler/ckm/rpcserver/ckmproto"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
)

// CKMctrler is a mock instance of CKM, without any dependency on CKM components.
type CKMctrler struct {
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

func (c *CKMctrler) incrementRPCSuccess() {
	atomic.AddUint64(&c.rpcSuccess, 1)
}

func (c *CKMctrler) incrementRPCError() {
	atomic.AddUint64(&c.rpcError, 1)
}

// SignCertificateRequest is the handler for the RPC method with the same name
// It receives a CSR and returns a signed certificate
func (c *CKMctrler) SignCertificateRequest(ctx context.Context, req *ckmproto.CertificateSignReq) (*ckmproto.CertificateSignResp, error) {
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
		return &ckmproto.CertificateSignResp{Approved: false}, err
	}

	err = csr.CheckSignature()
	if err != nil {
		log.Errorf("Received CSR with invalid signature, error: %v", err)
		return &ckmproto.CertificateSignResp{Approved: false}, err
	}

	cert, err := certs.SignCSRwithCA(7 /* days */, csr, c.certificate, c.privateKey)
	if err != nil {
		log.Errorf("Error signing CSR: %v", err)
		return &ckmproto.CertificateSignResp{Approved: false}, err
	}

	return &ckmproto.CertificateSignResp{
		Certificate: &ckmproto.Certificate{Certificate: cert.Raw},
		Approved:    true}, nil
}

// GetTrustRoots is the handler for the RPC method that returns trust roots
// It returns an array of trusted certificates
func (c *CKMctrler) GetTrustRoots(ctx context.Context, empty *ckmproto.Empty) (*ckmproto.TrustRoots, error) {
	certs := make([]*ckmproto.Certificate, 0)
	for _, c := range c.trustRoots {
		certs = append(certs, &ckmproto.Certificate{Certificate: c.Raw})
	}
	c.incrementRPCSuccess()
	return &ckmproto.TrustRoots{TrustRoots: certs}, nil
}

// GetCaTrustChain is the handler for the RPC method that returns the CA trust chain
func (c *CKMctrler) GetCaTrustChain(ctx context.Context, empty *ckmproto.Empty) (*ckmproto.CaTrustChain, error) {
	certs := make([]*ckmproto.Certificate, 0)
	for _, c := range c.trustChain {
		certs = append(certs, &ckmproto.Certificate{Certificate: c.Raw})
	}
	c.incrementRPCSuccess()
	return &ckmproto.CaTrustChain{Certificates: certs}, nil
}

// NewCKMctrler returns a mock CKM instance that uses passed-in keys and certs
func NewCKMctrler(listenURL, certPath, keyPath, rootsPath string) (*CKMctrler, error) {
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
	ctrler := &CKMctrler{
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

	// Do not use rpckit to avoid circular dependencies, as one of the TLS provider tests depends on CKM
	listener, err := net.Listen("tcp", listenURL)
	if err != nil {
		return nil, errors.Wrapf(err, "Could start listening at %s", listenURL)
	}
	server := grpc.NewServer()
	ckmproto.RegisterCertificatesServer(server, ctrler)
	ctrler.RPCServer = server
	ctrler.listenURL = listener.Addr().String()
	go server.Serve(listener)

	return ctrler, err
}

// Returns the URL the server is listening on
func (c *CKMctrler) GetListenURL() string {
	return c.listenURL
}

// Stop stops the gRPC server and performs cleanup
func (c *CKMctrler) Stop() error {
	c.RPCServer.Stop()
	return nil
}

func (c *CKMctrler) GetRPCSuccessCount() uint64 {
	return atomic.LoadUint64(&c.rpcSuccess)
}

func (c *CKMctrler) GetRPCErrorCount() uint64 {
	return atomic.LoadUint64(&c.rpcError)
}
