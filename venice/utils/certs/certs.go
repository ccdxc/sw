// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certs

import (
	"bytes"
	"crypto"
	"crypto/ecdsa"
	"crypto/rand"
	"crypto/rsa"
	"crypto/sha1"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/asn1"
	"encoding/pem"
	"fmt"
	"io"
	"io/ioutil"
	"math/big"
	"net"
	"os"
	"reflect"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/netutils"
)

const (
	certificateRequestPemBlockType = "CERTIFICATE REQUEST"
	ecParametersPemBlockType       = "EC PARAMETERS"
	ecPrivateKeyPemBlockType       = "EC PRIVATE KEY"
	rsaPrivateKeyPemBlockType      = "RSA PRIVATE KEY"
)

type options struct {
	notBefore time.Time
	notAfter  time.Time
	days      int
}

// WithNotBefore specifies the time at which the certificate starts to be valid
func WithNotBefore(nb time.Time) Option {
	return func(o *options) {
		o.notBefore = nb
	}
}

// WithNotAfter specifies the time at which the certificate stops to be valid
func WithNotAfter(na time.Time) Option {
	return func(o *options) {
		o.notAfter = na
	}
}

// WithValidityDays specifies the number of days the certificate is valid for.
// The validity start or stop date can be specified using WithNotBefore or WithNotAfter respectively
func WithValidityDays(days int) Option {
	return func(o *options) {
		o.days = days
	}
}

func applyOptions(cert *x509.Certificate, opts ...Option) error {
	var t options
	for _, o := range opts {
		if o != nil {
			o(&t)
		}
	}

	/*  Allowed validity parameter combinations

	    |---------------------------------------------------------------|
	    |    input params       |    NotBefore      |    NotAfter       |
	    |-----------------------|-------------------|-------------------|
	    |  Days                 |  Now              |  Now + days       |
	    |  NotBefore, Days      |  NotBefore        |  NotBefore + days |
	    |  NotAfter, Days       |  NotAfter - days  |  NotAfter         |
	    |  NotBefore, NotAfter  |  NotBefore        |  NotAfter         |
	    |---------------------------------------------------------------|

	    All other combinations are not valid
	*/

	if t.days != 0 && t.notBefore.IsZero() && t.notAfter.IsZero() {
		cert.NotBefore = time.Now()
		cert.NotAfter = cert.NotBefore.AddDate(0, 0, t.days)
	} else if t.days != 0 && !t.notBefore.IsZero() && t.notAfter.IsZero() {
		cert.NotBefore = t.notBefore
		cert.NotAfter = cert.NotBefore.AddDate(0, 0, t.days)
	} else if t.days != 0 && t.notBefore.IsZero() && !t.notAfter.IsZero() {
		cert.NotAfter = t.notAfter
		cert.NotBefore = cert.NotAfter.AddDate(0, 0, -t.days)
	} else if t.days == 0 && !t.notBefore.IsZero() && !t.notAfter.IsZero() {
		cert.NotBefore = t.notBefore
		cert.NotAfter = t.notAfter
	} else {
		return fmt.Errorf("Invalid parameters combinations: %+v", t)
	}

	if !cert.NotBefore.Before(cert.NotAfter) {
		return fmt.Errorf("NotAfter has to be later than NotBefore: %+v", t)
	}

	return nil
}

// Option fills the optional params for SelfSign and CreateCSR APIs
type Option func(opt *options)

func saveRsaPrivateKey(pemfile io.Writer, privatekey *rsa.PrivateKey) error {
	pemkey := &pem.Block{
		Type:  rsaPrivateKeyPemBlockType,
		Bytes: x509.MarshalPKCS1PrivateKey(privatekey)}
	return pem.Encode(pemfile, pemkey)
}

func saveEcPrivateKey(pemfile io.Writer, privatekey *ecdsa.PrivateKey) error {
	outputKey, err := x509.MarshalECPrivateKey(privatekey)
	if err != nil {
		return errors.Wrap(err, "Invalid ECDSA private key during SavePrivateKey")
	}

	pemkey := &pem.Block{
		Type:  ecPrivateKeyPemBlockType,
		Bytes: outputKey}
	return pem.Encode(pemfile, pemkey)
}

