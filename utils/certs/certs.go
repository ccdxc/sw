package certs

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/sha1"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/pem"
	"io/ioutil"
	"math/big"
	"net"
	"os"
	"time"

	netutils "github.com/pensando/sw/utils/net"
	"github.com/pkg/errors"
)

// SavePrivateKey saves private key to a file
func SavePrivateKey(outFile string, privatekey *rsa.PrivateKey) error {
	pemfile, err := os.Create(outFile)
	if err != nil {
		return errors.Wrap(err, "Unable to create file during SavePrivateKey")
	}
	defer pemfile.Close()

	pemkey := &pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: x509.MarshalPKCS1PrivateKey(privatekey)}
	return pem.Encode(pemfile, pemkey)
}

// ReadPrivateKey from a file
func ReadPrivateKey(caKeyFile string) (*rsa.PrivateKey, error) {
	privKeyBytes, err := ioutil.ReadFile(caKeyFile)
	if err != nil {
		return nil, errors.Wrap(err, "Unable to read private key")
	}
	block, _ := pem.Decode(privKeyBytes)
	if block == nil {
		return nil, errors.New("PEM block is nil while decoding privateKey")
	}
	if block.Type != "RSA PRIVATE KEY" {
		return nil, errors.New("RSA PRIVATE KEY not found in PEM block")
	}
	privatekey, err := x509.ParsePKCS1PrivateKey(block.Bytes)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Parse Private Key from PEM block.")
	}
	return privatekey, err
}

// SaveCertificate to a file
func SaveCertificate(certFile string, cert *x509.Certificate) error {
	pemfile, err := os.Create(certFile)
	if err != nil {
		return errors.Wrap(err, "Unable to create file during SaveCertificate")
	}
	defer pemfile.Close()

	pemkey := &pem.Block{
		Type:  "CERTIFICATE",
		Bytes: cert.Raw}
	return pem.Encode(pemfile, pemkey)
}

// ReadCertificates from a file
func ReadCertificates(certFile string) ([]*x509.Certificate, error) {
	bytes, err := ioutil.ReadFile(certFile)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to read Certificate.")
	}
	block, _ := pem.Decode(bytes)
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
	return cert, err
}

// SaveCSR to a file
func SaveCSR(certFile string, cert *x509.CertificateRequest) error {
	pemfile, _ := os.Create(certFile)
	defer pemfile.Close()
	pemkey := &pem.Block{
		Type:  "CERTIFICATE REQUEST",
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
	if block.Type != "CERTIFICATE REQUEST" {
		return nil, errors.New("CERTIFICATE  REQUEST not found in PEM block")
	}
	csr, err := x509.ParseCertificateRequest(block.Bytes)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Parse CSR after creation.")
	}
	return csr, err

}

// SelfSign CA privatekey for specified days
func SelfSign(days int, privatekey *rsa.PrivateKey) (*x509.Certificate, error) {
	hostname, err := os.Hostname()
	if err != nil {
		hostname = "*"
	}

	template := &x509.Certificate{
		IsCA: true,
		BasicConstraintsValid: true,
		SubjectKeyId:          []byte{1, 2, 3},
		SerialNumber:          big.NewInt(1234),
		Subject: pkix.Name{
			CommonName: hostname,
		},
		NotBefore:   time.Now(),
		NotAfter:    time.Now().AddDate(0, 0, days),
		ExtKeyUsage: []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth, x509.ExtKeyUsageServerAuth},
		KeyUsage:    x509.KeyUsageDigitalSignature | x509.KeyUsageCertSign,
	}
	serialNumberLimit := new(big.Int).Lsh(big.NewInt(1), 128)
	template.SerialNumber, err = rand.Int(rand.Reader, serialNumberLimit)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to Generate Random number during Self-signing certificate.")
	}

	publickey := &privatekey.PublicKey

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
func CreateCSR(privatekey *rsa.PrivateKey, dnsnames []string, ipaddrs []net.IP) (*x509.CertificateRequest, error) {
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
func SignCSRwithCA(days int, csr *x509.CertificateRequest, caCert *x509.Certificate, privatekey *rsa.PrivateKey) (*x509.Certificate, error) {
	publickey := csr.PublicKey

	template := &x509.Certificate{
		IsCA: false,
		BasicConstraintsValid: true,
		Subject:               csr.Subject,
		NotBefore:             time.Now(),
		NotAfter:              time.Now().AddDate(0, 0, days),
		ExtKeyUsage:           []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth, x509.ExtKeyUsageServerAuth},
		KeyUsage:              x509.KeyUsageDataEncipherment | x509.KeyUsageKeyEncipherment | x509.KeyUsageDigitalSignature,

		RawSubject:  csr.RawSubject,
		DNSNames:    csr.DNSNames,
		IPAddresses: csr.IPAddresses,
	}
	serialNumberLimit := new(big.Int).Lsh(big.NewInt(1), 128)
	var err error
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
