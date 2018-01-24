// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package utils

import (
	"crypto"
	"crypto/x509"
	"fmt"
	"path"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	"github.com/pensando/sw/venice/utils/certmgr"
)

// GetCertificateMgrDir returns the directory where CertificateMgr stores the keys
func GetCertificateMgrDir() string {
	return path.Join(env.Options.ConfigDir, "cmd", "certmgr")
}

// GetCaTrustChain returns the trust chain from the CA certificate to the root of trust in wire format
func GetCaTrustChain(cm *certmgr.CertificateMgr) []*certapi.Certificate {
	certs := make([]*certapi.Certificate, 0)
	for _, c := range cm.Ca().TrustChain() {
		certs = append(certs, &certapi.Certificate{Certificate: c.Raw})
	}
	return certs
}

// GetTrustRoots returns the trust roots that should be used by client when verifying trust chains in wire format
func GetTrustRoots(cm *certmgr.CertificateMgr) []*certapi.Certificate {
	certs := make([]*certapi.Certificate, 0)
	for _, c := range cm.Ca().TrustRoots() {
		certs = append(certs, &certapi.Certificate{Certificate: c.Raw})
	}
	return certs
}

// MakeCertMgrBundle creates a grpc.CertMgrBundle object that can be used
// to transport CA objects (key, certificate, trust roots, etc.) from a CMD
// instance to another. It requires a peer transport key in order to wrap
// the private key.
func MakeCertMgrBundle(cm *certmgr.CertificateMgr, peerID string, peerTransportKeyBytes []byte) (*grpc.CertMgrBundle, error) {
	peerTransportKey := cm.UnmarshalKeyAgreementKey(peerTransportKeyBytes)
	if peerTransportKey == nil {
		return nil, fmt.Errorf("Error unmarshaling peer transport key")
	}
	wrappedCaKey, err := cm.GetWrappedCaKey(peerID, peerTransportKey)
	if err != nil {
		return nil, fmt.Errorf("Error getting wrapped CA key: %v", err)
	}
	bundle := &grpc.CertMgrBundle{
		WrappedCaKey: wrappedCaKey,
		CaTrustChain: GetCaTrustChain(cm),
		TrustRoots:   GetTrustRoots(cm),
	}
	return bundle, nil
}

// UnpackCertMgrBundle takes a provided CertMgr bundle and peer transport key and unpacks
// the CA artifacts (signing key, trust chain, trust roots, ...) in KeyMgr.
// This should be called while the CertMgr is not running.
func UnpackCertMgrBundle(cm *certmgr.CertificateMgr, bundle *grpc.CertMgrBundle, peerTransportKey crypto.PublicKey) error {
	defer cm.DestroyKeyAgreementKey("self")
	trustChain := make([]*x509.Certificate, 0)
	for _, c := range bundle.GetCaTrustChain() {
		cert, err := x509.ParseCertificate(c.GetCertificate())
		if err != nil {
			return errors.Wrapf(err, "Error parsing CA trust chain certificate")
		}
		trustChain = append(trustChain, cert)
	}
	trustRoots := make([]*x509.Certificate, 0)
	for _, c := range bundle.GetTrustRoots() {
		cert, err := x509.ParseCertificate(c.GetCertificate())
		if err != nil {
			return errors.Wrapf(err, "Error parsing trust root certificate")
		}
		trustRoots = append(trustRoots, cert)
	}
	err := cm.LoadCaTrustChain(trustChain)
	if err != nil {
		return errors.Wrapf(err, "Error loading CA trust chain in CertMgr")
	}
	err = cm.LoadTrustRoots(trustRoots)
	if err != nil {
		return errors.Wrapf(err, "Error loading trust roots in CertMgr")
	}
	caCert := trustChain[0]
	err = cm.UnwrapCaKey(bundle.WrappedCaKey, caCert.PublicKey, "self", peerTransportKey)
	if err != nil {
		return errors.Wrapf(err, "Error unwrapping CA key in CertMgr")
	}
	return nil
}
