// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package mock

import (
	"bytes"
	"context"
	"crypto/x509"
	"fmt"
	"net"
	"testing"

	"github.com/pensando/sw/venice/ctrler/ckm/rpcserver/ckmproto"
	"github.com/pensando/sw/venice/utils/certs"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	certPath  = "../../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../../venice/utils/certmgr/testdata/roots.pem"
)

func TestCkmInit(t *testing.T) {
	// NEGATIVE TEST CASES

	// missing URL
	_, err := NewCKMctrler("", certPath, keyPath, rootsPath)
	Assert(t, err != nil, "CKM mock started while expected to fail")

	// missing paths
	_, err = NewCKMctrler("localhost:0", "", keyPath, rootsPath)
	Assert(t, err != nil, "CKM mock started while expected to fail")
	_, err = NewCKMctrler("localhost:0", certPath, "", rootsPath)
	Assert(t, err != nil, "CKM mock started while expected to fail")
	_, err = NewCKMctrler("localhost:0", certPath, keyPath, "")
	Assert(t, err != nil, "CKM mock started while expected to fail")
}

func TestCkmMockRPC(t *testing.T) {
	ckm, err := NewCKMctrler("localhost:0", certPath, keyPath, rootsPath)
	AssertOk(t, err, "Error creating mock CKM instance")
	defer ckm.Stop()

	// Compare trust roots -- we know there is only 1 in the file
	trustRootsResp, err := ckm.GetTrustRoots(context.Background(), &ckmproto.Empty{})
	AssertOk(t, err, "Error retrieving trust roots from CKM endpoint")
	fileRoot, err := certs.ReadCertificate(rootsPath)
	AssertOk(t, err, "Error reading trust roots from file")
	Assert(t, bytes.Equal(trustRootsResp.GetTrustRoots()[0].GetCertificate(), fileRoot.Raw), fmt.Sprintf("Trust roots do not match"))

	// retrieve trust chain. It should match {ca.cert.pem.crt, root.cert.pem}
	trustChainResp, err := ckm.GetCaTrustChain(context.Background(), &ckmproto.Empty{})
	AssertOk(t, err, "Error retrieving trust chain from CKM endpoint")
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
	certResp, err := ckm.SignCertificateRequest(context.Background(), &ckmproto.CertificateSignReq{Csr: csr.Raw})
	AssertOk(t, err, "Error getting CSR signed")
	Assert(t, certResp.GetApproved() == true, "CKM rejected CSR")
	cert, err := x509.ParseCertificate(certResp.GetCertificate().GetCertificate())

	intermediatePool := x509.NewCertPool()
	intermediatePool.AddCert(fileCert)
	trustRootsPool := x509.NewCertPool()
	trustRootsPool.AddCert(fileRoot)
	chain, err := cert.Verify(x509.VerifyOptions{Intermediates: intermediatePool, Roots: trustRootsPool})
	AssertOk(t, err, "Error verifying signed cert")
	Assert(t, len(chain) >= 1, "Error verifying signed cert")

	// Invalid CSR content
	_, err = ckm.SignCertificateRequest(context.Background(), &ckmproto.CertificateSignReq{Csr: nil})
	Assert(t, (err != nil), "SignCertificateRequest with nil CSR succeeded while expected to fail")

	// empty CSR
	emptyCsr := &x509.CertificateRequest{}
	_, err = ckm.SignCertificateRequest(context.Background(), &ckmproto.CertificateSignReq{Csr: emptyCsr.Raw})
	Assert(t, (err != nil), "SignCertificateRequest with empty CSR succeeded while expected to fail")

	// CSR with invalid signature
	invalidSigCsr := make([]byte, len(csr.Raw))
	copy(invalidSigCsr, csr.Raw)
	// Signature is at the end, so change the last byte to make it invalid
	invalidSigCsr[len(invalidSigCsr)-1]++
	_, err = ckm.SignCertificateRequest(context.Background(), &ckmproto.CertificateSignReq{Csr: invalidSigCsr})
	Assert(t, (err != nil), "SignCertificateRequest with tampered CSR succeeded while expected to fail")
}

func TestStartStopCkmMock(t *testing.T) {
	// start
	ckm, err := NewCKMctrler("localhost:0", certPath, keyPath, rootsPath)
	AssertOk(t, err, "Error creating mock CKM instance")
	ckmListenURL := ckm.GetListenURL()

	// stop
	ckm.Stop()

	// restart
	ckm, err = NewCKMctrler(ckmListenURL, certPath, keyPath, rootsPath)
	AssertOk(t, err, "Error restarting mock CKM instance")
	defer ckm.Stop()
}
