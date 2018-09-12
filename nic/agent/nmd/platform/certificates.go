// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package platform

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/sha1"
	"crypto/x509"
	"crypto/x509/pkix"
	"math/big"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/cluster"
)

// Code in this file is meant to go away once we have APIs to read
// the platform certificate and sign data using the platform key

func (np *NaplesPlatformAgent) initPlatformCredentials(nic *cluster.SmartNIC) error {
	// For the moment, create a private key and a self-signed certificate
	key, err := ecdsa.GenerateKey(elliptic.P384(), rand.Reader)
	if err != nil {
		return errors.Wrapf(err, "Error generating platform key")
	}

	sn := big.NewInt(0)
	sn.SetBytes([]byte(nic.Status.SerialNum)) // the result of SetBytes() is always positive

	pubKeyBytes, err := x509.MarshalPKIXPublicKey(key.Public())
	if err != nil {
		return errors.Wrap(err, "Failed to Marshall public key")
	}
	skid := sha1.Sum(pubKeyBytes)

	template := &x509.Certificate{ // per IEEE802.1ar
		BasicConstraintsValid: true,
		KeyUsage:              x509.KeyUsageDigitalSignature | x509.KeyUsageKeyEncipherment,
		IsCA:                  false,
		NotBefore:             time.Date(2016, time.January, 1, 0, 0, 0, 0, time.UTC),
		NotAfter:              time.Date(2046, time.January, 1, 0, 0, 0, 0, time.UTC),
		Subject: pkix.Name{
			Organization:       []string{"Pensando Systems"},
			OrganizationalUnit: []string{"Pensando Manufacturing CA"},
			SerialNumber:       "PID=NAPLESv1 SN=" + nic.Status.SerialNum,
			CommonName:         nic.Name,
		},
		SerialNumber: sn,
		SubjectKeyId: skid[:],
	}

	cert, err := x509.CreateCertificate(rand.Reader, template, template, key.Public(), key)
	if err != nil {
		return errors.Wrapf(err, "Error generating platform certificate")
	}
	np.key = key
	np.certificate = cert
	return nil
}
