// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpckit

import (
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/testenv"
)

var naplesCertSrvURL = "pen-master:" + globals.CMDCertAPIPort

// SetN4STLSProvider overrides the default rpckit TLS provider
// The rpckit default TLS provider expects to pull certificates from a local CMD
// endpoint. Right now we don't have it on NAPLES, so the TLS provider needs to
// fetch certificates from Venice nodes.
// In the future we will have a local endpoint that accepts requests from NAPLES
// processes and proxies them to Venice, so the default rpckit TLS provider will
// work and this code will not be needed any more
func SetN4STLSProvider() {
	if testenv.GetRpckitTestMode() {
		SetTestModeDefaultTLSProvider(nil)
		return
	}
	log.Infof("Setting TLS provider to point to: %v", naplesCertSrvURL)
	tlsProvider := func(svcName string) (TLSProvider, error) {
		p, err := tlsproviders.NewDefaultCKMBasedProvider(naplesCertSrvURL, svcName)
		if err != nil {
			return nil, err
		}
		return p, nil
	}
	testenv.EnableRpckitTestMode()
	SetTestModeDefaultTLSProvider(tlsProvider)
}
