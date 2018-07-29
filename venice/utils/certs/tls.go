// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package certs

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/tls"
	"crypto/x509"
	"crypto/x509/pkix"
	"fmt"
	"net"
	"os"
	"path"
	"strings"

	"github.com/pkg/errors"
)

const (
	tlsCredentialsCertFileName     = "cert.pem"
	tlsCredentialsKeyFileName      = "key.pem"
	tlsCredentialsCaBundleFileName = "ca-bundle.pem"
)

// CSRSigner is a function that takes a CSR and returns a signed certificate
type CSRSigner func(*x509.CertificateRequest) (*x509.Certificate, error)

// GetTLSCredentialsPaths gets the file paths for a TLS config stored using StoreTLSCredentials()
func GetTLSCredentialsPaths(dir string) (certPath, keyPath, caBundlePath string) {
	certPath = path.Join(dir, tlsCredentialsCertFileName)
	keyPath = path.Join(dir, tlsCredentialsKeyFileName)
	caBundlePath = path.Join(dir, tlsCredentialsCaBundleFileName)
	return
}

// StoreTLSCredentials stores TLS credentials in a dedicated directory.
func StoreTLSCredentials(cert *x509.Certificate, key crypto.PrivateKey, caBundle []*x509.Certificate, dir string, perm os.FileMode) error {

	if cert == nil || key == nil || caBundle == nil {
		return fmt.Errorf("All parameters required")
	}

	err := os.MkdirAll(dir, perm)
	if err != nil {
		return errors.Wrapf(err, "Error creating director: %v with perm %v", dir, perm)
	}

	certPath, keyPath, caBundlePath := GetTLSCredentialsPaths(dir)
	// Clean up all files in case of error.
	// Do not delete directory because it may have not been created by us
	defer func(err error) {
		if err != nil {
			DeleteTLSCredentials(dir)
		}
	}(err)

	err = SaveCertificate(certPath, cert)
	if err != nil {
		return err
	}
	err = SaveCertificates(caBundlePath, caBundle)
	if err != nil {
		return err
	}
	err = SavePrivateKey(keyPath, key)
	if err != nil {
		return err
	}
	return nil
}

// LoadTLSCredentials loads TLS credentials stored using StoreTLSCredentials() and returns the correspondig tls.Config object
func LoadTLSCredentials(dir string) (*tls.Config, error) {
	certPath, keyPath, caBundlePath := GetTLSCredentialsPaths(dir)
	tlsCert, err := tls.LoadX509KeyPair(certPath, keyPath)
	if err != nil {
		return nil, errors.Wrapf(err, "Error loading key pair, cert path: %v, key path: %v", certPath, keyPath)
	}

	caBundle, err := ReadCertificates(caBundlePath)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading CA bundle, file: %v", caBundlePath)
	}

	tlsCredentials := tls.Config{
		Certificates: []tls.Certificate{tlsCert},
		RootCAs:      NewCertPool(caBundle),
	}

	return &tlsCredentials, nil
}

// CreateTLSCredentials creates a full set of mTLS credentials (private key, certificate and CA trust bundle)
// to validate peer cert and stores it on the filesystem in the specified directory.
// The csrSigner parameter is a function that takes a CSR and returns a signed certificate.
// trustRoots is the certificate bundle used by client to verify server certificate.
// Crypto parameters and file-system permissions are hard-coded to reasonable values but can be parametrized if needed.
func CreateTLSCredentials(dir string, subject *pkix.Name, dnsnames []string, ipaddrs []net.IP, trustRoots []*x509.Certificate, csrSigner CSRSigner) error {
	if csrSigner == nil {
		return errors.New("Cannot generate TLS credentials without a CSR signer")
	}
	kvsPrivKey, err := ecdsa.GenerateKey(elliptic.P384(), rand.Reader)
	if err != nil {
		return errors.Wrapf(err, "error generating private key")
	}

	csr, err := CreateCSR(kvsPrivKey, subject, dnsnames, ipaddrs)
	if err != nil {
		return errors.Wrapf(err, "error generating csr")
	}
	cert, err := csrSigner(csr)
	if err != nil {
		return errors.Wrapf(err, "error generating certificate")
	}

	err = StoreTLSCredentials(cert, kvsPrivKey, trustRoots, dir, 0700)
	if err != nil {
		return errors.Wrapf(err, "error storing TLS credentials")
	}

	return nil
}

// DeleteTLSCredentials deletes the TLS credentials stored in the specifies directory
// It does not delete any other file or the directory itself
func DeleteTLSCredentials(dir string) error {
	certPath, keyPath, caBundlePath := GetTLSCredentialsPaths(dir)

	failures := make([]string, 0)
	err := os.RemoveAll(keyPath)
	if err != nil {
		failures = append(failures, fmt.Sprintf("Error deleting %s: %v", keyPath, err))
	}
	err = os.RemoveAll(certPath)
	if err != nil {
		failures = append(failures, fmt.Sprintf("Error deleting %s: %v", certPath, err))
	}
	err = os.RemoveAll(caBundlePath)
	if err != nil {
		failures = append(failures, fmt.Sprintf("Error deleting %s: %v", caBundlePath, err))
	}
	if len(failures) > 0 {
		return fmt.Errorf("%s", strings.Join(failures, ","))
	}
	return nil
}
