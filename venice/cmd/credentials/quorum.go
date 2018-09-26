// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package credentials

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/quorum"
)

// SetQuorumInstanceAuth creates credentials for authentication of a quorum instance with its peers and
// with clients and adds them to the supplied quorum config.
// The csrSigner parameter is a function that takes a CSR and returns a signed certificate.
// trustRoots is the certificate bundle used by server to verify clients certificate
func SetQuorumInstanceAuth(c *quorum.Config, csrSigner certs.CSRSigner, trustRoots []*x509.Certificate) error {
	if csrSigner == nil {
		return errors.New("Cannot set quorum server auth without a CSR signer")
	}
	kvsPrivKey, err := ecdsa.GenerateKey(elliptic.P384(), rand.Reader)
	if err != nil {
		return errors.Wrapf(err, "error generating private key")
	}
	_, ipaddrs := netutils.NameAndIPs()
	csr, err := certs.CreateCSR(kvsPrivKey, nil, []string{globals.Etcd, c.MemberName}, ipaddrs)
	if err != nil {
		return errors.Wrapf(err, "error generating csr")
	}
	cert, err := csrSigner(csr)
	if err != nil {
		return errors.Wrapf(err, "error generating certificate")
	}

	c.PeerAuthEnabled = true
	c.PeerCert = cert
	c.PeerPrivateKey = kvsPrivKey
	c.PeerCATrustBundle = trustRoots

	c.ClientAuthEnabled = true
	c.ClientCert = cert
	c.ClientPrivateKey = kvsPrivKey
	c.ClientCATrustBundle = trustRoots

	return nil
}

// GenQuorumClientAuth creates credentials for clients to authenticate with a
// quorum member and stores them in a well-known location.
// The csrSigner parameter is a function that takes a CSR and returns a signed certificate.
// trustRoots is the certificate bundle used by client to verify server certificate
func GenQuorumClientAuth(csrSigner certs.CSRSigner, trustRoots []*x509.Certificate) error {
	return certs.CreateTLSCredentials(globals.EtcdClientAuthDir, nil, []string{globals.Etcd + "-client"}, nil, trustRoots, csrSigner)
}
