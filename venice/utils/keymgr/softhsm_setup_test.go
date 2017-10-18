// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"os"
	"testing"
)

// Setup a SoftHSM for the tests in this package
func setup() {
	err := CreateSoftHSMSandbox()
	if err != nil {
		panic("Error creating SoftHSM sandbox")
	}
}

func shutdown() {
	DestroySoftHSMSandbox()
}

func TestMain(m *testing.M) {
	setup()
	code := m.Run()
	shutdown()
	os.Exit(code)
}
