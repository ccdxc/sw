// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package credentials

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"crypto/x509/pkix"
	"net"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/cmd/validation"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
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

	dnsNames := []string{globals.Etcd}
	ipAddrs := []net.IP{}
	// Etcd only binds to IP addresses, so to keep it simple clients also use IP addresses to connect
	// If user gave a hostname, we need to put corresponding address in the certificate
	addrs, err := net.LookupIP(c.MemberName)
	if err == nil {
		ipAddrs = append(ipAddrs, addrs[0])
	}
	dnsNames, ipAddrs = certs.AddNodeSANIDs(c.MemberName, dnsNames, ipAddrs)

	// When an Etcd instance tries to connect to another, the server checks that either:
	// - the IP address is present in the IP SANs of the certificates provided by the client
	// - the IP address reverse-resolves to a name that is present in the DNS SANs
	//
	// If there are multiple addresses configured on the same interface, we may receive a packet
	// with a SRC IP that is different from what user provided. To work around this issue, we
	// find the network interface with the IP we know about and put in the certificate all
	// the other IPs configured on that interface.
	if len(ipAddrs) > 0 {
		intf, _, found, err := netutils.FindInterfaceForIP(ipAddrs[0].String())
		if err == nil && found {
			otherAddrs, err := intf.Addrs()
			if err == nil {
				for _, otherAddr := range otherAddrs {
					if ip, _, err := net.ParseCIDR(otherAddr.String()); err == nil {
						if validation.IsValidNodeIP(&ip) {
							ipAddrs = netutils.AppendIPIfNotPresent(ip, ipAddrs)
						}
					}
				}
			} else {
				log.Errorf("Error getting addresses for interface %v: %v", intf.Name, err)
			}
		} else {
			log.Errorf("Error getting same-interface addresses for IP address %v: %v", ipAddrs[0], err)
		}
	}

	csr, err := certs.CreateCSR(kvsPrivKey, &pkix.Name{CommonName: globals.Etcd}, dnsNames, ipAddrs)
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
