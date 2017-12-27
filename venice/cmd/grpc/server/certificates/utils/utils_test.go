// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package utils

import (
	"bytes"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"fmt"
	"testing"

	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/keymgr"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func verifyBundle(t *testing.T, bundle *grpc.CertMgrBundle, trustChain, trustRoots []*x509.Certificate) {
	Assert(t, len(trustChain) == len(bundle.GetCaTrustChain()),
		fmt.Sprintf("Error verifying trust chain. Have: %d certificates, want: %d",
			len(bundle.GetCaTrustChain()), len(trustChain)))
	Assert(t, len(trustRoots) == len(bundle.GetTrustRoots()),
		fmt.Sprintf("Error verifying trust roots. Have: %d certificates, want: %d",
			len(bundle.GetTrustRoots()), len(trustRoots)))
	for i, c := range bundle.GetCaTrustChain() {
		Assert(t, bytes.Equal(c.Certificate, trustChain[i].Raw), "Error verifying trust chain, index: %d, Have %+v, Want: %+v", c, trustChain[i])
	}
	for i, c := range bundle.GetTrustRoots() {
		Assert(t, bytes.Equal(c.Certificate, trustRoots[i].Raw), "Error verifying trust roots, index: %d, Have %+v, Want: %+v", c, trustRoots[i])
	}
}

func TestBundlePackUnpack(t *testing.T) {
	// Use 2 different CertMgr with 2 different backends
	cm1, err := certmgr.NewDefaultCertificateMgr()
	AssertOk(t, err, "Failed to instantiate CertificateMgr")
	defer cm1.Close()
	// SoftHSM installed on dev vms does not support multiple tokens,
	// so always use GoCrypto for second backend
	be2, err := keymgr.NewGoCryptoBackend("certmgr")
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	km2, err := keymgr.NewKeyMgr(be2)
	AssertOk(t, err, "Error instantiating KeyMgr")
	cm2, err := certmgr.NewCertificateMgr(km2)
	AssertOk(t, err, "Error instantiating CertMgr")
	defer cm2.Close()
	// Start CA on CM1
	err = cm1.StartCa(true)
	AssertOk(t, err, "Error starting CA")
	Assert(t, cm1.IsReady(), "CertificateMgr not ready")
	// generate key-agreement-keys on CM1 and CM2
	kak1, err := cm1.GetKeyAgreementKey("cm2")
	AssertOk(t, err, "Error generating key-agreement-keyr")
	kak2, err := cm2.GetKeyAgreementKey("self")
	AssertOk(t, err, "Error generating key-agreement-keyr")
	// prepare the bundle on CM1
	bundle, err := MakeCertMgrBundle(cm1, "cm2", cm1.MarshalKeyAgreementKey(kak2))
	AssertOk(t, err, "Error preparing CertMgr bundle")
	verifyBundle(t, bundle, cm1.Ca().TrustChain(), cm1.Ca().TrustRoots())
	// unpack the bundle on CM2 and start CA
	err = UnpackCertMgrBundle(cm2, bundle, kak1)
	AssertOk(t, err, "Error unpacking CertMgr bundle")
	err = cm2.StartCa(false)
	AssertOk(t, err, "Error starting CA")
	Assert(t, cm1.IsReady(), "CertificateMgr not ready")
	// calling again after CA is supposed to fail
	err = UnpackCertMgrBundle(cm2, bundle, kak1)
	Assert(t, err != nil, "Unpacking CertMgr bundle succeeded when expected to fail")
	// verify the bundle
	verifyBundle(t, bundle, cm2.Ca().TrustChain(), cm2.Ca().TrustRoots())
	// sign a certificate on CM2 and check it on CM1 to verify the private key
	privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating private key")
	csr, err := certs.CreateCSR(privateKey, []string{"Hello World"}, nil)
	AssertOk(t, err, "Error creating CSR")
	cert, err := cm2.Ca().Sign(csr)
	AssertOk(t, err, "Error getting CSR signed")
	ok, err := cm1.Ca().Verify(cert)
	AssertOk(t, err, "Certificate verification failed")
	Assert(t, ok, "Certificate verification failed")
}
