// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"testing"

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
	defer be.Close()
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
}
