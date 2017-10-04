// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rsa"
)

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

var ecdsaKeyTypeToCurve = map[KeyType]elliptic.Curve{
	ECDSA224: elliptic.P224(),
	ECDSA256: elliptic.P256(),
	ECDSA384: elliptic.P384(),
	ECDSA521: elliptic.P521(),
}

var ecdsaCurveToKeyType = map[elliptic.Curve]KeyType{
	elliptic.P224(): ECDSA224,
	elliptic.P256(): ECDSA256,
	elliptic.P384(): ECDSA384,
	elliptic.P521(): ECDSA521,
}

// return KeyMgr's KeyType from a crypto.PublicKey
func getKeyType(key crypto.PublicKey) KeyType {
	switch key.(type) {
	case *rsa.PublicKey:
		return rsaBitSizeToKeyType[key.(*rsa.PublicKey).N.BitLen()]
	case *ecdsa.PublicKey:
		return ecdsaCurveToKeyType[key.(*ecdsa.PublicKey).Curve]
	default:
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
