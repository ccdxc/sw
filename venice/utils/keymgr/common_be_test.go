// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"fmt"
	mathrand "math/rand"
	"reflect"
	"sync"
	"testing"

	"github.com/pensando/sw/venice/utils/certs"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var keyPairTypes = []KeyType{
	RSA1024,
	RSA2048,
	RSA4096,
	//ECDSA224, // not supported by openssl installed in dev VMs
	ECDSA256,
	ECDSA384,
	//ECDSA521, // not supported by SoftHSM installed in dev VMs
}

// ---------------------------------------------------------------------------------------//
// helper functions for multiple test-cases                                               //
// ---------------------------------------------------------------------------------------//
func testObjectStoreSerialAccess(t *testing.T, keyMgr *KeyMgr, prefix string) {
	keyID := prefix + "-key"
	certID := prefix + "-cert"
	bundleID := prefix + "-bundle"

	key, err := keyMgr.CreateKeyPair(keyID, ECDSA256)
	AssertOk(t, err, "Error generating private key")
	Assert(t, key.KeyType == ECDSA256, "Error generating private key")
	cert, err := certs.SelfSign(certID, key, certs.WithValidityDays(1))
	AssertOk(t, err, "Error generating self-signed certificate")

	// KeyPair object
	key2, err := keyMgr.GetObject(keyID, ObjectTypeKeyPair)
	AssertOk(t, err, "Error retrieving KeyPair object")
	Assert(t, reflect.DeepEqual(key, key2), fmt.Sprintf("Looked-up object does not match stored one. Have: %+v, Want: %+v", key2, key))
	err = keyMgr.StoreObject(key2)
	Assert(t, err != nil, "Adding existing object did not fail as expected")
	err = keyMgr.DestroyObject(keyID, ObjectTypeKeyPair)
	AssertOk(t, err, "Error destroying KeyPair object")
	key3, err := keyMgr.GetObject(keyID, ObjectTypeKeyPair)
	AssertOk(t, err, "Error retrieving KeyPair object")
	Assert(t, key3 == nil, "KeyPair object still available after destruction")
	err = keyMgr.DestroyObject(keyID, ObjectTypeKeyPair)
	Assert(t, err != nil, "Destroying non-existing keypair did not fail as expected")

	// Certificate object
	err = keyMgr.StoreObject(NewCertificateObject(certID, cert))
	AssertOk(t, err, "Error storing certificate object")
	cert2Obj, err := keyMgr.GetObject(certID, ObjectTypeCertificate)
	AssertOk(t, err, "Error retrieving certificate object")
	err = keyMgr.StoreObject(cert2Obj)
	Assert(t, err != nil, "Adding existing object did not fail as expected")
	cert2 := cert2Obj.(*Certificate)
	Assert(t, reflect.DeepEqual(cert, cert2.Certificate), fmt.Sprintf("Looked-up object does not match stored one. Have: %+v, Want: %+v", cert2.Certificate, cert))
	err = keyMgr.DestroyObject(certID, ObjectTypeCertificate)
	AssertOk(t, err, "Error destroying certificate object")
	cert3, err := keyMgr.GetObject(certID, ObjectTypeCertificate)
	AssertOk(t, err, "Error retrieving certificate object")
	Assert(t, cert3 == nil, "Certificate object still available after destruction")
	err = keyMgr.DestroyObject(certID, ObjectTypeCertificate)
	Assert(t, err != nil, "Destroying non-existing certificate did not fail as expected")

	// CertificateBundle object
	signKeyID := "sign-" + keyID
	signKey, err := keyMgr.CreateKeyPair(signKeyID, ECDSA384)
	AssertOk(t, err, "Error generating private key")
	defer keyMgr.DestroyObject(signKeyID, ObjectTypeKeyPair)
	bundle := make([]*x509.Certificate, 0)
	for i := 0; i < 20; i++ {
		cert, err := certs.SelfSign(getBundleCertificateID(bundleID, i), signKey, certs.WithValidityDays(1))
		bundle = append(bundle, cert)
		AssertOk(t, err, "Error generating self-signed certificate")
	}
	err = keyMgr.StoreObject(NewCertificateBundleObject(bundleID, bundle))
	AssertOk(t, err, "Error storing certificate bundle")
	bundle2Obj, err := keyMgr.GetObject(bundleID, ObjectTypeCertificateBundle)
	AssertOk(t, err, "Error retrieving certificate bundle")
	bundle2 := bundle2Obj.(*CertificateBundle).Certificates
	for i, c := range bundle2 {
		Assert(t, reflect.DeepEqual(c, bundle[i]), fmt.Sprintf("Looked-up object does not match stored one. Have: %+v, Want: %+v", c, bundle[i]))
	}
	err = keyMgr.DestroyObject(bundleID, ObjectTypeCertificateBundle)
	AssertOk(t, err, "Error destroying certificate bundle")
	bundle3Obj, err := keyMgr.GetObject(bundleID, ObjectTypeCertificateBundle)
	AssertOk(t, err, "Error retrieving certificate bundle")
	Assert(t, bundle3Obj == nil, "CertificateBundle object still available after destruction")
	err = keyMgr.DestroyObject(bundleID, ObjectTypeCertificate)
	Assert(t, err != nil, "Destroying non-existing bundle did not fail as expected")
}

