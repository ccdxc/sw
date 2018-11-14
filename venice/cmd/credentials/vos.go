// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package credentials

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"crypto/x509/pkix"
	"net"
	"os"
	"path"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
)

const (
	// These file names need to match those in github.com/minio/minio/cmd/config-dir.go
	httpsCertFileName = "public.crt"
	httpsKeyFileName  = "private.key"
)

// GenVosHTTPSAuth generates the credentials for Minio backend to authenticate itself to clients.
func GenVosHTTPSAuth(dir string, csrSigner certs.CSRSigner, caTrustChain []*x509.Certificate) error {
	if csrSigner == nil {
		return errors.New("Cannot generate VOS credentials without a CSR signer")
	}
	// As of 10/2018 Minio only supports P-256
	privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		return errors.Wrapf(err, "error generating private key")
	}
	name := globals.Vos
	csr, err := certs.CreateCSR(privateKey, &pkix.Name{CommonName: name}, []string{}, []net.IP{})
	if err != nil {
		return errors.Wrapf(err, "error generating csr")
	}
	cert, err := csrSigner(csr)
	if err != nil {
		return errors.Wrapf(err, "error generating certificate")
	}

	perm := os.FileMode(0700)
	err = os.MkdirAll(dir, perm)
	if err != nil {
		return errors.Wrapf(err, "Error creating directory: %v with permissions: %v", dir, perm)
	}

	certsPath := path.Join(dir, httpsCertFileName)
	err = certs.SaveCertificates(certsPath, append([]*x509.Certificate{cert}, caTrustChain...))
	if err != nil {
		return err
	}

	keyPath := path.Join(dir, httpsKeyFileName)
	err = certs.SavePrivateKey(keyPath, privateKey) // Minio only supports PKCS-1
	if err != nil {
		return err
	}

	return nil
}

// GenVosAuth generate credentials for Vos instances backed by Minio
func GenVosAuth(csrSigner certs.CSRSigner, caTrustChain []*x509.Certificate) error {
	err := GenVosHTTPSAuth(globals.VosHTTPSAuthDir, csrSigner, caTrustChain)
	if err != nil {
		return errors.Wrapf(err, "Error generating VOS node credentials")
	}
	return nil
}
