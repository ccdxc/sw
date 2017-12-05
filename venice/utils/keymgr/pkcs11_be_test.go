// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// +build linux

package keymgr

import (
	"fmt"
	"io/ioutil"
	"os"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func newPkcs11Backend(t *testing.T) *Pkcs11Backend {
	be, err := NewPkcs11Backend(Pkcs11Lib, "pkcs11be-keymgr", "1234")
	AssertOk(t, err, "Error creating Pkcs11Backend")
	return be
}

func TestInitPkcs11Backend(t *testing.T) {
	// NEGATIVE TEST-CASES
	// empty module path
	_, err := NewPkcs11Backend("", "hello", "1234")
	Assert(t, err != nil, "NewPkcs11Backend succeeded when expected to fail")

	// invalid module
	tmpFile, err := ioutil.TempFile("", "example")
	defer os.Remove(tmpFile.Name())
	AssertOk(t, err, "Error creating temporary file")
	_, err = NewPkcs11Backend(tmpFile.Name(), "hello", "1234")
	Assert(t, err != nil, "NewPkcs11Backend succeeded when expected to fail")

	// POSITIVE TEST-CASES
	// single backend
	be := newPkcs11Backend(t)
	defer be.Close()
	hsmInfo, err := be.GetInfo()
	AssertOk(t, err, "Error getting HSM info")
	t.Logf("HSM module: %+v", hsmInfo)

	if HasMultiTokenSupport(hsmInfo) {
		for i := 0; i < 10; i++ {
			tokenLabel := fmt.Sprintf("keymgr_test-%d", i)
			pin := fmt.Sprintf("1234%d", i)
			be, err := NewPkcs11Backend(Pkcs11Lib, tokenLabel, pin)
			AssertOk(t, err, "Error instantiating backend")
			defer be.Close()
			be2, err := NewPkcs11Backend(Pkcs11Lib, tokenLabel, pin)
			AssertOk(t, err, "Error instantiating backend")
			defer be2.Close()
		}
	}
}

func TestClosePkcs11Backend(t *testing.T) {
	be := newPkcs11Backend(t)
	_, err := be.CreateKeyPair("key1", ECDSA256)
	AssertOk(t, err, "Error creating keypair")
	be.Close()
	// Check that the object has been wiped out
	be = newPkcs11Backend(t)
	key, err := be.GetObject("key1", ObjectTypeKeyPair)
	AssertOk(t, err, "Error creating Pkcs11Backend")
	Assert(t, key == nil, "Found leftover object after Close()")
}
