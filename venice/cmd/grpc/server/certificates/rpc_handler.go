// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certificates

import (
	"crypto/x509"
	"fmt"

	"github.com/pkg/errors"
	"golang.org/x/net/context"

	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/utils"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/log"
)

// RPCHandler handles all certificates gRPC calls.
type RPCHandler struct {
	CertMgr *certmgr.CertificateMgr
}

// SignCertificateRequest checks a CSR submitted by a client, signs it and returns a certificate
func (h *RPCHandler) SignCertificateRequest(ctx context.Context, req *certapi.CertificateSignReq) (*certapi.CertificateSignResp, error) {
	peerID := ctxutils.GetPeerID(ctx)
	if !h.CertMgr.IsReady() {
		log.Errorf("Received invalid certificate request, peerID %v, error: CertMgr not ready", peerID)
		return nil, fmt.Errorf("CertMgr not ready")
	}
	csr, err := x509.ParseCertificateRequest(req.GetCsr())
	if err != nil {
		log.Errorf("Received invalid certificate request, peerID: %v, error: %v", peerID, err)
		return nil, errors.Wrap(err, "Invalid certificate request")
	}
	err = csr.CheckSignature()
	if err != nil {
		log.Errorf("Received CSR with invalid signature, peerID: %v, error: %v", peerID, err)
		return nil, errors.Wrap(err, "Certificate request has invalid signature")
	}
	cert, err := h.CertMgr.Ca().Sign(csr)
	if err != nil {
		log.Errorf("Error signing CSR, peerID: %v, error: %v", peerID, err)
		return nil, errors.Wrap(err, "Error signing certificate request")
	}
	log.Infof("Returning certificate, peerID: %v, CN:%v, SANs: %v", peerID, cert.Subject.Names, cert.DNSNames)
	return &certapi.CertificateSignResp{
		Certificate: &certapi.Certificate{Certificate: cert.Raw},
	}, nil
}

// GetCaTrustChain returns the trust chain from the CA certificate to the root of trust
func (h *RPCHandler) GetCaTrustChain(ctx context.Context, empty *certapi.Empty) (*certapi.CaTrustChain, error) {
	if !h.CertMgr.IsReady() {
		return &certapi.CaTrustChain{}, fmt.Errorf("CertMgr not ready")
	}
	certs := utils.GetCaTrustChain(h.CertMgr)
	return &certapi.CaTrustChain{Certificates: certs}, nil
}

// GetTrustRoots returns the trust roots that should be used by client when verifying trust chains
func (h *RPCHandler) GetTrustRoots(ctx context.Context, empty *certapi.Empty) (*certapi.TrustRoots, error) {
	if !h.CertMgr.IsReady() {
		return &certapi.TrustRoots{}, fmt.Errorf("CertMgr not ready")
	}
	certs := utils.GetTrustRoots(h.CertMgr)
	return &certapi.TrustRoots{TrustRoots: certs}, nil
}

// NewRPCHandler returns a new handler for the RPC interface
func NewRPCHandler(cm *certmgr.CertificateMgr) *RPCHandler {
	return &RPCHandler{
		CertMgr: cm,
	}
}
