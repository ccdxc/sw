// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"testing"
)

func TestAll(t *testing.T) {
	// On Mac, execute tests only for GoCrypto backend
	gcbe := newGoCryptoBackend(t)
	defer gcbe.Close()
	backends := []Backend{gcbe}
	testAll(t, backends)
}
