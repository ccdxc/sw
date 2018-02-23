// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"testing"

	"crypto"
	"crypto/rand"
	"crypto/rsa"

	. "github.com/pensando/sw/venice/utils/testutils"
)

// Test initialization
func TestKeyMgrInit(t *testing.T) {
	// Negative test-case: nil backend
	_, err := NewKeyMgr(nil)
	Assert(t, err != nil, "NewKeyMgr succeeded with nil backend")
}

// Test the checks performed on API parameters
func TestParameterValidation(t *testing.T) {
	be, err := NewDefaultBackend("keymgr")
	AssertOk(t, err, "Error instantiating default backend")
	keyMgr, err := NewKeyMgr(be)
	defer keyMgr.Close()
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
}

// Tests invalid key param IDs
func TestInvalidKeyParamID(t *testing.T) {
	be, err := NewDefaultBackend("keymgr")
	AssertOk(t, err, "Error instantiating default backend")
	keyMgr, err := NewKeyMgr(be)
	defer keyMgr.Close()
	AssertOk(t, err, "Error instantiating KeyMgr")

	privKey, err := rsa.GenerateKey(rand.Reader, 2048)
	AssertOk(t, err, "Failed to generate RSA Key")

	var invalidWrappedKey []byte
	var invalidPubKey crypto.PublicKey
	invalidTgtKeyPairID := ""
	invalidUnwrappingID := ""

	// InvalidTargetKeyPairID validation
	_, err = keyMgr.UnwrapKeyPair(invalidTgtKeyPairID, []byte("key"), privKey.PublicKey, "unwrapping-id")
	Assert(t, err != nil, "Unwrapping key pairs with invalid targetKeyPairID should fail.")

	// InvalidTargetKeyPairID validation
	_, err = keyMgr.UnwrapKeyPair("target-pair-id", invalidWrappedKey, privKey.PublicKey, "unwrapping-id")
	Assert(t, err != nil, "Unwrapping key pairs with invalid wrappedKey should fail.")

	// InvalidTargetKeyPairID validation
	_, err = keyMgr.UnwrapKeyPair("target-pair-id", []byte("key"), invalidPubKey, "unwrapping-id")
	Assert(t, err != nil, "Unwrapping key pairs with invalid invalid publicKey should fail.")

	// InvalidTargetKeyPairID validation
	_, err = keyMgr.UnwrapKeyPair("target-pair-id", []byte("key"), privKey.PublicKey, invalidUnwrappingID)
	Assert(t, err != nil, "Unwrapping key pairs with invalid unwrappingID should fail.")
}