// SavePrivateKey saves private key to a file
func SavePrivateKey(outFile string, privatekey crypto.PrivateKey) error {
	pemfile, err := os.Create(outFile)
	if err != nil {
		return errors.Wrap(err, "Unable to create file during SavePrivateKey")
	}
	defer pemfile.Close()

	switch keyType := privatekey.(type) {
	case *rsa.PrivateKey:
		return saveRsaPrivateKey(pemfile, privatekey.(*rsa.PrivateKey))
	case *ecdsa.PrivateKey:
		return saveEcPrivateKey(pemfile, privatekey.(*ecdsa.PrivateKey))
	default:
		return errors.Wrapf(err, "Unknown key type: %T. Only RSA and ECDSA keys are supported", keyType)
	}
}

func readRsaPrivateKey(data []byte) (crypto.PrivateKey, error) {
	privatekey, err := x509.ParsePKCS1PrivateKey(data)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Parse RSA Private Key from PEM block.")
	}
	return privatekey, err
}

func readEcPrivateKey(key, params []byte) (crypto.PrivateKey, error) {
	// Golang crypto library assumes that key parameters are always specified in the key block itself.
	// If we find a separate "EC PARAMETERS" block, we check the syntax but otherwise ignore it.
	if params != nil {
		namedCurveOID := new(asn1.ObjectIdentifier)
		_, err := asn1.Unmarshal(params, namedCurveOID)
		if err != nil {
			return nil, errors.Wrap(err, "Failed to parse EC curve parameters from PEM block.")
		}
	}

	privateKey, err := x509.ParseECPrivateKey(key)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Parse EC Private Key from PEM block.")
	}

	return privateKey, err
}

// ReadPrivateKey from a file
func ReadPrivateKey(caKeyFile string) (crypto.PrivateKey, error) {
	privKeyBytes, err := ioutil.ReadFile(caKeyFile)
	if err != nil {
		return nil, errors.Wrap(err, "Unable to read private key")
	}

	block, rest := pem.Decode(privKeyBytes)
	if block == nil {
		return nil, errors.New("PEM block is nil while decoding privateKey")
	}

	switch block.Type {
	case rsaPrivateKeyPemBlockType:
		return readRsaPrivateKey(block.Bytes)

	case ecPrivateKeyPemBlockType:
		return readEcPrivateKey(block.Bytes, nil)

	case ecParametersPemBlockType:
		keyBlock, _ := pem.Decode(rest)
		if keyBlock == nil || keyBlock.Type != ecPrivateKeyPemBlockType {
			return nil, errors.New("No EC PRIVATE KEY block found")
		}
		return readEcPrivateKey(keyBlock.Bytes, block.Bytes)

	default:
		return nil, errors.New("No keys found in PEM block")
	}
}

// SaveCertificates to a file
func SaveCertificates(certFile string, certs []*x509.Certificate) error {
	pemFile, err := os.Create(certFile)
	if err != nil {
		return errors.Wrap(err, "Unable to create file during SaveCertificate")
	}
	defer pemFile.Close()

	for _, cert := range certs {
		pemBlock := &pem.Block{
			Type:  "CERTIFICATE",
			Bytes: cert.Raw,
		}
		err := pem.Encode(pemFile, pemBlock)
		if err != nil {
			return err
		}
	}
	return nil
}

// SaveCertificate to a file
func SaveCertificate(certFile string, cert *x509.Certificate) error {
	return SaveCertificates(certFile, []*x509.Certificate{cert})
}

// ReadCertificates from a file
func ReadCertificates(certFile string) ([]*x509.Certificate, error) {
	var result []*x509.Certificate
	bytes, err := ioutil.ReadFile(certFile)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to read Certificate.")
	}
	for {
		if len(bytes) == 0 {
			break
		}
		block, rest := pem.Decode(bytes)
		if block == nil {
			return nil, errors.New("PEM block is nil while decoding certificate")
		}
		if block.Type != "CERTIFICATE" {
			return nil, errors.New("CERTIFICATE not found in PEM block")
		}
		cert, err := x509.ParseCertificates(block.Bytes)
		if err != nil {
			return nil, errors.Wrap(err, "Failed to Parse CERT from PEM block.")
		}
		result = append(result, cert...)
		bytes = rest
	}
	return result, err
}

