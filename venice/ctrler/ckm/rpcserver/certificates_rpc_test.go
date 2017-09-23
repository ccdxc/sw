// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"bytes"
	"context"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"os"
	"reflect"
	"testing"

	ckmgrpc "github.com/pensando/sw/venice/ctrler/ckm/rpcserver/ckmproto"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestCertificatesRPC(t *testing.T) {
	// create a temporary directory and instantiate Certificate Manager
	keyStoreDir, err := ioutil.TempDir("", "ckmrpcservertest")
	defer os.RemoveAll(keyStoreDir)
	AssertOk(t, err, fmt.Sprintf("Error creating temporary directory %s", keyStoreDir))
	cm, err := certmgr.NewCertificateMgr(keyStoreDir)
	AssertOk(t, err, "Error instantiating certificates manager")

	serverName := "ckm"

	// create RPC server
	rpcServer, err := NewRPCServer(serverName, "localhost:0", cm)
	defer rpcServer.rpcServer.Stop()
	AssertOk(t, err, "Error creating RPC server")
	serverURL := rpcServer.GetListenURL()

	// create RPC client
	rpcClient, err := rpckit.NewRPCClient(serverName, serverURL)
	defer rpcClient.ClientConn.Close()
	AssertOk(t, err, "Error creating RPC client")
	client := ckmgrpc.NewCertificatesClient(rpcClient.ClientConn)

	// Fetch trust roots and compare against those in Certificate Manager
	rootsResp, err := client.GetTrustRoots(context.Background(), &ckmgrpc.Empty{})
	AssertOk(t, err, "Error fetching trust roots")

	cmTrustRoots := cm.Ca().TrustRoots()
	trustRoots := x509.NewCertPool()

	for i, r := range rootsResp.GetTrustRoots() {
		trustRoots.AddCert(&cmTrustRoots[i])
		if !reflect.DeepEqual(r.GetCertificate(), cmTrustRoots[i].Raw) {
			t.Fatalf("TrustRoots() RPC response does not match CM trust roots.\nHave: %+v\nWant:%+v", r.GetCertificate(), cmTrustRoots[i].Raw)
		}
	}

	// We have bootstrapped the CA with a self-signed certificate, so the signing key
	// certificate should be part of the trust roots
	tcs, err := client.GetCaTrustChain(context.Background(), &ckmgrpc.Empty{})
	AssertOk(t, err, "Error fetching CA trust chain")
	caCert := tcs.GetCertificates()[0].GetCertificate()
	var found bool
	for _, r := range rootsResp.GetTrustRoots() {
		if bytes.Equal(r.GetCertificate(), caCert) {
			found = true
			break
		}
	}
	Assert(t, found, "Unable to find self-signed CA cert among trust roots.")

	// Get a CSR signed and compare against asking cm to sign it directly
	const dnsName = "HelloWorld"
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating private key")
	csr, err := certs.CreateCSR(key, []string{dnsName}, nil)
	AssertOk(t, err, "Error creating CSR")

	signResp, err := client.SignCertificateRequest(context.Background(), &ckmgrpc.CertificateSignReq{Csr: csr.Raw})
	AssertOk(t, err, "Error invoking SignCertificateRequest RPC")
	Assert(t, (signResp.GetApproved() == true), "RPC server rejected CSR")

	// we cannot compare against a local Sign() call because timestamps, serial number, etc. might be different
	// so we just verify the certificate
	cert, err := x509.ParseCertificate(signResp.GetCertificate().GetCertificate())
	AssertOk(t, err, "Error invoking SignCertificateRequest RPC")

	verifyOpts := x509.VerifyOptions{
		DNSName: dnsName,
		Roots:   trustRoots,
	}
	_, err = cert.Verify(verifyOpts)
	AssertOk(t, err, "Error verifying certificate returned by SignCertificateRequest RPC")

	// NEGATIVE TEST-CASES

	//  nil CSR
	_, err = client.SignCertificateRequest(context.Background(), &ckmgrpc.CertificateSignReq{Csr: nil})
	Assert(t, (err != nil), "SignCertificateRequest with nil CSR succeeded while expected to fail")

	// empty CSR
	emptyCsr := &x509.CertificateRequest{}
	_, err = client.SignCertificateRequest(context.Background(), &ckmgrpc.CertificateSignReq{Csr: emptyCsr.Raw})
	Assert(t, (err != nil), "SignCertificateRequest with empty CSR succeeded while expected to fail")

	// CSR with invalid signature
	invalidSigCsr := make([]byte, len(csr.Raw))
	copy(invalidSigCsr, csr.Raw)
	// Signature is at the end, so change the last byte to make it invalid
	invalidSigCsr[len(invalidSigCsr)-1]++
	_, err = client.SignCertificateRequest(context.Background(), &ckmgrpc.CertificateSignReq{Csr: invalidSigCsr})
	Assert(t, (err != nil), "SignCertificateRequest with tampered CSR succeeded while expected to fail")

}
