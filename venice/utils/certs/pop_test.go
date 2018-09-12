// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package certs

import (
	//"crypto/sha256"
	//"bytes"
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func verifyKeyCertPair(t *testing.T, key crypto.Signer, cert *x509.Certificate) error {
	challenge, err := GeneratePoPNonce()
	AssertOk(t, err, "Error generating challenge")
	claimantRandom, signature, err := GeneratePoPChallengeResponse(key, challenge)
	AssertOk(t, err, "Error generating challenge response")
	return VerifyPoPChallenge(challenge, claimantRandom, signature, cert)
}

func TestGeneratePoPChallengeResponse(t *testing.T) {
	challenge, err := GeneratePoPNonce()
	AssertOk(t, err, "Error generating challenge")
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating key")

	// Negative test cases
	_, _, err = GeneratePoPChallengeResponse(nil, challenge)
	Assert(t, err != nil, "GeneratePoPChallengeResponse did not fail with nil a signer")
	_, _, err = GeneratePoPChallengeResponse(key, nil)
	Assert(t, err != nil, "GeneratePoPChallengeResponse did not fail with nil challenge")
	_, _, err = GeneratePoPChallengeResponse(key, []byte(""))
	Assert(t, err != nil, "GeneratePoPChallengeResponse did not fail with invalid challenge")
	_, _, err = GeneratePoPChallengeResponse(key, []byte("AAAA"))
	Assert(t, err != nil, "GeneratePoPChallengeResponse did not fail with invalid challenge")
	_, _, err = GeneratePoPChallengeResponse(key, []byte("0123456789012345678901234567890123456789"))
	Assert(t, err != nil, "SignPopChallenge did not fail with invalid nonce")
}

func TestVerifyPoPChallenge(t *testing.T) {
	challenge, err := GeneratePoPNonce()
	AssertOk(t, err, "Error generating challenge")
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating key")
	claimantRandom, signature, err := GeneratePoPChallengeResponse(key, challenge)
	AssertOk(t, err, "Error signing challenge")
	cert, err := SelfSign("CA", key, WithValidityDays(1))

	// Negative test cases
	err = VerifyPoPChallenge(nil, claimantRandom, signature, cert)
	Assert(t, err != nil, "VerifyPoPChallenge did not fail without challenge")
	err = VerifyPoPChallenge([]byte("hello"), claimantRandom, signature, cert)
	Assert(t, err != nil, "VerifyPoPChallenge did not fail with invalid challenge")
	err = VerifyPoPChallenge(challenge, nil, signature, cert)
	Assert(t, err != nil, "VerifyPoPChallenge did not fail without claimant nonce")
	err = VerifyPoPChallenge(challenge, []byte("hello"), signature, cert)
	Assert(t, err != nil, "VerifyPoPChallenge did not fail with invalid claimant nonce")
	err = VerifyPoPChallenge(challenge, claimantRandom, nil, cert)
	Assert(t, err != nil, "VerifyPoPChallenge did not fail without signature")
	err = VerifyPoPChallenge(challenge, claimantRandom, []byte("hello"), cert)
	Assert(t, err != nil, "VerifyPoPChallenge did not fail with invalid signature")
	err = VerifyPoPChallenge(challenge, claimantRandom, signature, nil)
	Assert(t, err != nil, "VerifyPoPChallenge did not fail without public key")
}

// TestEndToEnd tests the entire flow
func TestPoPEndToEnd(t *testing.T) {
	curves := []elliptic.Curve{
		elliptic.P224(),
		elliptic.P256(),
		elliptic.P384(),
		elliptic.P521(),
	}

	// Positive test cases for all key types and sizes
	keys := make([]crypto.Signer, 0)
	for _, c := range curves {
		k, err := ecdsa.GenerateKey(c, rand.Reader)
		AssertOk(t, err, "Error generating ECDSA key")
		keys = append(keys, k)
	}
	for _, bits := range []int{1024, 2048, 4096} {
		k, err := rsa.GenerateKey(rand.Reader, bits)
		AssertOk(t, err, "Error generating RSA key")
		keys = append(keys, k)
	}
	for _, key := range keys {
		cert, err := SelfSign("CA", key, WithValidityDays(1))
		AssertOk(t, err, "Error generating cert")
		err = verifyKeyCertPair(t, key, cert)
		AssertOk(t, err, "Error verifying PoP")
	}

	// Negative test cases
	k1, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating ECDSA key")
	k2, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating ECDSA key")
	cert, err := SelfSign("CA", k2, WithValidityDays(1))
	AssertOk(t, err, "Error generating Certificate key")
	err = verifyKeyCertPair(t, k2, cert)
	AssertOk(t, err, "Unexpected failure on valid key cert pair")
	// Wrong key
	err = verifyKeyCertPair(t, k1, cert)
	Assert(t, err != nil, "Unexpected success on invalid key cert pair")
	// Wrong type
	k3, err := rsa.GenerateKey(rand.Reader, 1024)
	AssertOk(t, err, "Error generating RSA key")
	err = verifyKeyCertPair(t, k3, cert)
	Assert(t, err != nil, "Unexpected success on invalid key cert pair")
}
