// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package utils

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"fmt"

	"github.com/pensando/sw/venice/utils/certs"
)

// GetCAKit returns a minimal set of CA artifcacts for tests that need to generate TLS credentials
func GetCAKit() (certs.CSRSigner, *x509.Certificate, []*x509.Certificate, error) {
	caKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		return nil, nil, nil, fmt.Errorf("Error generating CA key: %v", err)
	}
	caCert, err := certs.SelfSign("ca", caKey, certs.WithValidityDays(1))
	if err != nil {
		return nil, nil, nil, fmt.Errorf("Error generating self-signer cert: %v", err)
	}

	csrSigner := func(csr *x509.CertificateRequest) (*x509.Certificate, error) {
		return certs.SignCSRwithCA(csr, caCert, caKey, certs.WithValidityDays(1))
	}

	return csrSigner, caCert, []*x509.Certificate{caCert}, nil
}
