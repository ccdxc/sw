// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package tlsproviders

import (
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"

	"github.com/pkg/errors"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"

	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/keymgr"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	// Well known IDs
	kmbClientBaseID   = "client"
	kmbCaTrustChainID = "ca-trust-chain-bundle"
	kmbTrustRootsID   = "trust-roots-bundle"
	// KeyMgrBasedDefaultBaseID is the base id that will be used if no client/server key id is found
	kmbDefaultBaseID = "default"
	// Suffixes are needed because KeyMgr does not allow objects of different types with the same name
	kmbCertIDSuffix = "-cert"
	kmbKeyIDSuffix  = "-key"
)

// KeyMgrBasedProvider is a TLS Provider which generates private keys locally and retrieves
// corresponding certificates from the Cluster Management Daemon (CMD)
type KeyMgrBasedProvider struct {
	// the KeyMgr instance used to generate and store keys and certificates
	keyMgr *keymgr.KeyMgr
}

func (p *KeyMgrBasedProvider) getTLSCertificate(id string) (*tls.Certificate, error) {
	privateKey, err := p.keyMgr.GetObject(id+kmbKeyIDSuffix, keymgr.ObjectTypeKeyPair)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading key pair %s", id)
	}

	cert, err := p.keyMgr.GetObject(id+kmbCertIDSuffix, keymgr.ObjectTypeCertificate)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading certificate %s", id)
	}

	if privateKey == nil || cert == nil {
		// incomplete credentials
		return nil, nil
	}

	caTrustChain, err := p.GetCaTrustChain()
	if err != nil {
		return nil, errors.Wrapf(err, "Error getting CA trust chain")
	}

	bundle := [][]byte{cert.(*keymgr.Certificate).Certificate.Raw}
	for _, c := range caTrustChain {
		bundle = append(bundle, c.Raw)
	}

	return &tls.Certificate{
		PrivateKey:  privateKey,
		Certificate: bundle,
	}, nil
}

func (p *KeyMgrBasedProvider) getTLSNamedCredentialsOrDefault(name string) (*tls.Certificate, *x509.CertPool, error) {
	var tlsCertificate *tls.Certificate
	var err error
	for _, id := range []string{name, kmbDefaultBaseID} {
		tlsCertificate, err = p.getTLSCertificate(id)
		if err != nil {
			return nil, nil, errors.Wrapf(err, "Error retrieving TLS certificate, id: %s", id)
		}
		if tlsCertificate != nil {
			break
		}
	}

	trustRoots, err := p.GetTrustRoots()
	if err != nil {
		return nil, nil, errors.Wrapf(err, "Error getting trust roots")
	}

	return tlsCertificate, trustRoots, nil
}

// GetCaTrustChain returns the CA trust chain that is passed to peers
func (p *KeyMgrBasedProvider) GetCaTrustChain() ([]*x509.Certificate, error) {
	caTrustChain, err := p.keyMgr.GetObject(kmbCaTrustChainID, keymgr.ObjectTypeCertificateBundle)
	if err != nil {
		return nil, errors.Wrapf(err, "Error retrieving CA trust chain")
	}
	if caTrustChain != nil {
		return caTrustChain.(*keymgr.CertificateBundle).Certificates, nil
	}
	return nil, nil
}

// GetTrustRoots returns the trust roots that are used to evaluate peer certificates
func (p *KeyMgrBasedProvider) GetTrustRoots() (*x509.CertPool, error) {
	trustRoots, err := p.keyMgr.GetObject(kmbTrustRootsID, keymgr.ObjectTypeCertificateBundle)
	if err != nil {
		log.Errorf("Error retrieving trust roots: %v", err)
	}
	if trustRoots != nil {
		return certs.NewCertPool(trustRoots.(*keymgr.CertificateBundle).Certificates), nil
	}
	return nil, nil
}

// GetServerOptions returns server options to be passed to grpc.NewServer()
func (p *KeyMgrBasedProvider) GetServerOptions(serverName string) (grpc.ServerOption, error) {
	tlsCertificate, trustRoots, err := p.getTLSNamedCredentialsOrDefault(serverName)
	if err != nil {
		log.Errorf("Error in GetServerOptions for server %s: %v", serverName, err)
		return nil, err
	}
	if tlsCertificate == nil {
		log.Errorf("No certificate found for server %s and no default set", serverName)
		return nil, fmt.Errorf("No certificate found for server %s and no default set", serverName)
	}
	tlsConfig := getTLSServerConfig(serverName, tlsCertificate, trustRoots)
	return grpc.Creds(credentials.NewTLS(tlsConfig)), nil
}

