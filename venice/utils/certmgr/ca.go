// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certmgr

import (
	"crypto"
	"crypto/x509"
	"fmt"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/keymgr"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// The size of the private key to use if we are signing our own root certificate
	caSelfSignedKeySize = 4096

	// The validity period of the self-signed root certificate (1 year and 10 days)
	caSelfSignedCertDurationDays = 375

	// The validity period of the certificates that we issue to cluster nodes
	nodeCertDurationDays = 33

	caKeyID                 = "CaKey"
	caKeyType               = keymgr.RSA4096
	caCertificateID         = "CaCert"
	intermediatesBundleName = "intermediate"
	trustRootsBundleName    = "root"
)

// CertificateAuthority is the interface for issuing certificates
// Most of the methods are made available via a gRPC endpoint by rpcserver
type CertificateAuthority struct {
	keyMgr        *keymgr.KeyMgr
	caKey         crypto.PrivateKey
	caCertificate *x509.Certificate
	trustChain    []*x509.Certificate
	trustRoots    []*x509.Certificate
	issuedCerts   []*x509.Certificate
	ready         bool
}

// Calls Golang's x509.Verify() function to search for a valid trust chain from cert
// to a trust root using known intermediate certificates
func (l *CertificateAuthority) getValidTrustChains(cert *x509.Certificate, intermediates []*x509.Certificate) ([][]*x509.Certificate, error) {
	// For the moment we just check signatures and times.
	// We accept whatever Common Name or SAN is there.
	// We will have more complex validation logic later.
	options := x509.VerifyOptions{
		Roots:         certs.NewCertPool(l.trustRoots),
		Intermediates: certs.NewCertPool(intermediates),
		KeyUsages:     []x509.ExtKeyUsage{x509.ExtKeyUsageAny},
	}

	// TODO: Docker Swarmkit has a more strict validation algorithm than Verify()
	//       See if we want to use that one instead
	return cert.Verify(options)
}

// initializes a certificate services instance
func (l *CertificateAuthority) init(bootstrap bool) error {
	// Try to read an existing private key from keymgr.
	// If an object with ID caKeyID does not exist, caKey will be nil, but no error is returned
	caKey, err := l.keyMgr.GetObject(caKeyID, keymgr.ObjectTypeKeyPair)
	if err != nil {
		return errors.Wrap(err, "Error reading CA key from backend")
	}

	// If we fail to read an existing private key, we bootstrap the CA with
	// a generated private key and a self-signed certificate
	if caKey == nil && bootstrap {
		caKey, err = l.keyMgr.CreateKeyPair(caKeyID, caKeyType)
		if err != nil {
			return errors.Wrap(err, "Error generating signing key")
		}
		selfSignedCert, err := certs.SelfSign(caSelfSignedCertDurationDays, "CMD ROOT CA", caKey)
		if err != nil {
			return errors.Wrap(err, "Error generating self-signed certificate")
		}
		err = l.keyMgr.StoreObject(keymgr.NewCertificateObject(caCertificateID, selfSignedCert))
		if err != nil {
			return errors.Wrap(err, "Error storing self-signed certificate")
		}
		err = l.keyMgr.StoreObject(keymgr.NewCertificateBundleObject(trustRootsBundleName, []*x509.Certificate{selfSignedCert}))
		if err != nil {
			return errors.Wrap(err, "Error creating trust roots bundle")
		}
	}
	if caKey != nil {
		l.caKey = caKey
	} else {
		return fmt.Errorf("CA signing key not found")
	}
	// read CA certificate
	caCert, err := l.keyMgr.GetObject(caCertificateID, keymgr.ObjectTypeCertificate)
	if err != nil {
		return errors.Wrap(err, "Error reading CA certificate")
	}
	l.caCertificate = caCert.(*keymgr.Certificate).Certificate

	// validate that public and private key match
	valid, err := certs.ValidateKeyCertificatePair(l.caKey, l.caCertificate)
	if !valid || err != nil {
		return errors.Wrap(err, "Public and private key do not match")
	}

	// Load intermediates if present
	var intermediates []*x509.Certificate
	intermediatesBundle, err := l.keyMgr.GetObject(intermediatesBundleName, keymgr.ObjectTypeCertificateBundle)
	if err != nil {
		return errors.Wrap(err, "Error reading intermediate certificates")
	}
	if intermediatesBundle != nil {
		intermediates = intermediatesBundle.(*keymgr.CertificateBundle).Certificates
	}
	log.Infof("Loaded %v intermediates certificates", len(intermediates))

	// Load trust roots if present
	trustRootsBundle, err := l.keyMgr.GetObject(trustRootsBundleName, keymgr.ObjectTypeCertificateBundle)
	if err != nil {
		return errors.Wrap(err, "Error reading trust roots")
	}
	if trustRootsBundle != nil {
		l.trustRoots = trustRootsBundle.(*keymgr.CertificateBundle).Certificates
	}
	log.Infof("Loaded %v trust roots", len(l.trustRoots))

	// form and validate the CA trust chain
	tcs, err := l.getValidTrustChains(l.caCertificate, intermediates)
	if err != nil || len(tcs) < 1 {
		return errors.Wrap(err, "Error establishing trust chain for the CA")
	}
	l.trustChain = tcs[0]
	log.Infof("CA trust chain length: %v", len(l.trustChain))

	l.ready = true
	return nil
}

