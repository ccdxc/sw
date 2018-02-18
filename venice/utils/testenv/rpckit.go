// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// Test-mode parameters for rpckit

package testenv

// rpckitTestMode determines whether rpckit is running in test mode or not
var rpckitTestMode bool

// EnableRpckitTestMode switches rpckit to a mode in which the default TLS provider can be overridden.
// It is invoked by utils/testutils init() function.
// Default TLS provider for test mode is nil. Custom provider can be set using rpckit.SetTestModeDefaultTLSProvider
func EnableRpckitTestMode() {
	rpckitTestMode = true
}

// GetRpckitTestMode returns true if rpckit is set to execute in test mode
func GetRpckitTestMode() bool {
	return rpckitTestMode
}
