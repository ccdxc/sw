// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package certs

import (
	"crypto"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"os"
	"path"

	"github.com/pkg/errors"
)

const (
	tlsCredentialsCertFileName     = "cert.pem"
	tlsCredentialsKeyFileName      = "key.pem"
	tlsCredentialsCaBundleFileName = "ca-bundle.pem"
)

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
			os.RemoveAll(certPath)
			os.RemoveAll(keyPath)
			os.RemoveAll(caBundlePath)
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
