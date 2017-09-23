// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certmgr

import (
	"crypto"
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"fmt"
	"os"
	"path"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// The size of the private key to use if we are signing our own root certificate
	caSelfSignedKeySize = 4096

	// The validity period of the self-signed root certificate (1 year and 10 days)
	caSelfSignedCertDurationDays = 375

	// The validity period of the certificates that we issue to cluster nodes
	nodeCertDurationDays = 33

	// The names of the files where we store crypto artifacts
	// FIXME: introduce a proper storage interface that can be implemented by
	//        multiple providers
	caKeyFileName         = "ca.key.pem"
	caCertificateFileName = "ca.cert.pem"
	intermediatesFileName = "intermediates.cert.pem"
	trustRootsFileName    = "roots.pem"
)

// CertificateAuthority is the interface for issuing certificates
// Most of the methods are made available via a gRPC endpoint by rpcserver
type CertificateAuthority struct {
	configDir     string
	caKey         crypto.PrivateKey
	caCertificate x509.Certificate
	trustChain    []*x509.Certificate
	trustRoots    []*x509.Certificate
	issuedCerts   []*x509.Certificate
	ready         bool
}

// Calls Golang's x509.Verify() function to search for a valid trust chain from cert
// to a trust root using known intermediate certificates
func (l *CertificateAuthority) getValidTrustChains(cert *x509.Certificate) ([][]*x509.Certificate, error) {
	// For the moment we just check signatures and times.
	// We accept whatever Common Name or SAN is there.
	// We will have more complex validation logic later.
	options := x509.VerifyOptions{
		Roots:         certs.NewCertPool(l.trustRoots),
		Intermediates: certs.NewCertPool(l.trustChain),
		KeyUsages:     []x509.ExtKeyUsage{x509.ExtKeyUsageAny},
	}

	// TODO: Docker Swarmkit has a more strict validation algorithm than Verify()
	//       See if we want to use that one instead
	return cert.Verify(options)
}

// initializes a certificate services instance
func (l *CertificateAuthority) init() error {
	caKeyPath := path.Join(l.configDir, caKeyFileName)
	caCertificatePath := path.Join(l.configDir, caCertificateFileName)

	caKey, err := certs.ReadPrivateKey(caKeyPath)
	// If we fail to read an existing private key, we bootstrap the CA with
	// a generated private key and a self-signed certificate
	if err != nil {
		caKey, err = rsa.GenerateKey(rand.Reader, caSelfSignedKeySize)
		if err != nil {
			return errors.Wrap(err, fmt.Sprintf("Error generating signing key, path: %v", caKeyPath))
		}
		err = certs.SavePrivateKey(caKeyPath, caKey)
		if err != nil {
			return errors.Wrap(err, fmt.Sprintf("Error storing generated signing key, path: %v", caKeyPath))
		}
		err = os.Chmod(caKeyPath, 0600) // drop permissions
		if err != nil {
			return errors.Wrap(err, fmt.Sprintf("Error changing permissions for signing key file: %v", caKeyPath))
		}

		selfSignedCert, err := certs.SelfSign(caSelfSignedCertDurationDays, "CKM ROOT CA", caKey)
		if err != nil {
			return errors.Wrap(err, "Error generating self-signed certificate")
		}
		certs.SaveCertificate(caCertificatePath, selfSignedCert)
		if err != nil {
			return errors.Wrap(err, fmt.Sprintf("Error storing self-signed certificate, path: %v", caCertificatePath))
		}
	}
	l.caKey = caKey

	// read CA certificate
	readCert, err := certs.ReadCertificate(caCertificatePath)
	if err != nil {
		return errors.Wrap(err, "Error reading CA certificate")
	}
	l.caCertificate = *readCert

	// validate that public and private key match
	valid, err := certs.ValidateKeyCertificatePair(l.caKey, &l.caCertificate)
	if !valid || err != nil {
		return errors.Wrap(err, "Public and private key do not match")
	}

	// load intermediates if present
	intermediatesPath := path.Join(l.configDir, intermediatesFileName)
	intermediates, err := certs.ReadCertificates(intermediatesPath)
	if err != nil {
		log.Infof("Loaded %v intermediates certificates", len(intermediates))
	}

	// Load trust roots if present
	trustRootsPath := path.Join(l.configDir, trustRootsFileName)
	readCerts, err := certs.ReadCertificates(trustRootsPath)
	if err == nil {
		for _, cert := range readCerts {
			l.trustRoots = append(l.trustRoots, cert)
		}
	}
	log.Infof("Loaded %v trusted roots", len(l.trustRoots))

	// if our cert is self-signed, add ourselves to trustRoots
	if certs.IsSelfSigned(&l.caCertificate) {
		l.trustRoots = append(l.trustRoots, &l.caCertificate)
		log.Infof("Added self-signed certificate to trusted roots")
	}

	// form and validate the CA trust chain
	tcs, err := l.getValidTrustChains(&l.caCertificate)
	if err != nil || len(tcs) < 1 {
		return fmt.Errorf("Error establishing trust chain for the CA: %v", err)
	}
	l.trustChain = tcs[0]
	log.Infof("CA trust chain length: %v", len(l.trustChain))

	l.ready = true
	return nil
}

