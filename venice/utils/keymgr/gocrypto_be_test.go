// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"io/ioutil"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestGoCryptoBackendInit(t *testing.T) {
	// NEGATIVE TEST-CASES
	// empty dir
	_, err := NewGoCryptoBackend("")
	Assert(t, err != nil, "NewCertificateAuthority succeeded with empty dir")
	// invalid dir
	_, err = NewGoCryptoBackend("\x00")
	Assert(t, err != nil, "NewCertificateAuthority succeeded with invalid dir")
}

func newGoCryptoBackend(t *testing.T) Backend {
	dir, err := ioutil.TempDir("", "gocrypto_be_test")
	AssertOk(t, err, "Error creating temp dir")
	be, err := NewGoCryptoBackend(dir)
	AssertOk(t, err, "Error instantiating GoCrypto backend")
	return be
}
