// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certmgr

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"reflect"
	"testing"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/certs"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testCertsDir = "testdata"
)

func TestCaInit(t *testing.T) {
	// NEGATIVE TEST-CASES

	// empty dir
	_, err := NewCertificateAuthority("")
	Assert(t, err != nil, "NewCertificateAuthority succeeded with empty dir")

	// dir exists but is not writable
	dir, err := ioutil.TempDir("", "certsvc")
	defer os.RemoveAll(dir)
	AssertOk(t, err, "Error creating temporary directory")
	err = os.Chmod(dir, 0400)
	AssertOk(t, err, "Error changing permissions on temporary directory")
	_, err = NewCertificateAuthority(dir)
	Assert(t, err != nil, "NewCertificateAuthority succeeded with invalid dir")

	// path exists but is a file
	tmpfile, err := ioutil.TempFile("", "ca")
	AssertOk(t, err, "Error creating temporary directory")
	defer os.RemoveAll(tmpfile.Name())
	_, err = NewCertificateAuthority(tmpfile.Name())
	Assert(t, err != nil, "NewCertificateAuthority succeeded with invalid dir")
}

func TestSelfSignedFlow(t *testing.T) {
	dir, err := ioutil.TempDir("", "certsvc")
	defer os.RemoveAll(dir)
	AssertOk(t, err, "Error creating temporary directory")

	caKeyPath := path.Join(dir, caKeyFileName)
	caCertificatePath := path.Join(dir, caCertificateFileName)

	// first test the bootstrap case where we start with no keys and certs
	cs, err := NewCertificateAuthority(dir)
	AssertOk(t, err, "Error instantiating new CA")
	Assert(t, cs.IsReady(), "Certificates service failed to start")

	caKey, err := certs.ReadPrivateKey(caKeyPath)
	AssertOk(t, err, "Error generating private key")

	caCertificate, err := certs.ReadCertificate(caCertificatePath)
	AssertOk(t, err, "Error generating CA certificate")

	Assert(t, certs.IsSelfSigned(caCertificate), fmt.Sprintf("Expected self-signed certificate, found one with\nIssuer: %+v\nSubject: %+v", caCertificate.Issuer, caCertificate.Subject))

	// validate that public and private key match
	valid, err := certs.ValidateKeyCertificatePair(caKey, caCertificate)
	Assert(t, valid && (err == nil), fmt.Sprintf("Public and private key do not match, error: %v", err))

	// check that the self-signed certificate is the only trust root
	Assert(t, caCertificate.Equal(&cs.TrustRoots()[0]), "CA certificate not found in trusted roots")
	Assert(t, len(cs.TrustRoots()) == 1, "Found unexpected certificate in trust roots")

	// check that the ca trust chain contains only the self-signed cert
	Assert(t, caCertificate.Equal(&cs.TrustChain()[0]), "CA certificate not found in CA trust chain")
	Assert(t, len(cs.TrustChain()) == 1, "Found unexpected certificate in trust chain")

	// Now instantiate a new Certificates Service on the same directory
	// It should read the exact same certificates
	cs2, err := NewCertificateAuthority(dir)
	AssertOk(t, err, "Error instantiating new CA")

	if !reflect.DeepEqual(cs, cs2) {
		t.Fatalf("New instance of certificates service does not match reference\n"+
			"Expected: %+v\n\nFound: %+v\n", cs, cs2)
	}
}

func createSandbox(t *testing.T, srcDir string, permissions os.FileMode) (string, error) {
	sandbox, err := ioutil.TempDir("", "casandbox")
	if err != nil {
		return "", errors.Wrap(err, "Error creating sandbox directory")
	}
	err = os.Chmod(sandbox, permissions)
	if err != nil {
		return "", errors.Wrap(err, "Error creating setting permissions on sandbox")
	}

	// create symbolic links to original files
	dir, err := os.Open(srcDir)
	if err != nil {
		os.RemoveAll(sandbox)
		return "", errors.Wrap(err, "Error opening src dir")
	}
	files, err := dir.Readdir(-1)
	if err != nil {
		os.RemoveAll(sandbox)
		return "", errors.Wrap(err, "Error reading src dir")
	}
	wd, err := os.Getwd()
	if err != nil {
		os.RemoveAll(sandbox)
		return "", errors.Wrap(err, "Error getting working directory")
	}
	for _, f := range files {
		err = os.Symlink(path.Join(wd, srcDir, f.Name()), path.Join(sandbox, f.Name()))
		if err != nil {
			os.RemoveAll(sandbox)
			return "", errors.Wrapf(err, "Error linking file %v in sandbox", f.Name())
		}
	}

	return sandbox, nil
}

