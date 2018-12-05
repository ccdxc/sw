// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package utils

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"

	"fmt"
	"go/build"
	"os"
	"path"
	"sync"

	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/testenv"
)

const (
	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath  = "github.com/pensando/sw/venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "github.com/pensando/sw/venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "github.com/pensando/sw/venice/utils/certmgr/testdata/roots.pem"
)

var (
	mutex        sync.Mutex
	certSrv      *certsrv.CertSrv
	tlsProviders []*tlsproviders.CMDBasedProvider
)

// SetupIntegTLSProvider sets up TLS for integration tests.
// It creates a mock CMD certificates endpoint and points default TLS provider to it.
func SetupIntegTLSProvider() error {
	gopath := os.Getenv("GOPATH")
	if gopath == "" {
		gopath = build.Default.GOPATH
	}

	// start certificate server
	certSrv, err := certsrv.NewCertSrv("localhost:0", path.Join(gopath, "src", certPath), path.Join(gopath, "src", keyPath), path.Join(gopath, "src", rootsPath))
	if err != nil {
		return fmt.Errorf("error starting certificates server: %v", err)
	}
	log.Infof("Created cert endpoint at %s", certSrv.GetListenURL())

	// instantiate a CKM-based TLS provider and make it default for all rpckit clients and servers
	tlsProvider := func(svcName string) (rpckit.TLSProvider, error) {
		p, err := tlsproviders.NewDefaultCMDBasedProvider(certSrv.GetListenURL(), svcName)
		if err != nil {
			return nil, err
		}
		mutex.Lock()
		tlsProviders = append(tlsProviders, p)
		mutex.Unlock()
		return p, nil
	}
	testenv.EnableRpckitTestMode()
	rpckit.SetTestModeDefaultTLSProvider(tlsProvider)
	return nil
}

// CleanupIntegTLSProvider cleans up TLS setup for integration tests
func CleanupIntegTLSProvider() {
	errProvider := func(svcName string) (rpckit.TLSProvider, error) {
		return nil, fmt.Errorf("Suite is being shutdown")
	}
	rpckit.SetTestModeDefaultTLSProvider(errProvider)

	mutex.Lock()
	if tlsProviders != nil {
		for _, t := range tlsProviders {
			t.Close()
		}
		tlsProviders = nil
	}
	mutex.Unlock()

	if certSrv != nil {
		certSrv.Stop()
	}

}

// GetCAKit returns a minimal set of CA artifcacts for tests that need to generate TLS credentials
func GetCAKit() (certs.CSRSigner, *x509.Certificate, []*x509.Certificate, error) {
	caKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		return nil, nil, nil, fmt.Errorf("Error generating CA key: %v", err)
	}
	caCert, err := certs.SelfSign("ca", caKey, certs.WithValidityDays(1))
	if err != nil {
		return nil, nil, nil, fmt.Errorf("Error generating self-signer cert: %v", err)
	}

	csrSigner := func(csr *x509.CertificateRequest) (*x509.Certificate, error) {
		return certs.SignCSRwithCA(csr, caCert, caKey, certs.WithValidityDays(1))
	}

	return csrSigner, caCert, []*x509.Certificate{caCert}, nil
}
