// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpckit

import (
	"sync"
	"time"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/testenv"
)

var (
	naplesCertSrvURL = "pen-master:" + globals.CMDCertAPIPort
	tlsProviderOnce  sync.Once
)

// SetN4STLSProvider overrides the default rpckit TLS provider
// The rpckit default TLS provider expects to pull certificates from a local CMD
// endpoint. Right now we don't have it on NAPLES, so the TLS provider needs to
// fetch certificates from Venice nodes.
// In the future we will have a local endpoint that accepts requests from NAPLES
// processes and proxies them to Venice, so the default rpckit TLS provider will
// work and this code will not be needed any more
func SetN4STLSProvider() {
	tlsProviderOnce.Do(setN4STLSProvider)
}

func setN4STLSProvider() {
	if testenv.GetRpckitTestMode() {
		SetTestModeDefaultTLSProvider(nil)
		return
	}
	log.Infof("Setting TLS provider to point to: %v", naplesCertSrvURL)
	tlsProvider := func(svcName string) (TLSProvider, error) {
		mu.Lock()
		defer mu.Unlock()
		if defaultTLSProvider != nil {
			return defaultTLSProvider, nil
		}
		// FIXME
		// Right now in E2E tests the NAPLES containers can start few mins before Venice cluster is formed.
		// For the moment we just use long retries here, but eventually client should retry
		tp, err := tlsproviders.NewDefaultCMDBasedProvider(naplesCertSrvURL, svcName,
			tlsproviders.WithConnRetryInterval(5*time.Second),
			tlsproviders.WithConnMaxRetries(60))
		if err != nil {
			log.Errorf("Error getting CMD-based TLS provider for service %s at %s", svcName, naplesCertSrvURL)
			return nil, err
		}
		defaultTLSProvider = tp
		return defaultTLSProvider, nil
	}
	testenv.EnableRpckitTestMode()
	SetTestModeDefaultTLSProvider(tlsProvider)
}