func testSerialKeyOps(t *testing.T, keyMgr *KeyMgr, keyPrefix string) {
	msg := []byte("Hello, I'm test string. Sign me!")
	signerOpts := crypto.SHA256
	for i, keyPairType := range keyPairTypes {
		keyID := fmt.Sprintf("%s-%d", keyPrefix, i)
		keyPair, err := keyMgr.CreateKeyPair(keyID, keyPairType)
		AssertOk(t, err, fmt.Sprintf("Error creating key pair, type: %d", i))
		defer keyMgr.DestroyObject(keyID, ObjectTypeKeyPair)

		signature, err := keyPair.Sign(rand.Reader, msg, signerOpts)
		AssertOk(t, err, fmt.Sprintf("Error signing msg with key pair, type: %d", i))
		valid, err := VerifySignature(keyPair.Public(), signature, msg, signerOpts)
		AssertOk(t, err, fmt.Sprintf("Error verifying signature, key pair type: %d", i))
		Assert(t, valid, fmt.Sprintf("Signature not valid, key pair type: %d", i))
	}
}

// ---------------------------------------------------------------------------------------//
// Test functions. Should pass for all backends.                                          //
// ---------------------------------------------------------------------------------------//

// test creating, storing, retrieving and destroying objects
func testObjectStore(t *testing.T, be Backend) {
	keyMgr, err := NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")

	// NEGATIVE TEST-CASES
	// Empty ID
	_, err = keyMgr.CreateKeyPair("", ECDSA256)
	Assert(t, err != nil, "CreateKeyPair succeeded with empty ID")
	emptyIDObj := &Certificate{}
	err = keyMgr.StoreObject(emptyIDObj)
	Assert(t, err != nil, "StoreObject succeeded with empty ID")
	// Name starting with -
	_, err = keyMgr.CreateKeyPair("-ABCDE", ECDSA256)
	Assert(t, err != nil, "CreateKeyPair succeeded with invalid ID")
	err = keyMgr.StoreObject(NewCertificateBundleObject("-", nil))
	Assert(t, err != nil, "StoreObject succeeded with invalid ID")
	// Nil object
	err = keyMgr.StoreObject(nil)
	Assert(t, err != nil, "StoreObject succeeded with nil object")

	// POSITIVE TEST-CASES: store, retrieval, destruction
	// generate a private key and use it so self-sign a cert
	testObjectStoreSerialAccess(t, keyMgr, "testObjectStore")
}

// test signing and verifying operations with all supported key types
func testKeyOps(t *testing.T, be Backend) {
	keyMgr, err := NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")
	testSerialKeyOps(t, keyMgr, "testKeyOps")
}

// test the case in which the backend contains objects that are not in the KeyMgr cache
// because they were added or imported beforehand or because KeyMgr has restarted
func testWarmStart(t *testing.T, be Backend) {
	keyID := "HelloKey"
	certID := "HelloCert"

	// instantiate a KeyMgr instance and populate it with a KeyPair and a cert
	keyMgr, err := NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")

	// generate and store a key-pair and a cert
	key, err := keyMgr.CreateKeyPair(keyID, RSA2048)
	AssertOk(t, err, "Error generating private key")
	defer keyMgr.DestroyObject(keyID, ObjectTypeKeyPair)
	cert, err := certs.SelfSign(certID, key, certs.WithValidityDays(1))
	AssertOk(t, err, "Error generating self-signed certificate")
	err = keyMgr.StoreObject(NewCertificateObject(certID, cert))
	AssertOk(t, err, "Error storing certificate")
	defer keyMgr.DestroyObject(certID, ObjectTypeCertificate)

	// Now instantiate an independent instance of KeyMgr with the same backend
	keyMgr2, err := NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")
	// Make sure that the objects are still available and match what was stored.
	key2, err := keyMgr2.GetObject(keyID, ObjectTypeKeyPair)
	AssertOk(t, err, "Error retrieving key")
	Assert(t, key2 != nil, "Key not found")
	Assert(t, reflect.DeepEqual(key, key2), fmt.Sprintf("Looked-up object does not match stored one. Have: %+v, Want: %+v", key2, key))
	certObj := NewCertificateObject(certID, cert)
	cert2, err := keyMgr2.GetObject(certID, ObjectTypeCertificate)
	AssertOk(t, err, "Error retrieving certificate")
	Assert(t, cert2 != nil, "Certificate not found")
	Assert(t, reflect.DeepEqual(certObj, cert2), fmt.Sprintf("Looked-up object does not match stored one. Have: %+v, Want: %+v", cert2, certObj))
}