// Sign validates and sign a certificate signing request (CSR)
func (l *CertificateAuthority) Sign(csr *x509.CertificateRequest) (*x509.Certificate, error) {
	cert, err := certs.SignCSRwithCA(nodeCertDurationDays, csr, &l.caCertificate, l.caKey)
	if err != nil {
		return nil, errors.Wrap(err, "Error signing CSR")
	}
	l.issuedCerts = append(l.issuedCerts, cert)
	return cert, err
}

// TrustRoots returns the trust roots that should be trusted by all members of the cluster.
// These include the root of the CA trust chain + other roots that should be
// trusted by cluster members, if any
func (l *CertificateAuthority) TrustRoots() []x509.Certificate {
	result := make([]x509.Certificate, 0)
	for _, r := range l.trustRoots {
		result = append(result, *r)
	}
	return result
}

// TrustChain returns the trust chain of the CA, needed to verify certificates issued by this CA.
// The first certificate contains the CA signing key. The last certificate contains the
// trust root and is typically self-signed. In between are the certificates of
// intermediate CAs, if any. If the CA uses a self-signed certificate, the trust chain
// consists of that certificate only.
func (l *CertificateAuthority) TrustChain() []x509.Certificate {
	result := make([]x509.Certificate, 0)
	for _, c := range l.trustChain {
		result = append(result, *c)
	}
	return result
}

// Verify checks the supplied certificate using the CA trust chain
func (l *CertificateAuthority) Verify(cert *x509.Certificate) (bool, error) {
	chains, err := l.getValidTrustChains(cert)
	valid := (err == nil) && (chains != nil) && len(chains) > 0
	return valid, err
}

// IsReady returns true when the CA is ready to serve requests, false otherwise
func (l *CertificateAuthority) IsReady() bool {
	return l.ready
}

// NewCertificateAuthority provides a new instance of Certificate Services
func NewCertificateAuthority(dir string) (*CertificateAuthority, error) {
	if dir == "" {
		return nil, errors.New("CA root directory is required")
	}

	dirInfo, err := os.Stat(dir)
	if err != nil && !os.IsNotExist(err) {
		return nil, errors.Wrapf(err, "Error getting permissions on root directory, path %v", dir)
	}
	if err == nil {
		// directory exists, we need to check permissions
		if !dirInfo.Mode().IsDir() {
			return nil, fmt.Errorf("Path %v exists but is not a directory", dir)
		}
		if (dirInfo.Mode() & 0777) != 0700 {
			return nil, fmt.Errorf("Root directory, path %v has invalid permissions. Expect: 0700, found %#o", dir, (dirInfo.Mode() & 0777))
		}
	}

	err = os.MkdirAll(dir, 0700)
	if err != nil {
		return nil, errors.Wrap(err, fmt.Sprintf("Could not open CA root directory, path: %v", dir))
	}

	ca := &CertificateAuthority{
		configDir: dir,
		ready:     false,
	}

	err = ca.init()
	if err != nil || !ca.ready {
		return nil, errors.Wrap(err, "CA failed to start")
	}

	return ca, nil
}
