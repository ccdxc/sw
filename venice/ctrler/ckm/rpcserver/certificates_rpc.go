// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"crypto/x509"
	"errors"

	"github.com/pensando/sw/venice/ctrler/ckm/rpcserver/ckmproto"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/log"

	context "golang.org/x/net/context"
)

// CertificatesRPCServer handles all certificates gRPC calls.
type CertificatesRPCServer struct {
	certificateMgr *certmgr.CertificateMgr
}

// SignCertificateRequest checks a CSR submitted by a client, signs it and returns a certificate
func (c *CertificatesRPCServer) SignCertificateRequest(ctx context.Context, req *ckmproto.CertificateSignReq) (*ckmproto.CertificateSignResp, error) {
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
	if c.certificateMgr == nil || !c.certificateMgr.IsReady() {
		log.Errorf("Certificates service is not ready to process CSR")
		return &ckmproto.CertificateSignResp{Approved: false}, errors.New("Certificate service not ready")
	}
	cert, err := c.certificateMgr.Ca().Sign(csr)
	if err != nil {
		log.Errorf("Error signing CSR: %v", err)
		return &ckmproto.CertificateSignResp{Approved: false}, err
	}

	return &ckmproto.CertificateSignResp{
		Certificate: &ckmproto.Certificate{Certificate: cert.Raw},
		Approved:    true}, nil
}

// GetCaTrustChain returns the trust chain from the CA certificate to the root of trust
func (c *CertificatesRPCServer) GetCaTrustChain(ctx context.Context, empty *ckmproto.Empty) (*ckmproto.CaTrustChain, error) {
	certs := make([]*ckmproto.Certificate, 0)
	for _, c := range c.certificateMgr.Ca().TrustChain() {
		certs = append(certs, &ckmproto.Certificate{Certificate: c.Raw})
	}
	return &ckmproto.CaTrustChain{Certificates: certs}, nil
}

// GetTrustRoots returns the trust roots that should be used by client when verifying trust chains
func (c *CertificatesRPCServer) GetTrustRoots(ctx context.Context, empty *ckmproto.Empty) (*ckmproto.TrustRoots, error) {
	certs := make([]*ckmproto.Certificate, 0)
	for _, c := range c.certificateMgr.Ca().TrustRoots() {
		certs = append(certs, &ckmproto.Certificate{Certificate: c.Raw})
	}
	return &ckmproto.TrustRoots{TrustRoots: certs}, nil
}

// NewCertificatesRPCServer returns a certificates RPC server
func NewCertificatesRPCServer(cs *certmgr.CertificateMgr) (*CertificatesRPCServer, error) {
	return &CertificatesRPCServer{certificateMgr: cs}, nil
}