// For the moment key import functions are used for testing only.
func testKeyImport(t *testing.T, be Backend) {
	// instantiate a KeyMgr instance and populate it with a KeyPair and a cert
	keyMgr, err := NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")

	for i, keyType := range keyPairTypes {
		keyID := fmt.Sprintf("importedKey-%d", i)
		var keyPair *KeyPair
		switch keyType {
		case RSA1024, RSA2048, RSA4096:
			key, err := rsa.GenerateKey(rand.Reader, rsaKeyTypeToBitSize[keyType])
			AssertOk(t, err, fmt.Sprintf("Error creating key, type : %v", keyType))
			keyPair = NewKeyPairObject(keyID, key)
		case ECDSA224, ECDSA256, ECDSA384, ECDSA521:
			key, err := ecdsa.GenerateKey(ecdsaKeyTypeToCurve[keyType], rand.Reader)
			AssertOk(t, err, fmt.Sprintf("Error creating key, type : %v", keyType))
			keyPair = NewKeyPairObject(keyID, key)
		}
		defer keyMgr.DestroyObject(keyID, ObjectTypeKeyPair)
		err = keyMgr.StoreObject(keyPair)
		AssertOk(t, err, fmt.Sprintf("Error storing key pair %+v", keyPair))
		keyPair2, err := keyMgr.GetObject(keyID, ObjectTypeKeyPair)
		AssertOk(t, err, fmt.Sprintf("Error retrieving key pair %+v", keyPair))
		Assert(t, reflect.DeepEqual(keyPair, keyPair2), fmt.Sprintf("Looked-up object does not match stored one. Have: %+v, Want: %+v", keyPair2, keyPair))
	}
}

func testParallelAccess(t *testing.T, be Backend) {
	// Instantiate two keyMgr on the same backend, so that we exercise the
	// synchronization mechanisms of KeyMgr as well as of the backend
	km1, err := NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")
	km2, err := NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")

	kms := []*KeyMgr{km1, km2}
	var wg sync.WaitGroup
	for i := 0; i < 10; i++ {
		prefix := fmt.Sprintf("test-%d", i)
		km := kms[mathrand.Intn(2)]
		wg.Add(1)
		go func() {
			defer wg.Done()
			testObjectStoreSerialAccess(t, km, prefix)
		}()
		km = kms[mathrand.Intn(2)]
		wg.Add(1)
		go func() {
			defer wg.Done()
			testSerialKeyOps(t, km, prefix)
		}()
	}
	wg.Wait()
}

func testDeriveKeyNegativeCases(t *testing.T, be Backend) {
	km, err := NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating backend")
	key, err := km.CreateKeyPair("baseKey", ECDSA384)
	defer km.DestroyObject("baseKey", ObjectTypeKeyPair)
	AssertOk(t, err, "Error generating key")
	// empty base key id
	_, err = km.DeriveKey("derivedKey", AES256, "", key.Signer.Public())
	Assert(t, err != nil, "Key derivation succeeded without an ID for the base key")
	// empty derived key id
	_, err = km.DeriveKey("", AES256, "baseKey", key.Signer.Public())
	Assert(t, err != nil, "Key derivation succeeded without an ID for the derived key")
	// empty public key
	_, err = km.DeriveKey("derivedKey", AES256, "baseKey", nil)
	Assert(t, err != nil, "Key derivation succeeded without the public part of the base key")
	// unsupported base key type -- it must be ECDSA
	unsupportedKey, err := km.CreateKeyPair("unsupportedKey", RSA1024)
	defer km.DestroyObject("baseKey", ObjectTypeKeyPair)
	AssertOk(t, err, "Error generating key")
	_, err = km.DeriveKey("derivedKey", AES256, "baseKey", unsupportedKey.Signer.Public())
	Assert(t, err != nil, "Key derivation succeeded with unsupported base key")
	// unsupported derived key type -- it must be AES
	_, err = km.DeriveKey("derivedKey", ECDSA256, "baseKey", key.Signer.Public())
	Assert(t, err != nil, "Key derivation succeeded without the public part of the base key")
}

