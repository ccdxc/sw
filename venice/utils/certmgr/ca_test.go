// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certmgr

import (
	"crypto/ecdsa"
	"crypto/x509"
	"fmt"
	"path"
	"reflect"
	"testing"

	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/keymgr"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testCertsDir          = "testdata"
	caKeyFileName         = "ca.key.pem"
	caCertificateFileName = "ca.cert.pem"
	intermediatesFileName = "intermediates.cert.pem"
	trustRootsFileName    = "roots.pem"
)

// create KeyMgr instance with default backend
func createDefaultKeyMgr(t *testing.T) (*keymgr.KeyMgr, keymgr.Backend) {
	be, err := keymgr.NewDefaultBackend()
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	km, err := keymgr.NewKeyMgr(be)
	if err != nil {
		be.Close() // cleanup
	}
	AssertOk(t, err, "Error instantiating KeyMgr")
	return km, be
}

func TestCaInit(t *testing.T) {
	// NEGATIVE TEST-CASES

	// nil KeyMgr
	_, err := NewCertificateAuthority(nil)
	Assert(t, err != nil, "NewCertificateAuthority succeeded nil KeyMgr")
}

// Pre-load CA keys and certificates in key manager
func preloadCAKeys(t *testing.T, km *keymgr.KeyMgr) (*keymgr.KeyPair, *x509.Certificate, []*x509.Certificate) {
	caKeyPath := path.Join(testCertsDir, caKeyFileName)
	caCertificatePath := path.Join(testCertsDir, caCertificateFileName)
	trustRootsPath := path.Join(testCertsDir, trustRootsFileName)

	caKey, err := certs.ReadPrivateKey(caKeyPath)
	AssertOk(t, err, "Error reading private key")
	kp := keymgr.NewKeyPairObject(caKeyID, caKey.(*ecdsa.PrivateKey))
	err = km.StoreObject(kp)
	AssertOk(t, err, "Error storing private key")

	caCertificate, err := certs.ReadCertificate(caCertificatePath)
	AssertOk(t, err, "Error reading CA certificate")
	err = km.StoreObject(keymgr.NewCertificateObject(caCertificateID, caCertificate))
	AssertOk(t, err, "Error storing certificate object")

	trustRoots, err := certs.ReadCertificates(trustRootsPath)
	AssertOk(t, err, "Error reading intermediate trusted roots")
	err = km.StoreObject(keymgr.NewCertificateBundleObject(trustRootsBundleName, trustRoots))
	AssertOk(t, err, "Error storing trust roots bundle")

	return kp, caCertificate, trustRoots
}

func cleanupCAKeys(t *testing.T, km *keymgr.KeyMgr) {
	km.DestroyObject(caKeyID, keymgr.ObjectTypeKeyPair)
	km.DestroyObject(caCertificateID, keymgr.ObjectTypeCertificate)
	km.DestroyObject(intermediatesBundleName, keymgr.ObjectTypeCertificateBundle)
	km.DestroyObject(trustRootsBundleName, keymgr.ObjectTypeCertificateBundle)
}

func TestSelfSignedFlow(t *testing.T) {
	// create keymgr instance
	km, be := createDefaultKeyMgr(t)
	defer be.Close()

	// test the bootstrap case where we start with no keys and certs
	ca, err := NewCertificateAuthority(km)
	AssertOk(t, err, "Error instantiating new CA")
	defer cleanupCAKeys(t, km)
	Assert(t, ca.IsReady(), "Certificates service failed to start")

	// Check that CA key and certificates have been generated properly
	caKeyPairObj, err := km.GetObject(caKeyID, keymgr.ObjectTypeKeyPair)
	Assert(t, err == nil && caKeyPairObj != nil, "Error retrieving private key")
	caKeyPair := caKeyPairObj.(*keymgr.KeyPair)

	caCertificateObj, err := km.GetObject(caCertificateID, keymgr.ObjectTypeCertificate)
	Assert(t, err == nil && caCertificateObj != nil, "Error retrieving CA certificate")
	caCertificate := caCertificateObj.(*keymgr.Certificate).Certificate

	Assert(t, certs.IsSelfSigned(caCertificate), fmt.Sprintf("Expected self-signed certificate, found one with\nIssuer: %+v\nSubject: %+v", caCertificate.Issuer, caCertificate.Subject))

	// validate that public and private key match
	valid, err := certs.ValidateKeyCertificatePair(caKeyPair.Signer, caCertificate)
	Assert(t, valid && (err == nil), fmt.Sprintf("Public and private key do not match, error: %v", err))

	// check that the self-signed certificate is the only trust root
	Assert(t, len(ca.TrustRoots()) == 1, "Found unexpected number of trust roots, want: 1, have: %d", len(ca.TrustRoots()))
	Assert(t, caCertificate.Equal(&ca.TrustRoots()[0]), "CA certificate not found in trusted roots")

	// check that the ca trust chain contains only the self-signed cert
	Assert(t, caCertificate.Equal(&ca.TrustChain()[0]), "CA certificate not found in CA trust chain")
	Assert(t, len(ca.TrustChain()) == 1, "Found unexpected certificate in trust chain")

	// Now instantiate a new CA with the same KeyManager
	// It should read the exact same keys and certificates
	ca2, err := NewCertificateAuthority(km)
	AssertOk(t, err, "Error instantiating new CA")

	if !reflect.DeepEqual(ca, ca2) {
		t.Fatalf("New instance of certificates service does not match reference\n"+
			"Expected: %+v\n\nFound: %+v\n", ca, ca2)
	}
}

