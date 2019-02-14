// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"testing"
)

func TestAll(t *testing.T) {
	// On Mac, execute tests only for GoCrypto backend
	pgcbe := newGoCryptoBackend(t, true) // persistent
	defer pgcbe.Close()
	vgcbe := newGoCryptoBackend(t, false) // volatile
	defer vgcbe.Close()
	backends := []Backend{pgcbe, vgcbe}
	testAll(t, backends)
}