// Sign validates and sign a certificate signing request (CSR)
func (l *CertificateAuthority) Sign(csr *x509.CertificateRequest) (*x509.Certificate, error) {
	cert, err := certs.SignCSRwithCA(nodeCertDurationDays, csr, l.caCertificate, l.caKey)
	if err != nil {
		return nil, errors.Wrap(err, "Error signing CSR")
	}
	l.issuedCerts = append(l.issuedCerts, cert)
	return cert, err
}

// TrustRoots returns the trust roots that should be trusted by all members of the cluster.
// These include the root of the CA trust chain + other roots that should be
// trusted by cluster members, if any
func (l *CertificateAuthority) TrustRoots() []*x509.Certificate {
	result := make([]*x509.Certificate, 0)
	for _, r := range l.trustRoots {
		result = append(result, r)
	}
	return result
}

// TrustChain returns the trust chain of the CA, needed to verify certificates issued by this CA.
// The first certificate contains the CA signing key. The last certificate contains the
// trust root and is typically self-signed. In between are the certificates of
// intermediate CAs, if any. If the CA uses a self-signed certificate, the trust chain
// consists of that certificate only.
func (l *CertificateAuthority) TrustChain() []*x509.Certificate {
	result := make([]*x509.Certificate, 0)
	for _, c := range l.trustChain {
		result = append(result, c)
	}
	return result
}

// Verify checks the supplied certificate using the CA trust chain
func (l *CertificateAuthority) Verify(cert *x509.Certificate) (bool, error) {
	chains, err := l.getValidTrustChains(cert, l.trustChain)
	valid := (err == nil) && (chains != nil) && len(chains) > 0
	return valid, err
}

// IsReady returns true when the CA is ready to serve requests, false otherwise
func (l *CertificateAuthority) IsReady() bool {
	return l.ready
}

// NewCertificateAuthority provides a new instance of Certificate Services
// If no existing keys are found and bootstrap is true, the CA is bootstrapped with a
// generated private key and a self-signed certificate, otherwise an error is returned
func NewCertificateAuthority(km *keymgr.KeyMgr, bootstrap bool) (*CertificateAuthority, error) {
	if km == nil {
		return nil, errors.New("KeyMgr instance is required")
	}

	ca := &CertificateAuthority{
		keyMgr: km,
		ready:  false,
	}

	err := ca.init(bootstrap)
	if err != nil || !ca.ready {
		return nil, errors.Wrap(err, "CA failed to start")
	}

	return ca, nil
}
