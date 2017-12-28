// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// Functions to return default TLS provider

package rpckit

import (
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/testenv"
)

// GetDefaultTLSProvider returns the default TLS provider.
// svcName is needed by the TLS provider to acquire a valid certificate
func GetDefaultTLSProvider(svcName string) (TLSProvider, error) {
	if testenv.GetRpckitTestMode() {
		if testModeDefaultTLSProvider != nil {
			return testModeDefaultTLSProvider(svcName)
		}
		return nil, nil
	}
	// TODO: for the moment we still return nil as default TLS provider
	// we will switch to CKM-based provider once the server-side is ready
	/*tlsProvider, err := tlsproviders.NewDefaultCKMBasedProvider(ckmEndpoint, svcName)
	if err != nil {
		log.Errorf("Error getting CKM-based TLS provider for service %s at %s", svcName, ckmEndpoint)
		return nil, err
	}
	return tlsProvider, nil
	*/
	return nil, nil
}

// ********** FOR TESTING ONLY **********

// the default TLS provider for test mode
var testModeDefaultTLSProvider func(svcName string) (TLSProvider, error)

// SetTestModeDefaultTLSProvider allows overriding the default TLS provider.
// Can only be invoked when rpckit is in test mode.
// Only meant for use in tests.
func SetTestModeDefaultTLSProvider(tlsProvider func(svcName string) (TLSProvider, error)) {
	if !testenv.GetRpckitTestMode() {
		log.Fatalf("SetTestModeDefaultTLSProvider can only be invoked in test mode")
	}
	testModeDefaultTLSProvider = tlsProvider
}