func TestExternalCAFlow(t *testing.T) {
	// create keymgr instance
	km, be := createDefaultKeyMgr(t)
	defer be.Close()

	// Pre-load keys and certificates in key manager from files
	caKeyPair, caCertificate, trustRoots := preloadCAKeys(t, km)

	// Instantiate CA
	ca, err := NewCertificateAuthority(km)
	AssertOk(t, err, "Error instantiating certificate authority")
	Assert(t, ca.IsReady(), "Certificate authority not ready")
	defer cleanupCAKeys(t, km)

	// compare against a reference instance
	refCa := &CertificateAuthority{
		keyMgr:        km,
		caKey:         caKeyPair,
		caCertificate: caCertificate,
		trustChain:    []*x509.Certificate{caCertificate, trustRoots[0]},
		trustRoots:    trustRoots,
		ready:         true,
	}

	if !reflect.DeepEqual(ca, refCa) {
		t.Fatalf("New instance of certificates service does not match reference\n"+
			"Expected: %+v\nFound: %+v\n", ca, refCa)
	}
}

func TestCertificateVerification(t *testing.T) {
	// create keymgr instance
	km, be := createDefaultKeyMgr(t)
	defer be.Close()

	// Pre-load keys and certificates in key manager from files
	_, _, _ = preloadCAKeys(t, km)

	ca, err := NewCertificateAuthority(km)
	AssertOk(t, err, "Error instantiating certificate authority")
	Assert(t, ca.IsReady(), "Certificate authority not ready")
	defer cleanupCAKeys(t, km)

	// Open a certificate that was signed by this CA and check that it passes verification
	testCertPath := path.Join(testCertsDir, "nic-secp256r1-ecdsa.cert.pem")
	testCert, err := certs.ReadCertificate(testCertPath)
	AssertOk(t, err, "Error reading test certificate")

	valid, err := ca.Verify(testCert)
	Assert(t, valid && (err == nil), "Error verifying certificate")

	// Now tamper with the certificate and check that validation fails
	testCert.RawSubject[0] = testCert.RawSubject[0] + 1
	valid, err = ca.Verify(testCert)
	Assert(t, !valid, "Certificate verification did not fail as expected")
}

func TestCertificatesApi(t *testing.T) {
	// create keymgr instance
	km, be := createDefaultKeyMgr(t)
	defer be.Close()

	// Instantiate a new CA with self-signed certificate
	ca, err := NewCertificateAuthority(km)
	AssertOk(t, err, "Error instantiating certificate authority")
	Assert(t, ca.IsReady(), "CA not ready")
	defer cleanupCAKeys(t, km)

	key, err := km.CreateKeyPair("testkey", keymgr.ECDSA256)
	AssertOk(t, err, "Error generating private key")

	csr, err := certs.CreateCSR(key, nil, nil)
	AssertOk(t, err, "Error generating CSR")

	cert, err := ca.Sign(csr)
	AssertOk(t, err, "Error signing CSR")

	valid, err := ca.Verify(cert)
	Assert(t, valid && (err == nil), "Error verifying certificate")
}
