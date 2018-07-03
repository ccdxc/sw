// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package certs

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"os"
	"reflect"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func getCertKeyBundle(t *testing.T) (*x509.Certificate, crypto.PrivateKey, []*x509.Certificate) {
	privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating key")

	cert, err := SelfSign("", privateKey, WithValidityDays(1))
	AssertOk(t, err, "Error generating cert")

	bundleNames := []string{"b1", "b2", "b3"}
	bundle := make([]*x509.Certificate, len(bundleNames))

	for i, n := range bundleNames {
		bundle[i], err = SelfSign(n, privateKey, WithValidityDays(1))
		AssertOk(t, err, fmt.Sprintf("Error creating certificate %d: %v", i, err))
	}

	return cert, privateKey, bundle
}

func testIncompleteCredentials(t *testing.T, tmpDir string, cert *x509.Certificate, key crypto.PrivateKey, bundle []*x509.Certificate) {
	Assert(t, cert == nil || key == nil || bundle == nil, "This function expects incomplete credentials")
	err := StoreTLSCredentials(cert, key, bundle, tmpDir, 0700)
	Assert(t, err != nil, "Store TLS credentials did not fail with incomplete credentials")
	files, err := ioutil.ReadDir(tmpDir)
	AssertOk(t, err, fmt.Sprintf("Error reading directory %s: %v", tmpDir, err))
	Assert(t, len(files) == 0, fmt.Sprintf("Credentials not cleaned up after failure, found: %+v", files))
}

func TestTLSNegativeTestCases(t *testing.T) {
	_, err := LoadTLSCredentials("idonotexist")
	Assert(t, err != nil, "TestTLSNegativeTestCases succeeded with non-existing directory")
	_, err = LoadTLSCredentials("/etc")
	Assert(t, err != nil, "TestTLSNegativeTestCases succeeded with non-existing credentials")
	tmpDir, err := ioutil.TempDir("", "certs_tls_test")
	AssertOk(t, err, "Error creating temp dir")
	defer os.RemoveAll(tmpDir)
	err = StoreTLSCredentials(nil, nil, nil, tmpDir, 0700)
	Assert(t, err != nil, "Store TLS credentials did not fail with null credentials")
	// After failure, directory should have been cleaned up
	cert, privateKey, bundle := getCertKeyBundle(t)
	testIncompleteCredentials(t, tmpDir, cert, privateKey, nil)
	testIncompleteCredentials(t, tmpDir, cert, nil, bundle)
	testIncompleteCredentials(t, tmpDir, nil, cert, bundle)
}

func TestStoreAndLoadCredentials(t *testing.T) {
	tmpDir, err := ioutil.TempDir("", "certs_tls_test")
	AssertOk(t, err, "Error creating temp dir")
	os.RemoveAll(tmpDir)

	cert, privateKey, bundle := getCertKeyBundle(t)

	perm := os.FileMode(os.ModeDir | 0750)
	err = StoreTLSCredentials(cert, privateKey, bundle, tmpDir, perm)
	AssertOk(t, err, "Error storing credentials")
	defer os.RemoveAll(tmpDir)

	// check permissions
	info, err := os.Stat(tmpDir)
	AssertOk(t, err, "Error getting directory permissions")
	mode := info.Mode()
	Assert(t, mode == perm, fmt.Sprintf("Wrong directory permissions, have: %v, want: %v", mode, perm))

	tlsConfig, err := LoadTLSCredentials(tmpDir)
	AssertOk(t, err, "Error loading credentials")
	tlsCert := tlsConfig.Certificates[0]
	Assert(t, reflect.DeepEqual(tlsCert.PrivateKey, privateKey), fmt.Sprintf("Error loading private key, have: %+v, want: %+v", tlsCert.PrivateKey, privateKey))
	Assert(t, reflect.DeepEqual(tlsCert.Certificate[0], cert.Raw), fmt.Sprintf("Error loading certificate, have: %+v, want: %+v", tlsCert.Certificate[0], cert.Raw))
	certPool := NewCertPool(bundle)
	Assert(t, reflect.DeepEqual(tlsConfig.RootCAs, certPool), fmt.Sprintf("Error loading CA bundle, \nhave: %+v, \nwant: %+v", tlsConfig.RootCAs, certPool))
}