// GetDialOptions returns dial options to be passed to grpc.Dial()
func (p *KeyMgrBasedProvider) GetDialOptions(serverName string) (grpc.DialOption, error) {
	tlsCertificate, trustRoots, err := p.getTLSNamedCredentialsOrDefault(kmbClientBaseID)
	if err != nil {
		log.Errorf("Error in GetDialOptions(): %v", err)
		return nil, err
	}
	if tlsCertificate == nil {
		return nil, fmt.Errorf("No client certificate found and no default set, serverName: %s", serverName)
	}
	tlsConfig := getTLSClientConfig(serverName, tlsCertificate, trustRoots)
	return grpc.WithTransportCredentials(credentials.NewTLS(tlsConfig)), nil
}

/* --- functions below do not require locks because KeyMgr implementations are required to be thread-safe --- */

// CreateDefaultKeyPair creates a (public key, private key) pair to be used when no client/server-specific
// key pair is found.
func (p *KeyMgrBasedProvider) CreateDefaultKeyPair(keytype keymgr.KeyType) (*keymgr.KeyPair, error) {
	return p.keyMgr.UpdateKeyPair(kmbDefaultBaseID+kmbKeyIDSuffix, keytype)
}

// CreateClientKeyPair creates a (public key, private key) to return when GetDialOptions() is invoked
func (p *KeyMgrBasedProvider) CreateClientKeyPair(keytype keymgr.KeyType) (*keymgr.KeyPair, error) {
	return p.keyMgr.UpdateKeyPair(kmbClientBaseID+kmbKeyIDSuffix, keytype)
}

// CreateServerKeyPair creates a (public key, private key) to return when GetDialServerOptions() is invoked for a specific server
func (p *KeyMgrBasedProvider) CreateServerKeyPair(serverName string, keytype keymgr.KeyType) (*keymgr.KeyPair, error) {
	return p.keyMgr.UpdateKeyPair(serverName+kmbKeyIDSuffix, keytype)
}

// SetDefaultCertificate sets the certificate to be used when no client/server-specific server is found.
func (p *KeyMgrBasedProvider) SetDefaultCertificate(cert *x509.Certificate) error {
	return p.keyMgr.UpdateObject(keymgr.NewCertificateObject(kmbDefaultBaseID+kmbCertIDSuffix, cert))
}

// SetClientCertificate sets the certificate to return when GetDialOptions() is invoked
func (p *KeyMgrBasedProvider) SetClientCertificate(cert *x509.Certificate) error {
	return p.keyMgr.UpdateObject(keymgr.NewCertificateObject(kmbClientBaseID+kmbCertIDSuffix, cert))
}

// SetServerCertificate sets the certificate to return when GetServerOptions is invoked for a specific server
func (p *KeyMgrBasedProvider) SetServerCertificate(serverName string, cert *x509.Certificate) error {
	return p.keyMgr.UpdateObject(keymgr.NewCertificateObject(serverName+kmbCertIDSuffix, cert))
}

// SetCaTrustChain sets the trust chain to be supplied to a peer when establishing a connection
func (p *KeyMgrBasedProvider) SetCaTrustChain(certs []*x509.Certificate) error {
	return p.keyMgr.UpdateObject(keymgr.NewCertificateBundleObject(kmbCaTrustChainID, certs))
}

// SetTrustRoots sets the trust roots to be used when validating a peer certificate
func (p *KeyMgrBasedProvider) SetTrustRoots(certs []*x509.Certificate) error {
	return p.keyMgr.UpdateObject(keymgr.NewCertificateBundleObject(kmbTrustRootsID, certs))
}

// Close closes the client.
func (p *KeyMgrBasedProvider) Close() {
	if p.keyMgr != nil {
		p.keyMgr.Close()
		p.keyMgr = nil
	}
}

// NewKeyMgrBasedProvider instantiates a new CMD-based TLS provider
func NewKeyMgrBasedProvider(km *keymgr.KeyMgr) *KeyMgrBasedProvider {
	if km == nil {
		return nil
	}
	return &KeyMgrBasedProvider{keyMgr: km}
}

// NewDefaultKeyMgrBasedProvider instantiates a new keymgr-based TLS provider using a keymgr with default backend
func NewDefaultKeyMgrBasedProvider(id string) (*KeyMgrBasedProvider, error) {
	workDir, err := ioutil.TempDir("", "tlsprovider-"+id+"-")
	if err != nil {
		return nil, errors.Wrapf(err, fmt.Sprintf("Error creating workdir for GoCrypto backend %s", id))
	}
	be, err := keymgr.NewGoCryptoBackend(workDir)
	if err != nil {
		return nil, errors.Wrapf(err, "Error instantiating GoCrypto backend")
	}
	km, err := keymgr.NewKeyMgr(be)
	if err != nil {
		be.Close()
		return nil, errors.Wrapf(err, "Error instantiating keymgr")
	}
	return NewKeyMgrBasedProvider(km), nil
}