func TestExternalCAFlow(t *testing.T) {
	// CA won't start if the directory permissions are not right,
	// but git does not preserve permissions, so we need to create a temp dir from scratch
	dir, err := createSandbox(t, testCertsDir, 0700)
	AssertOk(t, err, "Error creating sandbox")
	defer os.RemoveAll(dir)
	cs, err := NewCertificateAuthority(dir)
	AssertOk(t, err, "Unable to start certificates service")

	// Check that all keys and certificates have been loaded properly
	caKeyPath := path.Join(dir, caKeyFileName)
	caCertificatePath := path.Join(dir, caCertificateFileName)
	trustRootsPath := path.Join(dir, trustRootsFileName)

	caKey, err := certs.ReadPrivateKey(caKeyPath)
	AssertOk(t, err, "Error reading private key")

	caCertificate, err := certs.ReadCertificate(caCertificatePath)
	AssertOk(t, err, "Error reading CA certificate")

	readCerts, err := certs.ReadCertificates(trustRootsPath)
	AssertOk(t, err, "Error reading intermediate trusted roots")

	var trustRoots []*x509.Certificate
	for _, cert := range readCerts {
		trustRoots = append(trustRoots, cert)
	}

	cs2 := &CertificateAuthority{
		configDir:     dir,
		caKey:         caKey,
		caCertificate: *caCertificate,
		trustChain:    []*x509.Certificate{caCertificate, trustRoots[0]},
		trustRoots:    trustRoots,
		ready:         true,
	}

	if !reflect.DeepEqual(cs, cs2) {
		t.Fatalf("New instance of certificates service does not match reference\n"+
			"Expected: %+v\nFound: %+v\n", cs, cs2)
	}
}

func TestCertificateVerification(t *testing.T) {
	// CA won't start if the directory permissions are not right,
	// but git does not preserve permissions, so we need to create a temp dir from scratch
	dir, err := createSandbox(t, testCertsDir, 0700)
	AssertOk(t, err, "Error creating sandbox")
	defer os.RemoveAll(dir)
	cs, err := NewCertificateAuthority(dir)
	AssertOk(t, err, "Error starting certificates service")

	testCertPath := path.Join(dir, "nic-secp256r1-ecdsa.cert.pem")
	testCert, err := certs.ReadCertificate(testCertPath)
	AssertOk(t, err, "Error reading test certificate")

	valid, err := cs.Verify(testCert)
	Assert(t, valid && (err == nil), "Error verifying certificate")

	// Now tamper with the certificate and see if validation fails
	testCert.RawSubject[0] = testCert.RawSubject[0] + 1
	valid, err = cs.Verify(testCert)
	Assert(t, !valid, "Certificate verification did not fail as expected")
}

func TestCertificatesApi(t *testing.T) {
	dir, err := ioutil.TempDir("", "certsvc")
	AssertOk(t, err, fmt.Sprintf("Error creating temporary directory %s", dir))
	defer os.RemoveAll(dir)

	// Instantiate a new, self-signed CA
	cs, err := NewCertificateAuthority(dir)
	Assert(t, err == nil && cs.IsReady(), "Error instantiating certificate authority")

	caKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating private key")

	csr, err := certs.CreateCSR(caKey, nil, nil)
	AssertOk(t, err, "Error generating CSR")

	cert, err := cs.Sign(csr)
	AssertOk(t, err, "Error signing CSR")

	valid, err := cs.Verify(cert)
	Assert(t, valid && (err == nil), "Error verifying certificate")
}
