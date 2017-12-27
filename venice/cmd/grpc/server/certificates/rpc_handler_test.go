// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certificates

import (
	"bytes"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"reflect"
	"testing"

	"github.com/pkg/errors"
	"golang.org/x/net/context"

	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/keymgr"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// newRPCServer creates a new instance of the certificates server
func newRPCServer(serverName, listenURL string, cm *certmgr.CertificateMgr) (*rpckit.RPCServer, error) {
	// empty url might be valid, so we let rpckit check it and return an error if it is not
	if serverName == "" || cm == nil {
		return nil, errors.New("All parameters are required")
	}
	// create an RPC server
	rpcServer, err := rpckit.NewRPCServer(serverName, listenURL)
	if err != nil {
		return nil, errors.Wrap(err, "Error creating rpc server")
	}
	// Instantiate handlers for certificates API
	srv := NewRPCHandler(cm)
	// register the RPC handlers
	certapi.RegisterCertificatesServer(rpcServer.GrpcServer, srv)
	// start
	rpcServer.Start()
	return rpcServer, nil
}

func TestCertificatesRPC(t *testing.T) {
	// Instantiate Key Manager and Certificate Manager
	be, err := keymgr.NewDefaultBackend("rpctest")
	AssertOk(t, err, "Error instantiating KeyMgr default backend")
	defer be.Close()
	km, err := keymgr.NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")
	cm, err := certmgr.NewCertificateMgr(km)
	AssertOk(t, err, "Error instantiating certificate manager")
	// create RPC server
	serverName := "testCmd"
	rpcServer, err := newRPCServer(serverName, "localhost:0", cm)
	defer rpcServer.Stop()
	AssertOk(t, err, "Error creating RPC server")
	serverURL := rpcServer.GetListenURL()
	// create RPC client
	rpcClient, err := rpckit.NewRPCClient(serverName, serverURL)
	defer rpcClient.ClientConn.Close()
	AssertOk(t, err, "Error creating RPC client")
	client := certapi.NewCertificatesClient(rpcClient.ClientConn)

	// Make sure we throw errors and handle gracefully the case in which we get
	// API calls before the CA is ready
	_, err = client.GetCaTrustChain(context.Background(), &certapi.Empty{})
	Assert(t, err != nil, "API call did not return an error when CA not ready")
	_, err = client.GetTrustRoots(context.Background(), &certapi.Empty{})
	Assert(t, err != nil, "API call did not return an error when CA not ready")
	_, err = client.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{})
	Assert(t, err != nil, "API call did not return an error when CA not ready")

	// Start the CA
	err = cm.StartCa(true)
	AssertOk(t, err, "Error starting certificate manager CA")
	// Fetch trust roots and compare against those in Certificate Manager
	rootsResp, err := client.GetTrustRoots(context.Background(), &certapi.Empty{})
	AssertOk(t, err, "Error fetching trust roots")
	cmTrustRoots := cm.Ca().TrustRoots()
	trustRoots := x509.NewCertPool()
	for i, r := range rootsResp.GetTrustRoots() {
		trustRoots.AddCert(cmTrustRoots[i])
		if !reflect.DeepEqual(r.GetCertificate(), cmTrustRoots[i].Raw) {
			t.Fatalf("TrustRoots() RPC response does not match CM trust roots.\nHave: %+v\nWant:%+v", r.GetCertificate(), cmTrustRoots[i].Raw)
		}
	}
	// We have bootstrapped the CA with a self-signed certificate, so the signing key
	// certificate should be part of the trust roots
	tcs, err := client.GetCaTrustChain(context.Background(), &certapi.Empty{})
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
	signResp, err := client.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: csr.Raw})
	AssertOk(t, err, "Error invoking SignCertificateRequest RPC")
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
	_, err = client.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: nil})
	Assert(t, (err != nil), "SignCertificateRequest with nil CSR succeeded while expected to fail")
	// empty CSR
	emptyCsr := &x509.CertificateRequest{}
	_, err = client.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: emptyCsr.Raw})
	Assert(t, (err != nil), "SignCertificateRequest with empty CSR succeeded while expected to fail")
	// CSR with invalid signature
	invalidSigCsr := make([]byte, len(csr.Raw))
	copy(invalidSigCsr, csr.Raw)
	// Signature is at the end, so change the last byte to make it invalid
	invalidSigCsr[len(invalidSigCsr)-1]++
	_, err = client.SignCertificateRequest(context.Background(), &certapi.CertificateSignReq{Csr: invalidSigCsr})
	Assert(t, (err != nil), "SignCertificateRequest with tampered CSR succeeded while expected to fail")
}