// ReadCertificate reads exactly 1 certificate from a file.
// It fails if it finds no certificate or more than 1 certificate in the file
func ReadCertificate(certFile string) (*x509.Certificate, error) {
	readCerts, err := ReadCertificates(certFile)
	if err != nil {
		return nil, err
	}
	if len(readCerts) < 1 || len(readCerts) > 1 {
		return nil, fmt.Errorf("Error reading certificate: expected 1, found: %v", len(readCerts))
	}
	return readCerts[0], nil
}

// SaveCSR to a file
func SaveCSR(certFile string, cert *x509.CertificateRequest) error {
	pemfile, _ := os.Create(certFile)
	defer pemfile.Close()
	pemkey := &pem.Block{
		Type:  certificateRequestPemBlockType,
		Bytes: cert.Raw}
	return pem.Encode(pemfile, pemkey)
}

// ReadCSR from a file
func ReadCSR(csrFile string) (*x509.CertificateRequest, error) {
	bytes, err := ioutil.ReadFile(csrFile)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to read CSR.")
	}
	block, _ := pem.Decode(bytes)
	if block == nil {
		return nil, errors.New("PEM block is nil while decoding CSR")
	}
	if block.Type != certificateRequestPemBlockType {
		return nil, errors.New("CERTIFICATE  REQUEST not found in PEM block")
	}
	csr, err := x509.ParseCertificateRequest(block.Bytes)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Parse CSR after creation.")
	}
	return csr, err

}

// SelfSign generates a self-signed certificate valid for the specified number of days using the supplied private key.
// If hostname is not provided, it populates it using os.Hostname()
func SelfSign(hostname string, privatekey crypto.PrivateKey, opts ...Option) (*x509.Certificate, error) {
	var err error
	if hostname == "" {
		hostname, err = os.Hostname()
		if err != nil {
			hostname = "*"
		}
	}

	template := &x509.Certificate{
		IsCA: true,
		BasicConstraintsValid: true,
		SubjectKeyId:          []byte{1, 2, 3},
		SerialNumber:          big.NewInt(1234),
		Subject: pkix.Name{
			CommonName: hostname,
		},
		ExtKeyUsage: []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth, x509.ExtKeyUsageServerAuth},
		KeyUsage:    x509.KeyUsageDigitalSignature | x509.KeyUsageCertSign,
		DNSNames:    []string{hostname},
	}

	err = applyOptions(template, opts...)
	if err != nil {
		return nil, errors.Wrapf(err, "Error applying certificate options")
	}
	serialNumberLimit := new(big.Int).Lsh(big.NewInt(1), 128)
	template.SerialNumber, err = rand.Int(rand.Reader, serialNumberLimit)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Generate Random number during Self-signing certificate.")
	}

	var publickey crypto.PublicKey
	switch keyType := privatekey.(type) {
	case *rsa.PrivateKey:
		publickey = privatekey.(*rsa.PrivateKey).Public()
	case *ecdsa.PrivateKey:
		publickey = privatekey.(*ecdsa.PrivateKey).Public()
	case crypto.Signer:
		publickey = privatekey.(crypto.Signer).Public()
	default:
		return nil, errors.Wrapf(err, "Unknown private key type: %T", keyType)
	}

	// create a self-signed certificate. template = parent
	parent := template
	bytes, err := x509.CreateCertificate(rand.Reader, template, parent, publickey, privatekey)
	if err != nil {
		return nil, errors.Wrap(err, "Failed Creating certificate")
	}

	cert, err := x509.ParseCertificates(bytes)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Parse CERT after creation.")
	}
	return cert[0], nil
}

