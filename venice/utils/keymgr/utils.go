// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rsa"
	"crypto/x509"
	"encoding/asn1"
	"fmt"
	"math/big"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/log"
)

// from src/pkg/crypto/rsa/pkcs1v15.go
var hashPrefixes = map[crypto.Hash][]byte{
	crypto.MD5:       {0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05, 0x05, 0x00, 0x04, 0x10},
	crypto.SHA1:      {0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14},
	crypto.SHA224:    {0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05, 0x00, 0x04, 0x1c},
	crypto.SHA256:    {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20},
	crypto.SHA384:    {0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30},
	crypto.SHA512:    {0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40},
	crypto.MD5SHA1:   {}, // A special TLS case which doesn't use an ASN1 prefix.
	crypto.RIPEMD160: {0x30, 0x20, 0x30, 0x08, 0x06, 0x06, 0x28, 0xcf, 0x06, 0x03, 0x00, 0x31, 0x04, 0x14},
}

// from src/pkg/crypto/x509/x509.go
var (
	oidPublicKeyRSA   = asn1.ObjectIdentifier{1, 2, 840, 113549, 1, 1, 1}
	oidPublicKeyDSA   = asn1.ObjectIdentifier{1, 2, 840, 10040, 4, 1}
	oidPublicKeyECDSA = asn1.ObjectIdentifier{1, 2, 840, 10045, 2, 1}
)

// from src/pkg/crypto/x509/x509.go
var (
	oidNamedCurveP224 = asn1.ObjectIdentifier{1, 3, 132, 0, 33}
	oidNamedCurveP256 = asn1.ObjectIdentifier{1, 2, 840, 10045, 3, 1, 7}
	oidNamedCurveP384 = asn1.ObjectIdentifier{1, 3, 132, 0, 34}
	oidNamedCurveP521 = asn1.ObjectIdentifier{1, 3, 132, 0, 35}
)

// from src/pkg/crypto/x509/x509.go
var curveOIDs = map[string]asn1.ObjectIdentifier{
	"P-224": oidNamedCurveP224,
	"P-256": oidNamedCurveP256,
	"P-384": oidNamedCurveP384,
	"P-521": oidNamedCurveP521,
}

// from src/pkg/crypto/x509/x509.go
func namedCurveFromOID(oid asn1.ObjectIdentifier) elliptic.Curve {
	switch {
	case oid.Equal(oidNamedCurveP224):
		return elliptic.P224()
	case oid.Equal(oidNamedCurveP256):
		return elliptic.P256()
	case oid.Equal(oidNamedCurveP384):
		return elliptic.P384()
	case oid.Equal(oidNamedCurveP521):
		return elliptic.P521()
	}
	return nil
}

var rsaKeyTypeToBitSize = map[KeyType]int{
	RSA1024: 1024,
	RSA2048: 2048,
	RSA4096: 4096,
}

var rsaBitSizeToKeyType = map[int]KeyType{
	1024: RSA1024,
	2048: RSA2048,
	4096: RSA4096,
}

func isRSAKey(kt KeyType) bool {
	return kt >= RSA1024 && kt <= RSA4096
}

// ecdsaKeyTypeToCurve is a map from a KeyMgr ECDSA key type to the corresponding Go elliptic Curve struct
var ecdsaKeyTypeToCurve = map[KeyType]elliptic.Curve{
	ECDSA224: elliptic.P224(),
	ECDSA256: elliptic.P256(),
	ECDSA384: elliptic.P384(),
	ECDSA521: elliptic.P521(),
}

// ecdsaCurveToKeyType is a map from a Go elliptic Curve struct to the corresponding KeyMgr ECDSA key type
var ecdsaCurveToKeyType = map[elliptic.Curve]KeyType{
	elliptic.P224(): ECDSA224,
	elliptic.P256(): ECDSA256,
	elliptic.P384(): ECDSA384,
	elliptic.P521(): ECDSA521,
}

func isECDSAKey(kt KeyType) bool {
	return kt >= ECDSA224 && kt <= ECDSA521
}

var symmetricKeyTypeToBitSize = map[KeyType]uint{
	AES128: 128,
	AES192: 192,
	AES256: 256,
}

var bitSizeToSymmetricKeyType = map[uint]KeyType{
	128: AES128,
	192: AES192,
	256: AES256,
}

func isAESKey(kt KeyType) bool {
	return kt >= AES128 && kt <= AES256
}

type ecdsaSignature struct {
	R, S *big.Int
}

func parseEcdsaRawSignature(rawSignature []byte) (*ecdsaSignature, error) {
	sl := len(rawSignature)

	if sl%2 != 0 {
		return nil, fmt.Errorf("Raw signature length is not even")
	}

	r := big.NewInt(0)
	s := big.NewInt(0)

	r.SetBytes(rawSignature[0 : sl/2])
	s.SetBytes(rawSignature[sl/2:])

	return &ecdsaSignature{r, s}, nil
}

