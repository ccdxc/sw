// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// Functions to return default TLS provider

package rpckit

import (
	"sync"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/testenv"
)

var certSrvEndpoint = "localhost:" + globals.CMDCertAPIPort

var mu sync.Mutex
var defaultTLSProvider TLSProvider

// GetDefaultTLSProvider returns the default TLS provider.
// svcName is needed by the TLS provider to acquire a valid certificate
func GetDefaultTLSProvider(svcName string) (TLSProvider, error) {
	if testenv.GetRpckitTestMode() {
		if testModeDefaultTLSProvider != nil {
			tp, err := testModeDefaultTLSProvider(svcName)
			return tp, err
		}
		return nil, nil
	}
	mu.Lock()
	defer mu.Unlock()
	if defaultTLSProvider != nil {
		return defaultTLSProvider, nil
	}
	tp, err := tlsproviders.NewDefaultCMDBasedProvider(certSrvEndpoint, svcName)
	if err != nil {
		log.Errorf("Error getting CMD-based TLS provider for service %s at %s", svcName, certSrvEndpoint)
		return nil, err
	}
	defaultTLSProvider = tp
	return defaultTLSProvider, nil
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
