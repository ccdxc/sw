// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"fmt"
	"io/ioutil"
	"os"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func newPkcs11Backend(t *testing.T) *Pkcs11Backend {
	be, err := NewPkcs11Backend(Pkcs11Lib, "keymgr", "1234")
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

	// SoftHSM versions before 2.2.0 do not automatically make extra uninitialized tokens
	// available when a token is initialized. https://github.com/opendnssec/SoftHSMv2/pull/198
	// In this case we cannot test multiple backends with different tokens on the same module.
	if (hsmInfo.ManufacturerID != "SoftHSM") ||
		(hsmInfo.LibraryVersion.Major >= 2 && hsmInfo.LibraryVersion.Minor >= 2) {
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

func TestPkcs11BackendObjectStore(t *testing.T) {
	be := newPkcs11Backend(t)
	defer be.Close()
	testObjectStore(t, be)
}

func TestPkcs11BackendKeyOps(t *testing.T) {
	be := newPkcs11Backend(t)
	defer be.Close()
	testKeyOps(t, be)
}

func TestPkcs11BackendKeyImport(t *testing.T) {
	be := newPkcs11Backend(t)
	defer be.Close()
	testKeyImport(t, be)
}

func TestPkcs11BackendWarmStart(t *testing.T) {
	be := newPkcs11Backend(t)
	defer be.Close()
	testWarmStart(t, be)
}

func TestPkcs11BackendParallelAccess(t *testing.T) {
	be := newPkcs11Backend(t)
	defer be.Close()
	testParallelAccess(t, be)
}
