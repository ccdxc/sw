// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package tlsproviders_test

import (
	"context"
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/keymgr"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func genKeyCert(t *testing.T, name string) (crypto.PrivateKey, *x509.Certificate) {
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating private key")
	cert, err := certs.SelfSign("testca", key, certs.WithValidityDays(1))
	AssertOk(t, err, "Error generating self-signed certificate")
	return key, cert
}

func getCert(t *testing.T, name string, key, caKey crypto.PrivateKey, caCert *x509.Certificate) *x509.Certificate {
	csr, err := certs.CreateCSR(key, nil, []string{name}, nil)
	AssertOk(t, err, "Error creating CSR")
	cert, err := certs.SignCSRwithCA(csr, caCert, caKey, certs.WithValidityDays(1))
	AssertOk(t, err, "Error signing CSR")
	return cert
}

// Generate default credentials and programs them in the provider
func setDefaultCredentials(t *testing.T, p *KeyMgrBasedProvider, caKey crypto.PrivateKey, caCert *x509.Certificate) (crypto.PrivateKey, *x509.Certificate) {
	key, err := p.CreateDefaultKeyPair(keymgr.ECDSA256)
	AssertOk(t, err, "Error creating default keypair")
	cert := getCert(t, "client", key, caKey, caCert)
	err = p.SetDefaultCertificate(cert)
	AssertOk(t, err, "Error setting default certificate")
	return key, cert
}

// Generate credentials for a server and programs them in the provider
func setServerCredentials(t *testing.T, p *KeyMgrBasedProvider, name string, caKey crypto.PrivateKey, caCert *x509.Certificate) (crypto.PrivateKey, *x509.Certificate) {
	key, err := p.CreateServerKeyPair(name, keymgr.ECDSA256)
	AssertOk(t, err, "Error creating server keypair")
	cert := getCert(t, name, key, caKey, caCert)
	err = p.SetServerCertificate(name, cert)
	AssertOk(t, err, "Error setting server certificate")
	return key, cert
}

// Generate credentials for clients and programs them in the provider
func setClientCredentials(t *testing.T, p *KeyMgrBasedProvider, caKey crypto.PrivateKey, caCert *x509.Certificate) (crypto.PrivateKey, *x509.Certificate) {
	key, err := p.CreateClientKeyPair(keymgr.ECDSA256)
	AssertOk(t, err, "Error creating client keypair")
	cert := getCert(t, "client", key, caKey, caCert)
	err = p.SetClientCertificate(cert)
	AssertOk(t, err, "Error setting server certificate")
	return key, cert
}

func verifyCredentials(t *testing.T, p *KeyMgrBasedProvider, name string, key crypto.PrivateKey, cert *x509.Certificate) {
	tlsCert, tr, err := p.GetTLSNamedCredentialsOrDefault(name)
	AssertOk(t, err, "Error getting tls credentials")
	Assert(t, tr != nil, "NIL trust roots")
	Assert(t, reflect.DeepEqual(tlsCert.PrivateKey, key), fmt.Sprintf("Retrieved %s key does not match", name))
	Assert(t, reflect.DeepEqual(tlsCert.Certificate[0], cert.Raw), fmt.Sprintf("Retrieved %s cert does not match", name))
}

