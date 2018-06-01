// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package credentials

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/quorum"
)

// SetQuorumPeerAuth creates credentials for authentication of a quorum node with its peers and
// adds them to the supplied quorum config
func SetQuorumPeerAuth(c *quorum.Config) error {
	kvsPrivKey, err := ecdsa.GenerateKey(elliptic.P384(), rand.Reader)
	if err != nil {
		return errors.Wrapf(err, "error generating private key")
	}
	csr, err := certs.CreateCSR(kvsPrivKey, nil, nil)
	if err != nil {
		return errors.Wrapf(err, "error generating csr")
	}
	cert, err := env.CertMgr.Ca().Sign(csr)
	if err != nil {
		return errors.Wrapf(err, "error generating certificate")
	}

	c.PeerAuthEnabled = true
	c.PeerCert = cert
	c.PeerPrivateKey = kvsPrivKey
	c.PeerCATrustBundle = env.CertMgr.Ca().TrustRoots()

	return nil
}
