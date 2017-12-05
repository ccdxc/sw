// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certmgr

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"reflect"
	"testing"

	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/keymgr"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestCertMgrInit(t *testing.T) {
	// NEGATIVE TEST-CASES

	// empty dir
	_, err := NewCertificateMgr(nil)
	Assert(t, err != nil, "NewCertificateAuthority succeeded with nil KeyMgr instance")
}

func TestRestart(t *testing.T) {
	// start
	cm, err := NewDefaultCertificateMgr()
	AssertOk(t, err, "Failed to instantiate CertificateMgr")
	Assert(t, !cm.IsReady(), "CertificateMgr ready before CA start")
	cm.StartCa(true)
	Assert(t, cm.IsReady(), "CertificateMgr not ready")
	Assert(t, nil != cm.Ca(), "CertificateMgr returned a nil CA")
	Assert(t, cm.Ca().IsReady(), "CertificateMgr CA not ready")
	// create test object
	key, err := cm.GetKeyAgreementKey("test")
	AssertOk(t, err, "Failed to create test key")
	// close
	cm.Close()
	Assert(t, !cm.IsReady(), "CertificateMgr ready after close")
	Assert(t, cm.Ca() == nil, "CertificateMgr CA still available after close")
	// check that we can start a new instance cleanly
	cm, err = NewDefaultCertificateMgr()
	AssertOk(t, err, "Failed to instantiate CertificateMgr")
	Assert(t, !cm.IsReady(), "CertificateMgr ready before CA start")
	key2, err := cm.GetKeyAgreementKey("test")
	Assert(t, !reflect.DeepEqual(key, key2), "Leftover key found")
	cm.StartCa(true)
	Assert(t, cm.IsReady(), "CertificateMgr not ready")
	Assert(t, nil != cm.Ca(), "CertificateMgr returned a nil CA")
	Assert(t, cm.Ca().IsReady(), "CertificateMgr CA not ready")
	key3, err := cm.GetKeyAgreementKey("test")
	Assert(t, reflect.DeepEqual(key2, key3), "Unable to find test object after starting CA")
	cm.Close()
	// try to close before CA is started
	cm, err = NewDefaultCertificateMgr()
	AssertOk(t, err, "Failed to instantiate CertificateMgr")
	Assert(t, !cm.IsReady(), "CertificateMgr ready before CA start")
	cm.Close()
	Assert(t, !cm.IsReady(), "CertificateMgr ready after close")
	Assert(t, cm.Ca() == nil, "CertificateMgr CA still available after close")
}

func TestKeyAgreementFunctions(t *testing.T) {
	// Create and start CertMgr instance
	cm, err := NewDefaultCertificateMgr()
	AssertOk(t, err, "Failed to instantiate CertificateMgr")
	defer cm.Close()
	cm.StartCa(true)

	// NEGATIVE TESTCASES
	f1 := cm.MarshalKeyAgreementKey(nil)
	Assert(t, f1 == nil, "MarshalKeyAgreementKey succeeded with invalid key")
	f2 := cm.UnmarshalKeyAgreementKey(nil)
	Assert(t, f2 == nil, "UnmarshalKeyAgreementKey succeeded with invalid key")
	f2 = cm.UnmarshalKeyAgreementKey([]byte{1, 2, 3})
	Assert(t, f2 == nil, "UnmarshalKeyAgreementKey succeeded with invalid key")

	// POSITIVE TESTCASES
	// create key-agreement-key
	keyAgreementKey, err := cm.GetKeyAgreementKey("test")
	AssertOk(t, err, "Error creating key-agreement-key")
	// check that calling GetKeyAgreementKey with same ID returns the same key
	sameKeyAgreementKey, err := cm.GetKeyAgreementKey("test")
	AssertOk(t, err, "Error creating key-agreement-key")
	Assert(t, reflect.DeepEqual(keyAgreementKey, sameKeyAgreementKey), "Second call to GetKeyAgreementKey with same ID returned different key")
	// check that calling GetKeyAgreementKey with different ID produces a new one
	diffKeyAgreementKey, err := cm.GetKeyAgreementKey("test2")
	AssertOk(t, err, "Error creating key-agreement-key")
	Assert(t, !reflect.DeepEqual(keyAgreementKey, diffKeyAgreementKey), "Second call to GetKeyAgreementKey with different ID returned same key")
	// test marshaling/unmarshaling
	mKeyAgreementKey := cm.MarshalKeyAgreementKey(keyAgreementKey)
	Assert(t, mKeyAgreementKey != nil, "Error marshaling key-agreement-key")
	uKeyAgreementKey := cm.UnmarshalKeyAgreementKey(mKeyAgreementKey)
	Assert(t, reflect.DeepEqual(keyAgreementKey, uKeyAgreementKey), "Error unmarshaling key-agreement-key")
	// destroy the key agreement keys
	err = cm.DestroyKeyAgreementKey("test")
	AssertOk(t, err, "Error destroying key-agreement-key")
	err = cm.DestroyKeyAgreementKey("test2")
	AssertOk(t, err, "Error destroying key-agreement-key")
	// second call should produce an error
	err = cm.DestroyKeyAgreementKey("test")
	Assert(t, err != nil, "DestroyKeyAgreementKey call succeeded when expected to fail")
	// GetWrappedCAKey should fail if we haven't generated a key-agreement-key beforehand
	_, err = cm.GetWrappedCaKey("test", keyAgreementKey)
	Assert(t, err != nil, "GetWrappedCaKey call succeeded when expected to fail")
}

func TestKeyTransport(t *testing.T) {
	// Use 2 different CertMgr with 2 different backends
	cm1, err := NewDefaultCertificateMgr()
	AssertOk(t, err, "Failed to instantiate CertificateMgr")
	defer cm1.Close()
	// SoftHSM installed on dev vms does not support multiple tokens,
	// so always use GoCrypto for second backend
	be2, err := keymgr.NewGoCryptoBackend("certmgr")
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	km2, err := keymgr.NewKeyMgr(be2)
	AssertOk(t, err, "Error instantiating KeyMgr")
	cm2, err := NewCertificateMgr(km2)
	AssertOk(t, err, "Error instantiating CertMgr")
	defer cm2.Close()

	// start cm1 and extract the CA key in wrapped form and the ca cert
	cm1.StartCa(true)
	Assert(t, cm1.IsReady(), "CertificateMgr not ready")
	kak1, err := cm1.GetKeyAgreementKey("kak2")
	AssertOk(t, err, "Error generating key-agreement-key")
	defer cm1.DestroyKeyAgreementKey("kak1")
	kak2, err := cm2.GetKeyAgreementKey("kak1")
	AssertOk(t, err, "Error generating key-agreement-key")
	defer cm2.DestroyKeyAgreementKey("kak2")
	wrappedCaKey, err := cm1.GetWrappedCaKey("kak2", kak2)
	AssertOk(t, err, "Error getting wrapped key")
	caCert := cm1.ca.TrustChain()[0]

	// unwrap CA key and load trust roots
	err = cm2.UnwrapCaKey(wrappedCaKey, caCert.PublicKey, "kak1", kak1)
	AssertOk(t, err, "Error unwrapping key")
	err = cm2.LoadCaTrustChain(cm1.ca.TrustChain())
	AssertOk(t, err, "Error loading trust chain")
	err = cm2.LoadTrustRoots(cm1.ca.TrustRoots())
	AssertOk(t, err, "Error loading trust roots")
	cm2.StartCa(true)
	Assert(t, cm2.IsReady(), "Error starting cm2")

	// generate a certificate on CM2 and verify it on CM1
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