func TestKeyMgrBasedProviderConfig(t *testing.T) {
	// NEGATIVE TEST-CASES
	// nil KeyMgr
	p := NewKeyMgrBasedProvider(nil)
	Assert(t, p == nil, "KeyMgrBasedProvider instantiation succceeded with nil KeyMgr")

	// create a real KeyMgr and instantiate the provider
	be, err := keymgr.NewDefaultBackend("rpckit_cmd_test")
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	defer be.Close()
	km, err := keymgr.NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")
	p = NewKeyMgrBasedProvider(km)
	Assert(t, p != nil, "KeyMgrBasedProvider instantiation failed unexpectedly")

	// no trust roots and ca trust chain yet -- should succeed but be empty
	cc, err := p.GetCaTrustChain()
	AssertOk(t, err, "Error getting CA trust chain")
	Assert(t, len(cc) == 0, "CA trust chain should be empty at this point")
	tr, err := p.GetTrustRoots()
	AssertOk(t, err, "Error getting trust roots")
	Assert(t, tr == nil, "Trust roots should be empty at this point")

	// Program valid CA Trust chain and trust roots
	caKey1, caCert1 := genKeyCert(t, "testca1")
	caTrustChain := []*x509.Certificate{caCert1}
	err = p.SetCaTrustChain(caTrustChain)
	AssertOk(t, err, "Error setting trust chain")
	cc, err = p.GetCaTrustChain()
	AssertOk(t, err, "Error getting CA trust chain")
	Assert(t, len(cc) == len(caTrustChain), "CA trust chain has wrong length. Want: %d, have: %d", len(caTrustChain), len(cc))

	_, caCert2 := genKeyCert(t, "testca2")
	trustRoots := []*x509.Certificate{caCert1, caCert2}
	err = p.SetTrustRoots(trustRoots)
	AssertOk(t, err, "Error setting trust roots")
	tr, err = p.GetTrustRoots()
	AssertOk(t, err, "Error getting trust roots")
	Assert(t, len(tr.Subjects()) == len(trustRoots), "Trust roots bundle has wrong length. Want: %d, have: %d", len(trustRoots), len(tr.Subjects()))

	// GetServerOptions and GetDialOptions should still fail at this point
	_, err = p.GetServerOptions("any")
	Assert(t, err != nil, "GetServerOptions did not return error as expected")
	_, err = p.GetDialOptions("any")
	Assert(t, err != nil, "GetDialOptions did not return error as expected")

	// Program default credentials and verify that they are used for both client and server
	defaultKey, defaultCert := setDefaultCredentials(t, p, caKey1, caCert1)
	verifyCredentials(t, p, "any", defaultKey, defaultCert)

	// Now both GetServerOptions and GetDialOptions should succeed
	_, err = p.GetServerOptions("any")
	AssertOk(t, err, "GetServerOptions failed unexpectedly")
	_, err = p.GetDialOptions("any") // servername does not matter
	AssertOk(t, err, "GetDialOptions failed unexpectedly")

	// Generate server-specific keys inside the provider and program certs
	srv1Key, srv1Cert := setServerCredentials(t, p, "srv1", caKey1, caCert1)
	// Verify that srv-specific credentials are returned if found, default otherwise
	verifyCredentials(t, p, "srv1", srv1Key, srv1Cert)
	verifyCredentials(t, p, "any", defaultKey, defaultCert)
	// client should still return default
	verifyCredentials(t, p, KMBClientBaseID, defaultKey, defaultCert)

	// Program client certificate
	// verify that client now returns the client-specific key
	clientKey, clientCert := setClientCredentials(t, p, caKey1, caCert1)
	verifyCredentials(t, p, KMBClientBaseID, clientKey, clientCert)

	// Generate another server-specific key inside the provider and program cert
	srv2Key, srv2Cert := setServerCredentials(t, p, "srv2", caKey1, caCert1)
	verifyCredentials(t, p, "srv1", srv1Key, srv1Cert)
	verifyCredentials(t, p, "srv2", srv2Key, srv2Cert)
	verifyCredentials(t, p, "any", defaultKey, defaultCert)

	// Update server, client and default keys
	newSrv1Key, newSrv1Cert := setServerCredentials(t, p, "srv1", caKey1, caCert1)
	newClientKey, newClientCert := setClientCredentials(t, p, caKey1, caCert1)
	newDefaultKey, newDefaultCert := setDefaultCredentials(t, p, caKey1, caCert1)
	verifyCredentials(t, p, "srv1", newSrv1Key, newSrv1Cert)
	verifyCredentials(t, p, "srv2", srv2Key, srv2Cert)
	verifyCredentials(t, p, "any", newDefaultKey, newDefaultCert)
	verifyCredentials(t, p, KMBClientBaseID, newClientKey, newClientCert)
}

func TestKeyMgrBasedProviderRPC(t *testing.T) {
	serverID := "KMBProviderTestServer"

	// create KeyMgr
	be, err := keymgr.NewDefaultBackend("rpckit_cmd_test")
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	defer be.Close()
	km, err := keymgr.NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")

	// create TLS provider and program keys
	tlsProvider := NewKeyMgrBasedProvider(km)
	Assert(t, tlsProvider != nil, "TLS provider initialization failed")
	caKey, caCert := genKeyCert(t, "KeyMgrBasedRPCTest")
	err = tlsProvider.SetTrustRoots([]*x509.Certificate{caCert})
	AssertOk(t, err, "Error setting trust roots")
	setServerCredentials(t, tlsProvider, serverID, caKey, caCert)
	setClientCredentials(t, tlsProvider, caKey, caCert)

	// create server
	rpcServer, err := rpckit.NewRPCServer(serverID, "localhost:0", rpckit.WithTLSProvider(tlsProvider))
	AssertOk(t, err, "Error creating test server at localhost:0")
	defer rpcServer.Stop()
	testHandler := rpckit.NewTestRPCHandler("dummy message", "test TLS response")
	rpckit.RegisterTestServer(rpcServer.GrpcServer, testHandler)
	rpcServer.Start()

	// create client
	rpcClient, err := rpckit.NewRPCClient("testClient", rpcServer.GetListenURL(), rpckit.WithTLSProvider(tlsProvider), rpckit.WithTracerEnabled(false), rpckit.WithRemoteServerName(serverID))
	defer rpcClient.Close()
	AssertOk(t, err, "Error creating test client")
	testClient := rpckit.NewTestClient(rpcClient.ClientConn)

	// Perform RPC call
	resp, err := testClient.TestRPC(context.Background(), &rpckit.TestReq{ReqMsg: "test TLS request"})
	Assert(t, (testHandler.ReqMsg == "test TLS request"), "Unexpected req msg", testHandler)
	Assert(t, (resp.RespMsg == "test TLS response"), "Unexpected resp msg", resp)
	AssertOk(t, err, "RPC error")
	t.Log("Successfully performed RPC")
}
