// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"testing"
)

func TestAll(t *testing.T) {
	// On Linux, Execute all tests for GoCrypto and Pkcs11 backends.
	pgcbe := newGoCryptoBackend(t, true) // persistent
	defer pgcbe.Close()
	vgcbe := newGoCryptoBackend(t, false) // volatile
	defer vgcbe.Close()
	pkbe := newPkcs11Backend(t)
	defer pkbe.Close()
	backends := []Backend{pgcbe, vgcbe, pkbe}
	testAll(t, backends)
}
