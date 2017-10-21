// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// +build linux

package certmgr

import (
	"os"
	"testing"

	"github.com/pensando/sw/venice/utils/keymgr"
)

// Setup a SoftHSM sandbox for the tests in this package

func setup() {
	err := keymgr.CreateSoftHSMSandbox()
	if err != nil {
		panic("Error creating SoftHSM sandbox")
	}
}

func shutdown() {
	keymgr.DestroySoftHSMSandbox()
}

func TestMain(m *testing.M) {
	setup()
	code := m.Run()
	shutdown()
	os.Exit(code)
}