// CreateCSR for my host and IP
func CreateCSR(privatekey crypto.PrivateKey, dnsnames []string, ipaddrs []net.IP) (*x509.CertificateRequest, error) {
	hostname, err := os.Hostname()
	if err != nil {
		return nil, errors.Wrap(err, "Unable to determine hostname")
	}

	// if both the dnsnames and ipaddrs are nil, caller wants us to populate with FQDN and IP address
	if dnsnames == nil && ipaddrs == nil {
		dnsnames, ipaddrs = netutils.NameAndIPs()
	}
	if dnsnames == nil && ipaddrs == nil {
		return nil, errors.New("Failed to determine Hostname and IP addresses")
	}

	template := &x509.CertificateRequest{
		Subject: pkix.Name{
			CommonName: hostname,
		},
		DNSNames:    dnsnames,
		IPAddresses: ipaddrs,
	}

	bytes, err := x509.CreateCertificateRequest(rand.Reader, template, privatekey)

	if err != nil {
		return nil, errors.Wrap(err, "Failed Creating CSR")
	}
	csr, err := x509.ParseCertificateRequest(bytes)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Parse CSR after creation.")
	}
	return csr, err

}

// SignCSRwithCA signs the CSR for specified number of days, by the CA privateKey and CA certificate
// Any security checks (whether CSR should be signed or not) are done by the caller
func SignCSRwithCA(csr *x509.CertificateRequest, caCert *x509.Certificate, privatekey crypto.PrivateKey, opts ...Option) (*x509.Certificate, error) {
	publickey := csr.PublicKey

	template := &x509.Certificate{
		IsCA: false,
		BasicConstraintsValid: true,
		Subject:               csr.Subject,
		ExtKeyUsage:           []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth, x509.ExtKeyUsageServerAuth},
		KeyUsage:              x509.KeyUsageDataEncipherment | x509.KeyUsageKeyEncipherment | x509.KeyUsageDigitalSignature,

		RawSubject:  csr.RawSubject,
		DNSNames:    csr.DNSNames,
		IPAddresses: csr.IPAddresses,
	}

	err := applyOptions(template, opts...)
	if err != nil {
		return nil, errors.Wrapf(err, "Error applying certificate options")
	}
	serialNumberLimit := new(big.Int).Lsh(big.NewInt(1), 128)
	template.SerialNumber, err = rand.Int(rand.Reader, serialNumberLimit)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Generate Random number during Certificate signing.")
	}
	pubBytes, err := x509.MarshalPKIXPublicKey(publickey)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Marshall public key during Certificate signing.")
	}
	hash := sha1.Sum(pubBytes)
	template.SubjectKeyId = hash[:]

	bytes, err := x509.CreateCertificate(rand.Reader, template, caCert, publickey, privatekey)

	if err != nil {
		return nil, errors.Wrap(err, "Failed Creating certificate")
	}
	cert, err := x509.ParseCertificates(bytes)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Parse CERT after creation.")
	}
	return cert[0], err

}

// IsSelfSigned returns whether the certificate is self-signed or not
func IsSelfSigned(cert *x509.Certificate) bool {
	return bytes.Equal(cert.RawIssuer, cert.RawSubject)
}

// ValidateKeyCertificatePair checks whether a private key matches the public key in a certificate
func ValidateKeyCertificatePair(privateKey crypto.PrivateKey, cert *x509.Certificate) (bool, error) {
	switch keyType := privateKey.(type) {
	case *rsa.PrivateKey:
		expectedPubKey := privateKey.(*rsa.PrivateKey).PublicKey
		return reflect.DeepEqual(cert.PublicKey, &expectedPubKey), nil
	case *ecdsa.PrivateKey:
		expectedPubKey := privateKey.(*ecdsa.PrivateKey).PublicKey
		return reflect.DeepEqual(cert.PublicKey, &expectedPubKey), nil
	case crypto.Signer:
		expectedPubKey := privateKey.(crypto.Signer).Public()
		return reflect.DeepEqual(cert.PublicKey, expectedPubKey), nil
	default:
		return false, errors.Errorf("Unknown key type: %T. Only RSA and ECDSA keys are supported", keyType)
	}
}

// NewCertPool creates a new x509.CertPool and adds the supplied certificates to it
func NewCertPool(certs []*x509.Certificate) *x509.CertPool {
	cp := x509.NewCertPool()
	for _, c := range certs {
		cp.AddCert(c)
	}
	return cp
}