func testWrapKeyPairNegativeCases(t *testing.T, be Backend) {
	km, err := NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating backend")
	baseKey, err := km.CreateKeyPair("baseKey", ECDSA384)
	defer km.DestroyObject("baseKey", ObjectTypeKeyPair)
	AssertOk(t, err, "Error generating key")
	_, err = km.DeriveKey("kek", AES256, "baseKey", baseKey.Signer.Public())
	AssertOk(t, err, "Error deriving key")
	defer km.DestroyObject("kek", ObjectTypeSymmetricKey)
	// Empty key pair ID
	_, err = km.WrapKeyPair("", "kek")
	Assert(t, err != nil, "WrapKeyPair succeeded with empty key pair ID")
	// Empty wrapping key ID
	_, err = km.WrapKeyPair("baseKey", "")
	Assert(t, err != nil, "WrapKeyPair succeeded with empty wrapping key ID")
	// Non-existent key pair ID
	_, err = km.WrapKeyPair("nonexistent", "kek")
	Assert(t, err != nil, "WrapKeyPair succeeded with nonexistent key pair ID")
	// Non-existent wrapping key ID
	_, err = km.WrapKeyPair("baseKey", "nonexistent")
	Assert(t, err != nil, "WrapKeyPair succeeded with nonexistent wrapping key ID")
}

func testUnwrapKeyPairNegativeCases(t *testing.T, be Backend) {
	km, err := NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating backend")
	baseKey, err := km.CreateKeyPair("baseKey", ECDSA384)
	defer km.DestroyObject("baseKey", ObjectTypeKeyPair)
	AssertOk(t, err, "Error generating key")
	_, err = km.DeriveKey("kek", AES256, "baseKey", baseKey.Signer.Public())
	AssertOk(t, err, "Error deriving key")
	defer km.DestroyObject("kek", ObjectTypeSymmetricKey)
	wrappedKey, err := km.WrapKeyPair("baseKey", "kek")
	AssertOk(t, err, "Error wrapping key")
	// Empty target key ID
	_, err = km.UnwrapKeyPair("", wrappedKey, baseKey.Public(), "kek")
	Assert(t, err != nil, "UnwrapKeyPair succeeded with empty target key ID")
	// Empty unwrapping key ID
	_, err = km.UnwrapKeyPair("unwrappedKey", wrappedKey, baseKey.Public(), "")
	Assert(t, err != nil, "UnwrapKeyPair succeeded with empty unwrapping key ID")
	// Non-existent unwrapping key ID
	_, err = km.UnwrapKeyPair("unwrappedKey", wrappedKey, baseKey.Public(), "nonexistent")
	Assert(t, err != nil, "UnwrapKeyPair succeeded with nonexistent wrapping key ID")
	// Empty wrapped key
	_, err = km.UnwrapKeyPair("unwrappedKey", nil, baseKey.Public(), "kek")
	Assert(t, err != nil, "UnwrapKeyPair succeeded with nil public key")
	// Empty public key
	_, err = km.UnwrapKeyPair("unwrappedKey", wrappedKey, nil, "kek")
	Assert(t, err != nil, "UnwrapKeyPair succeeded with nil public key")
}

