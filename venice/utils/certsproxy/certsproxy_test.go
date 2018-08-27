// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package certsproxy

import (
	"bytes"
	"context"
	"crypto/x509"
	"fmt"
	"net"
	"testing"

	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	srv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	certPath  = "../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../venice/utils/certmgr/testdata/roots.pem"
)

func TestInit(t *testing.T) {
	// NEGATIVE TESTCASES
	_, err := NewCertsProxy("", "localhost:0")
	Assert(t, err != nil, "Expected an error when listenURL is empty")
	_, err = NewCertsProxy("localhost:0", "")
	Assert(t, err != nil, "Expected an error when listenURL is empty")

	// POSITIVE TESTCASES
	// test options
	balancer := balancer.New(nil)
	tlsProvider := tlsproviders.KeyMgrBasedProvider{}

	_, err = NewCertsProxy("localhost:0", "service", rpckit.WithBalancer(balancer), rpckit.WithTLSProvider(&tlsProvider))
	AssertOk(t, err, "Error instantiating proxy with options")
}

func TestProxy(t *testing.T) {
	certsSrv, err := srv.NewCertSrv("localhost:0", certPath, keyPath, rootsPath)
	AssertOk(t, err, "Error creating certsSrv instance")
	certsSrvURL := certsSrv.GetListenURL()
	fmt.Printf("certsSrvURL: %s\n", certsSrvURL)

	certsProxy, err := NewCertsProxy("localhost:0", certsSrvURL)
	AssertOk(t, err, "Error creating certsProxy instance")
	certsProxy.Start()
	certsProxyURL := certsProxy.GetListenURL()
	defer certsProxy.Stop()

	// create RPC client
	rpcClient, err := rpckit.NewRPCClient("certs-proxy", certsProxyURL)
	AssertOk(t, err, "Error creating RPC client")
	defer rpcClient.Close()
	client := certapi.NewCertificatesClient(rpcClient.ClientConn)

	// Compare trust roots
	trustRootsResp, err := client.GetTrustRoots(context.Background(), &certapi.Empty{})
	AssertOk(t, err, "Error retrieving trust roots from client endpoint")
	fileRoot, err := certs.ReadCertificate(rootsPath)
	AssertOk(t, err, "Error reading trust roots from file")
	Assert(t, bytes.Equal(trustRootsResp.GetCertificates()[0].GetCertificate(), fileRoot.Raw), fmt.Sprintf("Trust roots do not match"))

	// Retrieve trust chain. It should match {ca.cert.pem.crt, root.cert.pem}
	trustChainResp, err := client.GetCaTrustChain(context.Background(), &certapi.Empty{})
	AssertOk(t, err, "Error retrieving trust chain from client endpoint")
	fileCert, err := certs.ReadCertificate(certPath)
	AssertOk(t, err, "Error reading signing cert from file")
	Assert(t, bytes.Equal(trustChainResp.GetCertificates()[0].GetCertificate(), fileCert.Raw),
		"First element of the CA trust chain does not match the CA certificate")
	Assert(t, bytes.Equal(trustChainResp.GetCertificates()[1].GetCertificate(), fileRoot.Raw),
		"Second element of the CA trust chain does not match the root certificate")

	// Get CSR signed and validate.
	key, err := certs.ReadPrivateKey(keyPath)
	AssertOk(t, err, "Error reading private key from file")
	csr, err := certs.CreateCSR(key, nil, []string{"HelloWorld"}, []net.IP{})
	AssertOk(t, err, "Error creating CSR")
	certResp, err := client.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: csr.Raw})
	AssertOk(t, err, "Error getting CSR signed")
	cert, err := x509.ParseCertificate(certResp.GetCertificate().GetCertificate())
	AssertOk(t, err, "Error parsing certificate")
	chain, err := cert.Verify(x509.VerifyOptions{
		Intermediates: certs.NewCertPool([]*x509.Certificate{fileCert}),
		Roots:         certs.NewCertPool([]*x509.Certificate{fileRoot}),
	})
	AssertOk(t, err, "Error verifying signed cert")
	Assert(t, len(chain) >= 1, "Error verifying signed cert")

	// Invalid CSR content -- not forwarded
	_, err = client.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: nil})
	Assert(t, (err != nil), "SignCertificateRequest with nil CSR succeeded while expected to fail")

	// Empty CSR -- not forwarded
	emptyCsr := &x509.CertificateRequest{}
	_, err = client.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: emptyCsr.Raw})
	Assert(t, (err != nil), "SignCertificateRequest with empty CSR succeeded while expected to fail")

	Assert(t, certsSrv.GetRPCSuccessCount() == 3,
		fmt.Sprintf("Unexpected number of succesful RPC calls. Have: %d, Want: %d", certsSrv.GetRPCSuccessCount(), 3))

	Assert(t, certsSrv.GetRPCErrorCount() == 0,
		fmt.Sprintf("Unexpected number of failed RPC calls. Have: %d, Want: %d", certsSrv.GetRPCErrorCount(), 0))

	// NEGATIVE TEST -- remote server is down, all calls should fail
	certsSrv.Stop()
	trustRootsResp, err = client.GetTrustRoots(context.Background(), &certapi.Empty{})
	Assert(t, err != nil, "RPC did not fail with remote server unavailable")
	trustChainResp, err = client.GetCaTrustChain(context.Background(), &certapi.Empty{})
	Assert(t, err != nil, "RPC did not fail with remote server unavailable")
	certResp, err = client.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: csr.Raw})
	Assert(t, err != nil, "RPC did not fail with remote server unavailable")
}