func rawToSignerSignature(rawSignature []byte, signer crypto.Signer) ([]byte, error) {
	switch keytype := signer.Public().(type) {
	case *rsa.PublicKey:
		return rawSignature, nil
	case *ecdsa.PublicKey:
		rs, err := parseEcdsaRawSignature(rawSignature)
		if err != nil {
			return nil, err
		}
		return asn1.Marshal(*rs)
	default:
		return nil, fmt.Errorf("unsupported key type %T", keytype)
	}
}

// GetPublicKeyType return KeyMgr's KeyType from a crypto.PublicKey
func GetPublicKeyType(key crypto.PublicKey) KeyType {
	switch keyType := key.(type) {
	case *rsa.PublicKey:
		return rsaBitSizeToKeyType[key.(*rsa.PublicKey).N.BitLen()]
	case *ecdsa.PublicKey:
		return ecdsaCurveToKeyType[key.(*ecdsa.PublicKey).Curve]
	default:
		log.Fatalf("unsupported key type %T", keyType)
		return Unknown
	}
}

// return a crypto.Signer interface given a RSA or ECDSA private key
func getSignerFromPrivateKey(key crypto.PrivateKey) crypto.Signer {
	switch key.(type) {
	case *rsa.PrivateKey:
		return key.(*rsa.PrivateKey)
	case *ecdsa.PrivateKey:
		return key.(*ecdsa.PrivateKey)
	default:
		return nil
	}
}

// VerifySignature verifies that the RSA/ECDSA signature of msg using the supplied public key
func VerifySignature(publicKey crypto.PublicKey, signature, msg []byte, opts crypto.Hash) (bool, error) {
	switch keyType := publicKey.(type) {
	case *rsa.PublicKey:
		key := publicKey.(*rsa.PublicKey)
		err := rsa.VerifyPKCS1v15(key, opts, msg, signature)
		if err != nil {
			return false, errors.Wrapf(err, "Error verifying RSA signature")
		}
		return true, nil
	case *ecdsa.PublicKey:
		key := publicKey.(*ecdsa.PublicKey)
		sigParams := new(struct{ R, S *big.Int })
		_, err := asn1.Unmarshal(signature, sigParams)
		if err != nil {
			return false, errors.Wrapf(err, "Error parsing ECDSA signature")
		}
		return ecdsa.Verify(key, msg, sigParams.R, sigParams.S), nil
	default:
		return false, fmt.Errorf("Unknown keypair type: %T", keyType)
	}
}

type pkcs8KeyInfo struct {
	Version    int
	Algo       []asn1.ObjectIdentifier
	PrivateKey []byte
}

// Pkcs8MarshalPrivateKey marshals an RSA/ECDSA private key using PKCS#8 format
// Unmarshaling can be done with Go's crypto.ParsePKCS8PrivateKey()
// See RFC 5208
func Pkcs8MarshalPrivateKey(privateKey crypto.PrivateKey) ([]byte, error) {
	var keyInfo pkcs8KeyInfo
	switch keyType := privateKey.(type) {
	case *ecdsa.PrivateKey:
		ecdsaKey := privateKey.(*ecdsa.PrivateKey)
		key, err := x509.MarshalECPrivateKey(ecdsaKey)
		if err != nil {
			return nil, err
		}
		oidNamedCurve := curveOIDs[ecdsaKey.PublicKey.Curve.Params().Name]
		if oidNamedCurve == nil {
			return nil, fmt.Errorf("Unknown elliptic curve %+v", ecdsaKey.PublicKey.Curve)
		}
		keyInfo.Version = 0
		keyInfo.Algo = make([]asn1.ObjectIdentifier, 2)
		keyInfo.Algo[0] = oidPublicKeyECDSA
		keyInfo.Algo[1] = oidNamedCurve
		keyInfo.PrivateKey = key
	case *rsa.PrivateKey:
		rsaKey := privateKey.(*rsa.PrivateKey)
		keyInfo.Version = 0
		keyInfo.Algo = make([]asn1.ObjectIdentifier, 1)
		keyInfo.Algo[0] = oidPublicKeyRSA
		keyInfo.PrivateKey = x509.MarshalPKCS1PrivateKey(rsaKey)
	default:
		return nil, fmt.Errorf("Unknown key type %T", keyType)
	}
	return asn1.Marshal(keyInfo)
}

// MarshalEcdsaPublicKey unmarshals an ECDSA public key to the coresponding Go crypto struct
func MarshalEcdsaPublicKey(key *ecdsa.PublicKey) []byte {
	if key == nil {
		return nil
	}
	return elliptic.Marshal(key.Curve, key.X, key.Y)
}

// UnmarshalEcdsaPublicKey unmarshals an ECDSA public key to the coresponding Go crypto struct
func UnmarshalEcdsaPublicKey(keyType KeyType, bytes []byte) *ecdsa.PublicKey {
	if bytes == nil {
		return nil
	}
	curve := ecdsaKeyTypeToCurve[keyType]
	x, y := elliptic.Unmarshal(curve, bytes)
	if x == nil || y == nil {
		return nil
	}
	return &ecdsa.PublicKey{
		Curve: curve,
		X:     x,
		Y:     y,
	}
}
