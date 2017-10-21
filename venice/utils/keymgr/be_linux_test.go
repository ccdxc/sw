// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"testing"
)

func TestAll(t *testing.T) {
	// On Linux, Execute all tests for GoCrypto and Pkcs11 backends.
	gcbe := newGoCryptoBackend(t)
	defer gcbe.Close()
	pkbe := newPkcs11Backend(t)
	defer pkbe.Close()
	backends := []Backend{gcbe, pkbe}
	testAll(t, backends)
}
