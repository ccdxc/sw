// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package mock

import (
	"bytes"
	"context"
	"crypto/x509"
	"fmt"
	"net"
	"testing"

	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	"github.com/pensando/sw/venice/utils/certs"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	certPath  = "../../../../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../../../../venice/utils/certmgr/testdata/roots.pem"
)

func TestCertSrvInit(t *testing.T) {
	// NEGATIVE TEST CASES

	// missing URL
	_, err := NewCertSrv("", certPath, keyPath, rootsPath)
	Assert(t, err != nil, "CertSrv mock started while expected to fail")

	// missing paths
	_, err = NewCertSrv("localhost:0", "", keyPath, rootsPath)
	Assert(t, err != nil, "CertSrv mock started while expected to fail")
	_, err = NewCertSrv("localhost:0", certPath, "", rootsPath)
	Assert(t, err != nil, "CertSrv mock started while expected to fail")
	_, err = NewCertSrv("localhost:0", certPath, keyPath, "")
	Assert(t, err != nil, "CertSrv mock started while expected to fail")
}

func TestCertSrvMockRPC(t *testing.T) {
	certsrv, err := NewCertSrv("localhost:0", certPath, keyPath, rootsPath)
	AssertOk(t, err, "Error creating mock CertSrv instance")
	defer certsrv.Stop()

	Assert(t, certsrv.GetActiveConnCount() == 0, "Active connections should be 0 initially")

	// Compare trust roots -- we know there is only 1 in the file
	trustRootsResp, err := certsrv.GetTrustRoots(context.Background(), &certapi.Empty{})
	AssertOk(t, err, "Error retrieving trust roots from CertSrv endpoint")
	fileRoot, err := certs.ReadCertificate(rootsPath)
	AssertOk(t, err, "Error reading trust roots from file")
	Assert(t, bytes.Equal(trustRootsResp.GetTrustRoots()[0].GetCertificate(), fileRoot.Raw), fmt.Sprintf("Trust roots do not match"))

	// retrieve trust chain. It should match {ca.cert.pem.crt, root.cert.pem}
	trustChainResp, err := certsrv.GetCaTrustChain(context.Background(), &certapi.Empty{})
	AssertOk(t, err, "Error retrieving trust chain from CertSrv endpoint")
	fileCert, err := certs.ReadCertificate(certPath)
	AssertOk(t, err, "Error reading signing cert from file")
	Assert(t, bytes.Equal(trustChainResp.GetCertificates()[0].GetCertificate(), fileCert.Raw),
		"First element of the CA trust chain does not match the CA certificate")
	Assert(t, bytes.Equal(trustChainResp.GetCertificates()[1].GetCertificate(), fileRoot.Raw),
		"Second element of the CA trust chain does not match the root certificate")

	// Get CSR signed and validate.
	key, err := certs.ReadPrivateKey(keyPath)
	AssertOk(t, err, "Error reading private key from file")
	csr, err := certs.CreateCSR(key, []string{"HelloWorld"}, []net.IP{})
	AssertOk(t, err, "Error creating CSR")
	certResp, err := certsrv.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: csr.Raw})
	AssertOk(t, err, "Error getting CSR signed")
	cert, err := x509.ParseCertificate(certResp.GetCertificate().GetCertificate())
	AssertOk(t, err, "Error parsing certificate")

	intermediatePool := x509.NewCertPool()
	intermediatePool.AddCert(fileCert)
	trustRootsPool := x509.NewCertPool()
	trustRootsPool.AddCert(fileRoot)
	chain, err := cert.Verify(x509.VerifyOptions{Intermediates: intermediatePool, Roots: trustRootsPool})
	AssertOk(t, err, "Error verifying signed cert")
	Assert(t, len(chain) >= 1, "Error verifying signed cert")

	// Invalid CSR content
	_, err = certsrv.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: nil})
	Assert(t, (err != nil), "SignCertificateRequest with nil CSR succeeded while expected to fail")

	// empty CSR
	emptyCsr := &x509.CertificateRequest{}
	_, err = certsrv.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: emptyCsr.Raw})
	Assert(t, (err != nil), "SignCertificateRequest with empty CSR succeeded while expected to fail")

	// CSR with invalid signature
	invalidSigCsr := make([]byte, len(csr.Raw))
	copy(invalidSigCsr, csr.Raw)
	// Signature is at the end, so change the last byte to make it invalid
	invalidSigCsr[len(invalidSigCsr)-1]++
	_, err = certsrv.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: invalidSigCsr})
	Assert(t, (err != nil), "SignCertificateRequest with tampered CSR succeeded while expected to fail")
}

func TestStartStopCertSrvMock(t *testing.T) {
	// start
	certsrv, err := NewCertSrv("localhost:0", certPath, keyPath, rootsPath)
	AssertOk(t, err, "Error creating mock CertSrv instance")
	certsrvListenURL := certsrv.GetListenURL()

	// stop
	certsrv.Stop()

	// restart
	certsrv, err = NewCertSrv(certsrvListenURL, certPath, keyPath, rootsPath)
	AssertOk(t, err, "Error restarting mock CertSrv instance")
	defer certsrv.Stop()
}
