package certs

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/tls"
	"crypto/x509"
	"encoding/pem"

	"github.com/pensando/sw/venice/utils/certs"
)

// GeneratePEMSelfSignedCertAndKey generates pem encoded EC key(256 bits) and self signed cert
func GeneratePEMSelfSignedCertAndKey(name string, validity int) (crypto.PrivateKey, string, *x509.Certificate, string, *tls.Certificate, error) {
	// PEM key
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		return nil, "", nil, "", nil, err
	}
	pemkey, err := GetPEMECKey(key)
	if err != nil {
		return nil, "", nil, "", nil, err
	}

	// PEM cert
	cert, err := certs.SelfSign(name, key, certs.WithValidityDays(validity))
	if err != nil {
		return nil, "", nil, "", nil, err
	}
	pemcert := GetPEMCert(cert)

	return key, pemkey, cert, pemcert, &tls.Certificate{Certificate: [][]byte{cert.Raw}, PrivateKey: key}, err
}

// GetPEMECKey gets PEM encoded ecdsa private key
func GetPEMECKey(key *ecdsa.PrivateKey) (string, error) {
	outputKey, err := x509.MarshalECPrivateKey(key)
	if err != nil {
		return "", err
	}
	b := &pem.Block{
		Type:  "EC PRIVATE KEY",
		Bytes: outputKey}
	pemkey := string(pem.EncodeToMemory(b))
	return pemkey, nil
}

// GetPEMCert gets PEM encoded x509 cert
func GetPEMCert(cert *x509.Certificate) string {
	b := &pem.Block{
		Type:  "CERTIFICATE",
		Bytes: cert.Raw}
	return string(pem.EncodeToMemory(b))
}
