// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// Test-mode parameters for rpckit

package testenv

import (
	"os"
)

const (
	veniceDevEnvVar = "VENICE_DEV"
)

func init() {
	_, isDevEnv := os.LookupEnv(veniceDevEnvVar)
	if isDevEnv {
		// Switch rpckit to test mode. This disables TLS by default.
		// Tests can instantiate a custom TLS provider and set it using
		// testenv.SetTestModeTLSProvider()
		EnableRpckitTestMode()
	}
}
