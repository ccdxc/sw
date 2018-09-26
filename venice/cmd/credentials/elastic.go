// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package credentials

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"crypto/x509/pkix"
	"net"
	"os"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/netutils"
)

func storeElasticCredentials(dir string, perm os.FileMode, key crypto.PrivateKey, cert *x509.Certificate, trustRoots []*x509.Certificate) error {
	err := os.MkdirAll(dir, perm)
	if err != nil {
		return errors.Wrapf(err, "Error creating directory: %v with permissions: %v", dir, perm)
	}

	certPath, keyPath, trustRootsPath := certs.GetTLSCredentialsPaths(dir)
	err = certs.SaveCertificate(certPath, cert)
	if err != nil {
		return err
	}
	err = certs.SaveCertificates(trustRootsPath, trustRoots)
	if err != nil {
		return err
	}
	err = certs.SavePkcs8PrivateKey(keyPath, key)
	if err != nil {
		return err
	}
	return nil
}

// GenElasticNodesAuth generates the credentials for an Elastic instance to authenticate itself to other cluster members.
func GenElasticNodesAuth(dir string, csrSigner certs.CSRSigner, trustRoots []*x509.Certificate) error {
	if csrSigner == nil {
		return errors.New("Cannot generate Elastic credentials without a CSR signer")
	}
	privateKey, err := ecdsa.GenerateKey(elliptic.P384(), rand.Reader)
	if err != nil {
		return errors.Wrapf(err, "error generating private key")
	}
	name := globals.ElasticSearch + "-node"
	hostname, ipaddrs := netutils.NameAndIPs()
	csr, err := certs.CreateCSR(privateKey, &pkix.Name{CommonName: name}, append(hostname, name), ipaddrs)
	if err != nil {
		return errors.Wrapf(err, "error generating csr")
	}
	cert, err := csrSigner(csr)
	if err != nil {
		return errors.Wrapf(err, "error generating certificate")
	}

	return storeElasticCredentials(dir, 0700, privateKey, cert, trustRoots)
}

// GenElasticHTTPSAuth generates the credentials for the Elastic server to authenticate itself to clients.
// Some Elastic clients (Kibana for example) does not support P384, so we create a P256 certificate
// specifically for HTTPs
func GenElasticHTTPSAuth(dir string, csrSigner certs.CSRSigner, trustRoots []*x509.Certificate) error {
	if csrSigner == nil {
		return errors.New("Cannot generate Elastic credentials without a CSR signer")
	}
	privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		return errors.Wrapf(err, "error generating private key")
	}
	name := globals.ElasticSearch + "-https"
	hostname, ipaddrs := netutils.NameAndIPs()
	csr, err := certs.CreateCSR(privateKey, &pkix.Name{CommonName: name}, append(hostname, name), ipaddrs)
	if err != nil {
		return errors.Wrapf(err, "error generating csr")
	}
	cert, err := csrSigner(csr)
	if err != nil {
		return errors.Wrapf(err, "error generating certificate")
	}

	return storeElasticCredentials(dir, 0700, privateKey, cert, trustRoots)
}

// GenElasticClientsAuth generates credentials for Venice Elastic Clients (EventMgr, Spyglass, etc.)
func GenElasticClientsAuth(dir string, csrSigner certs.CSRSigner, trustRoots []*x509.Certificate) error {
	if csrSigner == nil {
		return errors.New("Cannot generate Elastic credentials without a CSR signer")
	}
	privateKey, err := ecdsa.GenerateKey(elliptic.P384(), rand.Reader)
	if err != nil {
		return errors.Wrapf(err, "error generating private key")
	}
	csr, err := certs.CreateCSR(privateKey, &pkix.Name{CommonName: globals.ElasticSearch + "-client"}, []string{}, []net.IP{})
	if err != nil {
		return errors.Wrapf(err, "error generating csr")
	}
	cert, err := csrSigner(csr)
	if err != nil {
		return errors.Wrapf(err, "error generating certificate")
	}

	return storeElasticCredentials(dir, 0755, privateKey, cert, trustRoots)
}

// GenElasticAuth generate credentials for Elastic nodes to authenticate to each other and
// for clients to authenticate to Eleastic nodes
func GenElasticAuth(csrSigner certs.CSRSigner, trustRoots []*x509.Certificate) error {
	err := GenElasticNodesAuth(globals.ElasticNodeAuthDir, csrSigner, trustRoots)
	if err != nil {
		return errors.Wrapf(err, "Error generating Elastic node credentials")
	}
	err = GenElasticHTTPSAuth(globals.ElasticHTTPSAuthDir, csrSigner, trustRoots)
	if err != nil {
		return errors.Wrapf(err, "Error generating Elastic node credentials")
	}
	err = GenElasticClientsAuth(globals.ElasticClientAuthDir, csrSigner, trustRoots)
	if err != nil {
		return errors.Wrapf(err, "Error generating Elastic clients credentials")
	}
	return nil
}
