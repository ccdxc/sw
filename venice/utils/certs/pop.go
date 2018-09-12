// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package certs

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/rand"
	"crypto/rsa"
	"crypto/sha256"
	"crypto/x509"
	"encoding/asn1"
	"fmt"
	"math/big"

	"github.com/pkg/errors"
)

// This file provides utility functions to verify that a 3rd party has possession of
// the private key corresponding to an x509 Certificate.
//
// These are the verification steps (|| denotes concatenation, S signature):
// - verifier generates a challenge consisting of a nonce N and sends it to claimant
// - claimant generates a random string R, computes H = SHA(N || R), and sends back R and S(H)
// - verifier checks that H = SHA(N || R) and checks the signature S(C) using the public key in the certificate
//
// The purpose of the verifier nonce is to avoid replay attacks.
// The purpose of the claimant random string is to avoid chosen-plaintext attacks.
// We use random 256-bits strings for both.
//
// References:
// - ISO/IEC 9798-3
// - http://cacr.uwaterloo.ca/hac/about/chap10.pdf -- Section 10.3.3 (iii)
// - https://csrc.nist.gov/csrc/media/publications/fips/196/archive/1997-02-18/documents/fips196.pdf

const popNonceLen = sha256.Size

type ecdsaSignature struct {
	R, S *big.Int
}

// getHash generates the hash to be signed. It's the SHA of the concatenation of claimant and verifier nonces.
func getHash(N1, N2 []byte) ([]byte, error) {
	if len(N1) != popNonceLen {
		return nil, fmt.Errorf("Invalid nonce N1, size: %v", len(N1))
	}
	if len(N2) != popNonceLen {
		return nil, fmt.Errorf("Invalid nonce N2, size: %v", len(N2))
	}
	h := sha256.New()
	h.Write(N1)
	h.Write(N2)
	return h.Sum(nil), nil
}

func verifyECDSAChallenge(challenge, signature []byte, publicKey *ecdsa.PublicKey) error {
	var es ecdsaSignature
	rest, err := asn1.Unmarshal(signature, &es)
	if err != nil {
		return errors.Wrapf(err, "Error unmarshaling ECDSA signature")
	}
	if len(rest) > 0 {
		return fmt.Errorf("Malformed ECDSA signature: %d bytes left", len(rest))
	}
	// Verify already checks that R and S are positive and < N
	if !ecdsa.Verify(publicKey, challenge, es.R, es.S) {
		return fmt.Errorf("Invalid ECDSA signature")
	}
	return nil
}

func verifyRSAChallenge(challenge, signature []byte, publicKey *rsa.PublicKey) error {
	return rsa.VerifyPKCS1v15(publicKey, crypto.SHA256, challenge, signature)
}

// GeneratePoPNonce generates a nonce to be used either as challenge or as claimant nonce
func GeneratePoPNonce() ([]byte, error) {
	n := make([]byte, popNonceLen)
	_, err := rand.Read(n)
	if err != nil {
		return nil, err
	}
	return n, nil
}

// GeneratePoPChallengeResponse generates the response to be sent to the verifier.
// It contains the claimant nonce and the signature of Hash data
func GeneratePoPChallengeResponse(signer crypto.Signer, challenge []byte) ([]byte, []byte, error) {
	if signer == nil {
		return nil, nil, fmt.Errorf("No signer provided")
	}
	claimantRandom, err := GeneratePoPNonce()
	if err != nil {
		return nil, nil, errors.Wrapf(err, "Error generating claimant nonce")
	}
	hash, err := getHash(challenge, claimantRandom)
	if err != nil {
		return nil, nil, errors.Wrapf(err, "Error generating hash")
	}
	signature, err := signer.Sign(rand.Reader, hash, crypto.SHA256)
	if err != nil {
		return nil, nil, errors.Wrapf(err, "Error signing hash")
	}
	return claimantRandom, signature, nil
}

// VerifyPoPChallenge verifies that the response provided by the claimant proves
// possession of the private key corresponding to the public key in the certificate
func VerifyPoPChallenge(challenge, claimantRandom, signature []byte, cert *x509.Certificate) error {
	if len(signature) == 0 {
		return fmt.Errorf("Invalid signature, size: %v", len(signature))
	}
	if cert == nil {
		return fmt.Errorf("Certificate not provided")
	}
	hash, err := getHash(challenge, claimantRandom)
	if err != nil {
		return errors.Wrapf(err, "Error getting hash")
	}
	publicKey := cert.PublicKey
	switch publicKey.(type) {
	case *rsa.PublicKey:
		return verifyRSAChallenge(hash, signature, publicKey.(*rsa.PublicKey))
	case *ecdsa.PublicKey:
		return verifyECDSAChallenge(hash, signature, publicKey.(*ecdsa.PublicKey))
	default:
		return fmt.Errorf("Unknown key type: %T", publicKey)
	}
}
