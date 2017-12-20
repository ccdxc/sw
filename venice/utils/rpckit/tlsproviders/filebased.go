// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tlsproviders

import (
	"crypto"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"

	"github.com/pkg/errors"
)

const (
	privateKeyFileName  = "key.pem"
	certificateFileName = "cert.pem"
	trustRootsFileName  = "roots.pem"
)

// FileBasedProvider is a TLS Provider which assumes keys and certs are available as files
type FileBasedProvider struct {
	privateKey  crypto.PrivateKey
	certificate tls.Certificate
	trustRoots  x509.CertPool
}

// NewFileBasedProvider instantiates a new file-based TLS provider
func NewFileBasedProvider(certificatePath, privateKeyPath, trustRootsPath string) (*FileBasedProvider, error) {

	// Make sure both certFile and keyFile are specified or none
	if certificatePath == "" || privateKeyPath == "" {
		return nil, errors.New("Requires both cert file and key file")
	}

	certificate, err := tls.LoadX509KeyPair(certificatePath, privateKeyPath)
	if err != nil {
		return nil, errors.Wrap(err, fmt.Sprintf("Failed to read cert and key files(%s/%s)", certificatePath, privateKeyPath))
	}

	certPool := x509.NewCertPool()
	bs, err := ioutil.ReadFile(trustRootsPath)
	if err != nil {
		return nil, errors.Wrap(err, fmt.Sprintf("Failed to read ca cert: %v", trustRootsPath))
	}

	ok := certPool.AppendCertsFromPEM(bs)
	if !ok {
		return nil, errors.Wrap(err, fmt.Sprintf("Failed to append certs from file %v", trustRootsPath))
	}

	return &FileBasedProvider{
		privateKey:  certificate.PrivateKey,
		certificate: certificate,
		trustRoots:  *certPool,
	}, nil
}

// GetServerOptions returns server options to be passed to grpc.NewServer()
func (p *FileBasedProvider) GetServerOptions(serverName string) (grpc.ServerOption, error) {
	// serverName tells the TLS provider what to put in the certificate as a subject.
	// FileBasedProvider does not mint its own certificates, it just reads them from disk,
	// so it does not use it.
	tlsConfig := getTLSServerConfig(serverName, &p.certificate, &p.trustRoots)
	return grpc.Creds(credentials.NewTLS(tlsConfig)), nil
}

// GetDialOptions returns dial options to be passed to grpc.Dial()
func (p *FileBasedProvider) GetDialOptions(serverName string) (grpc.DialOption, error) {
	tlsConfig := getTLSClientConfig(serverName, &p.certificate, &p.trustRoots)
	return grpc.WithTransportCredentials(credentials.NewTLS(tlsConfig)), nil
}

// Close is no-op for this provider
func (p *FileBasedProvider) Close() {
}
