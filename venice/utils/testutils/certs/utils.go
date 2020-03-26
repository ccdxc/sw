package certs

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/tls"
	"crypto/x509"
	"encoding/pem"
	"errors"
	"fmt"
	"net/url"
	"strings"
	"testing"

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

// GetClientCertificate creates a self-signed client certificate with URI provided as argument
func GetClientCertificate(t *testing.T, uri string) (*x509.Certificate, error) {
	cert, _, err := GetTokenContents(t, uri)
	return cert, err
}

// GetTokenContents creates a self-signed client certificate and returns it along with the private key
func GetTokenContents(t *testing.T, uri string) (*x509.Certificate, *ecdsa.PrivateKey, error) {
	caKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		return nil, nil, errors.New("Error generating CA key")
	}
	caCert, err := certs.SelfSign("", caKey, certs.WithValidityDays(1))
	if err != nil {
		return nil, nil, errors.New("Error generating CA cert")
	}
	privateKey, err := ecdsa.GenerateKey(elliptic.P384(), rand.Reader)
	if err != nil {
		return nil, nil, errors.New("Error generating private key")
	}

	var URIs []*url.URL

	audienceURI, err := url.Parse(uri)
	if err != nil {
		return nil, nil, fmt.Errorf("Error parsing audience URI %s", uri)
	}
	URIs = append(URIs, audienceURI)
	template := &x509.CertificateRequest{URIs: URIs}
	csrBytes, err := x509.CreateCertificateRequest(rand.Reader, template, privateKey)
	if err != nil {
		return nil, nil, errors.New("Error creating CSR")
	}
	csr, err := x509.ParseCertificateRequest(csrBytes)
	if err != nil {
		return nil, nil, errors.New("Error parsing CSR")
	}
	clientCert, err := certs.SignCSRwithCA(csr, caCert, caKey, certs.WithValidityDays(1))
	if err != nil {
		return nil, nil, errors.New("Error signing certificate")
	}
	return clientCert, privateKey, nil
}

//GetVeniceIssuedClientCertificate creates a self-signed test certificate mimicing Venice
func GetVeniceIssuedClientCertificate(t *testing.T, audience []string) (*x509.Certificate, error) {
	if len(audience) == 0 {
		return nil, errors.New("Can not generate a test certificate for empty audience")
	}
	audienceURIStr := fmt.Sprintf("venice://%s/%s?%s", "test_cluster", "audience", strings.Join(audience, "&"))
	return GetClientCertificate(t, audienceURIStr)
}

//GetVeniceIssuedTokenContents creates a self-signed test certificate mimicing Venice
func GetVeniceIssuedTokenContents(t *testing.T, audience []string) (*x509.Certificate, *ecdsa.PrivateKey, error) {
	if len(audience) == 0 {
		return nil, nil, errors.New("Can not generate a test certificate for empty audience")
	}
	audienceURIStr := fmt.Sprintf("venice://%s/%s?%s", "test_cluster", "audience", strings.Join(audience, "&"))
	return GetTokenContents(t, audienceURIStr)
}
