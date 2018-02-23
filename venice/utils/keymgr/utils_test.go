package keymgr

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"testing"

	"encoding/asn1"

	. "github.com/pensando/sw/venice/utils/testutils"
)

// Tests raw signature parsing
func TestECDSARawSignatureParsing(t *testing.T) {
	t.Parallel()
	var rawSig []byte
	priv, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating ECDSA Key")
	hashed := []byte("test")
	r, s, err := ecdsa.Sign(rand.Reader, priv, hashed)
	AssertOk(t, err, "Signing failed")

	rawSig = append(rawSig, r.Bytes()...)
	rawSig = append(rawSig, s.Bytes()...)
	ecdsaSig, err := parseEcdsaRawSignature(rawSig)
	if len(rawSig)%2 == 0 {
		AssertOk(t, err, "Failed to parse raw ECDSA Signature")
		AssertOk(t, err, "Failed to parse raw ECDSA Signature")
		Assert(t, ecdsaSig != nil, "Parsed ECDSA Signature was nil")

		// Odd length raw signature
		ecdsaSig, err = parseEcdsaRawSignature(rawSig[1:])
		Assert(t, err != nil, "parsing raw signature with odd length should fail")
	}

}

// Tests ECDSA pub key marshal and unmarshal
func TestECDSAPublicKeyMarshalUnmarshal(t *testing.T) {
	t.Parallel()
	priv, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating ECDSA Key")
	mPub := MarshalEcdsaPublicKey(&priv.PublicKey)
	Assert(t, len(mPub) == 65, "Length of the public key should be 65")

	pub := UnmarshalEcdsaPublicKey(ECDSA256, mPub)
	AssertEquals(t, priv.PublicKey, *pub, "unmarshaling should yield the original public key")
}

// Tests OID to curve mappings
func TestNamedCurveOID(t *testing.T) {
	t.Parallel()
	c := namedCurveFromOID(asn1.ObjectIdentifier{1, 3, 132, 0, 33})
	AssertEquals(t, elliptic.P224(), c, "Expected the curve to be P224")
	c = namedCurveFromOID(asn1.ObjectIdentifier{1, 2, 840, 10045, 3, 1, 7})
	AssertEquals(t, elliptic.P256(), c, "Expected the curve to be P256")
	c = namedCurveFromOID(asn1.ObjectIdentifier{1, 3, 132, 0, 34})
	AssertEquals(t, elliptic.P384(), c, "Expected the curve to be P384")
	c = namedCurveFromOID(asn1.ObjectIdentifier{1, 3, 132, 0, 35})
	AssertEquals(t, elliptic.P521(), c, "Expected the curve to be P521")
	c = namedCurveFromOID(asn1.ObjectIdentifier{})
	Assert(t, c == nil, "Expected the curve to be invalid")
}