// Test end-to-end ECDH key transport (derivation + AES wrapping) between with two backends
func testECDHKeyTransport(t *testing.T, be1, be2 Backend) {
	// Create independent KeyMgr instances for the two backends
	km1, err := NewKeyMgr(be1)
	AssertOk(t, err, "Error instantiating backend 1")
	km2, err := NewKeyMgr(be2)
	AssertOk(t, err, "Error instantiating backend 2")
	// Create EC keys on the two backends. The keys are completely independent
	// but they have to be the same size (and use the same curve)
	key1, err := km1.CreateKeyPair("ECKey1", ECDSA384)
	AssertOk(t, err, "Error creating key pair in backend 1")
	defer km1.DestroyObject("ECKey1", ObjectTypeKeyPair)
	key2, err := km2.CreateKeyPair("ECKey2", ECDSA384)
	AssertOk(t, err, "Error creating key pair in backend 2")
	defer km2.DestroyObject("ECKey2", ObjectTypeKeyPair)
	// Perform ECDH key derivation on the two backends.
	// Backend1 uses private key key1 + public key key2, Backend2 uses private key key2 + public key key1
	// Both backends end up with identical AES key-encryption-key ("kek1" on Backend1, "kek2" on Backend2)
	_, err = km1.DeriveKey("kek1", AES256, "ECKey1", key2.Signer.Public())
	AssertOk(t, err, "Error deriving key on backend 1")
	defer km1.DestroyObject("kek1", ObjectTypeSymmetricKey)
	_, err = km2.DeriveKey("kek2", AES256, "ECKey2", key1.Signer.Public())
	AssertOk(t, err, "Error deriving key on backend 1")
	defer km2.DestroyObject("kek2", ObjectTypeSymmetricKey)
	// Generate an RSA key in Backend1 and wrap it with the AES key
	transpKeyRSA, err := km1.CreateKeyPair("TranspKeyRSA", RSA4096)
	defer km1.DestroyObject("TranspKeyRSA", ObjectTypeKeyPair)
	AssertOk(t, err, "Error generating key to transport")
	wrappedKeyRSA, err := km1.WrapKeyPair("TranspKeyRSA", "kek1")
	AssertOk(t, err, "Error wrapping key")
	// Unwrap the RSA key on backend 2. The public part needs to be supplied separately.
	unwrappedKeyPairRSA, err := km2.UnwrapKeyPair("UnwrappedKeyRSA", wrappedKeyRSA, transpKeyRSA.Public(), "kek2")
	AssertOk(t, err, "Error unwrapping key")
	defer km2.DestroyObject("UnwrappedKeyRSA", ObjectTypeKeyPair)
	// Now, to verify that what was unwrapped on be2 matches what what was wrapped in be1
	// We sign something in be2 and verify the signature in be1
	msg := []byte("0123456789ABCDEF0123456789ABCDEF")
	signerOpts := crypto.SHA256
	signatureRSA, err := unwrappedKeyPairRSA.Signer.Sign(rand.Reader, msg, signerOpts)
	AssertOk(t, err, "Error signing msg")
	valid, err := VerifySignature(transpKeyRSA.Public(), signatureRSA, msg, signerOpts)
	AssertOk(t, err, "Error verifying signature")
	Assert(t, valid, "Signature not valid")
	// Now transport an ECDSA key from backend2 to backend1
	transpKeyECDSA, err := km2.CreateKeyPair("TranspKeyECDSA", ECDSA256)
	AssertOk(t, err, "Error generating key to transport")
	defer km2.DestroyObject("TranspKeyECDSA", ObjectTypeKeyPair)
	wrappedKeyECDSA, err := km2.WrapKeyPair("TranspKeyECDSA", "kek2")
	AssertOk(t, err, "Error wrapping key")
	// Unwrap the ECDSA key on backend 1. The public part needs to be supplied separately.
	unwrappedKeyPairECDSA, err := km1.UnwrapKeyPair("UnwrappedKeyECDSA", wrappedKeyECDSA, transpKeyECDSA.Public(), "kek1")
	defer km1.DestroyObject("UnwrappedKeyECDSA", ObjectTypeKeyPair)
	AssertOk(t, err, "Error unwrapping key")
	// Now, to verify that what was unwrapped on be1 matches what what was wrapped in be2
	// We sign msg in be1 and verify the signature in be2
	signatureECDSA, err := unwrappedKeyPairECDSA.Signer.Sign(rand.Reader, msg, signerOpts)
	AssertOk(t, err, "Error signing msg")
	valid, err = VerifySignature(transpKeyECDSA.Public(), signatureECDSA, msg, signerOpts)
	AssertOk(t, err, "Error verifying signature")
	Assert(t, valid, "Signature not valid")
}

// ---------------------------------------------------------------------------------------//
// "main" function. Execute all tests for all backends.                                   //
// ---------------------------------------------------------------------------------------//

func testAll(t *testing.T, backends []Backend) {
	// Tests for individual backends
	for _, be := range backends {
		t.Logf("Running tests for backend type: %T", be)
		testObjectStore(t, be)
		testKeyOps(t, be)
		testKeyImport(t, be)
		testWarmStart(t, be)
		testParallelAccess(t, be)
		testDeriveKeyNegativeCases(t, be)
		testWrapKeyPairNegativeCases(t, be)
		testECDHKeyTransport(t, be, be)
	}

	numBackends := len(backends)
	if numBackends >= 2 {
		for i := 0; i < numBackends; i++ {
			// Cross-backend tests
			j := (i + 1) % numBackends
			testECDHKeyTransport(t, backends[i], backends[j])
		}
	}
}
