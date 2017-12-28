// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"sync"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/certs"
	kw "github.com/pensando/sw/venice/utils/crypto/keywrap"
)

// GoCryptoBackend is a KeyMgr backend based on Go crypto library
type GoCryptoBackend struct {
	sync.Mutex
	workDir string
}

func (be *GoCryptoBackend) getObjectPath(ID string, Type ObjectType) string {
	objectPath := path.Join(be.workDir, ID)
	switch Type {
	case ObjectTypeKeyPair:
		objectPath += ".key"
	case ObjectTypeSymmetricKey:
		objectPath += ".aes"
	case ObjectTypeCertificate:
		objectPath += ".crt"
	default:
		objectPath += ".bin"
	}
	return objectPath
}

func (be *GoCryptoBackend) newRsaKeyPair(keytype KeyType, bits int) (*KeyPair, error) {
	key, err := rsa.GenerateKey(rand.Reader, bits)
	if err != nil {
		return nil, err
	}
	return &KeyPair{
		KeyType: keytype,
		Signer:  key,
	}, nil
}

func (be *GoCryptoBackend) newEcdsaKeyPair(keytype KeyType, curve elliptic.Curve) (*KeyPair, error) {
	key, err := ecdsa.GenerateKey(curve, rand.Reader)
	if err != nil {
		return nil, err
	}
	return &KeyPair{
		KeyType: keytype,
		Signer:  key,
	}, nil
}

// CreateKeyPair generates a (public key, private key) of the specified type with the supplied ID
func (be *GoCryptoBackend) CreateKeyPair(id string, keyType KeyType) (*KeyPair, error) {
	var kp *KeyPair
	var err error

	// We don't need to lock/unlock here because the only operation that modifies state is the
	// StoreObject call below, which does its own lock/unlock, as it is part of the public interface
	switch {
	case isRSAKey(keyType):
		kp, err = be.newRsaKeyPair(keyType, rsaKeyTypeToBitSize[keyType])
	case isECDSAKey(keyType):
		kp, err = be.newEcdsaKeyPair(keyType, ecdsaKeyTypeToCurve[keyType])
	default:
		kp, err = nil, fmt.Errorf("Unsupported KeyType: %v", keyType)
	}
	if err != nil {
		return nil, errors.Wrapf(err, "Error creating key pair, ID: %s, type: %v", id, keyType)
	}
	kp.id = id
	kp.objType = ObjectTypeKeyPair
	err = be.StoreObject(kp)
	if err != nil {
		return nil, errors.Wrapf(err, "Error storing key pair, ID: %s, type: %v", id, keyType)
	}
	return kp, nil
}

func (be *GoCryptoBackend) readSymmetricKey(ID, fileName string) (Object, error) {
	key, err := ioutil.ReadFile(fileName)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading symmetric key, ID: %v", ID)
	}
	keyLenBits := uint(len(key) * 8)
	return NewSymmetricKeyObject(ID, bitSizeToSymmetricKeyType[keyLenBits], key), nil
}

func (be *GoCryptoBackend) getObject(ID string, Type ObjectType) (Object, error) {
	fileName := be.getObjectPath(ID, Type)
	_, err := os.Stat(fileName)
	if err != nil && os.IsNotExist(err) {
		// if object does not exist, it is not an error
		return nil, nil
	}
	switch Type {
	case ObjectTypeCertificate:
		cert, err := certs.ReadCertificate(fileName)
		if err != nil {
			return nil, errors.Wrapf(err, "Error reading certificate, ID: %v", ID)
		}
		return NewCertificateObject(ID, cert), nil
	case ObjectTypeKeyPair:
		key, err := certs.ReadPrivateKey(fileName)
		if err != nil {
			return nil, errors.Wrapf(err, "Error reading keypair, ID: %v", ID)
		}
		signer := getSignerFromPrivateKey(key)
		return NewKeyPairObject(ID, signer), nil
	case ObjectTypeSymmetricKey:
		return be.readSymmetricKey(ID, fileName)
	default:
		return nil, fmt.Errorf("Unsupported object type: %v", Type)
	}
}

// GetObject returns an object stored in KeyMgr.
func (be *GoCryptoBackend) GetObject(ID string, Type ObjectType) (Object, error) {
	if ID == "" {
		return nil, fmt.Errorf("Object ID not specified")
	}
	be.Lock()
	defer be.Unlock()
	return be.getObject(ID, Type)
}

func (be *GoCryptoBackend) writeSymmetricKey(obj Object, objectPath string) error {
	err := ioutil.WriteFile(objectPath, obj.(*SymmetricKey).Key.([]byte), 0600)
	if err != nil {
		return errors.Wrapf(err, "Error writing symmetric key, ID: %v", obj.ID())
	}
	return nil
}

func (be *GoCryptoBackend) storeObject(obj Object) error {
	id := obj.ID()
	objectPath := be.getObjectPath(id, obj.Type())
	if _, err := os.Stat(objectPath); !os.IsNotExist(err) {
		return errors.Wrapf(err, "Object with ID %s already exists", id)
	}
	switch obj.Type() {
	case ObjectTypeCertificate:
		err := certs.SaveCertificate(objectPath, obj.(*Certificate).Certificate)
		if err != nil {
			return errors.Wrapf(err, "Error writing certificate, ID: %v", id)
		}
	case ObjectTypeKeyPair:
		err := certs.SavePrivateKey(objectPath, obj.(*KeyPair).Signer)
		if err != nil {
			return errors.Wrapf(err, "Error writing private key, ID: %v", id)
		}
	case ObjectTypeSymmetricKey:
		return be.writeSymmetricKey(obj, objectPath)
	default:
		return fmt.Errorf("Unsupported object type: %v, ID: %v", obj.Type(), id)
	}
	return nil
}

// StoreObject stores an object in KeyMgr.
func (be *GoCryptoBackend) StoreObject(obj Object) error {
	if obj == nil {
		return fmt.Errorf("Object is nil")
	}
	if obj.ID() == "" {
		return fmt.Errorf("Object does not have a valid ID")
	}
	be.Lock()
	defer be.Unlock()
	return be.storeObject(obj)
}

// DestroyObject cleans up the state associated with the key pair with the given ID
func (be *GoCryptoBackend) DestroyObject(ID string, Type ObjectType) error {
	be.Lock()
	defer be.Unlock()
	return os.Remove(be.getObjectPath(ID, Type))
}

// DeriveKey derives a symmetric key using Diffie-Hellman key agreement.
// As of now, only ECDH-based derivation of AES keys is supported.
// The derived key is stored in the backend.
func (be *GoCryptoBackend) DeriveKey(derivedKeyID string, derivedKeyType KeyType, baseKeyPairID string, peerPublicKey crypto.PublicKey) (*SymmetricKey, error) {
	be.Lock()
	defer be.Unlock()
	baseKeyPair, err := be.getObject(baseKeyPairID, ObjectTypeKeyPair)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading base key pair, ID: %v", baseKeyPairID)
	}
	if baseKeyPair == nil {
		return nil, fmt.Errorf("Base key pair, ID: %v, does not exist", baseKeyPairID)
	}
	privKey := baseKeyPair.(*KeyPair).Signer.(*ecdsa.PrivateKey)
	pubKey := peerPublicKey.(*ecdsa.PublicKey)
	if pubKey == nil {
		return nil, fmt.Errorf("Public key mismatch, expecting ECDSA public key")
	}
	if privKey.PublicKey.Curve != pubKey.Curve {
		return nil, fmt.Errorf("Curve mismatch. Base: %s, Peer: %s", privKey.PublicKey.Curve.Params().Name, pubKey.Curve.Params().Name)
	}
	if !privKey.PublicKey.Curve.IsOnCurve(pubKey.X, pubKey.Y) {
		return nil, fmt.Errorf("Invalid public key. Point is not on the specified curve")
	}
	// multiply the point in the peer public key by our own private key (D)
	x, _ := pubKey.Curve.ScalarMult(pubKey.X, pubKey.Y, privKey.D.Bytes())
	// use the truncated least significative bytes of the X coordinate of the resulting point as derived key
	xBytes := x.Bytes()
	bytesLen := symmetricKeyTypeToBitSize[derivedKeyType] / 8
	key := make([]byte, bytesLen)
	copy(key, xBytes[len(xBytes)-int(bytesLen):])
	// form the symmetric key object, store it and return it
	keyObj := NewSymmetricKeyObject(derivedKeyID, derivedKeyType, key)
	err = be.storeObject(keyObj)
	if err != nil {
		return nil, errors.Wrapf(err, "Error storing derived key, ID: %v, type: %v", derivedKeyID, derivedKeyType)
	}
	return keyObj, nil
}

// WrapKeyPair encrypts a KeyPair with an existing symmetric key-encrypting-key (KEK).
// The wrapped key is returned as byte array and is not stored.
func (be *GoCryptoBackend) WrapKeyPair(keyPairID, kekID string) ([]byte, error) {
	be.Lock()
	defer be.Unlock()
	kek, err := be.getObject(kekID, ObjectTypeSymmetricKey)
	if err != nil {
		return nil, errors.Wrapf(err, "Error accessing wrapping key, ID: %v", kekID)
	}
	if kek == nil {
		return nil, fmt.Errorf("Wrapping key, ID: %v does not exist", kekID)
	}
	keyPair, err := be.getObject(keyPairID, ObjectTypeKeyPair)
	if err != nil {
		return nil, errors.Wrapf(err, "Error accessing key pair, ID: %v", keyPairID)
	}
	if keyPair == nil {
		return nil, fmt.Errorf("Key pair, ID: %v does not exist", keyPairID)
	}
	keyBytes, err := Pkcs8MarshalPrivateKey(keyPair.(*KeyPair).Signer)
	if err != nil {
		return nil, errors.Wrapf(err, "Error marshaling private key, ID: %v", keyPairID)
	}
	wrappedKey, err := kw.AesKeyWrapWithPad(keyBytes, kek.(*SymmetricKey).Key.([]byte))
	if err != nil {
		return nil, errors.Wrapf(err, "Error wrapping private key, ID: %v, with key: %v", keyPairID, kekID)
	}
	return wrappedKey, nil
}

// UnwrapKeyPair takes a wrapped KeyPair and decrypts it with a key-encrypting-key (KEK).
// The unwrapped KeyPair is stored in the backend.
func (be *GoCryptoBackend) UnwrapKeyPair(unwrappedKeyPairID string, wrappedKey []byte, publicKey crypto.PublicKey, kekID string) (*KeyPair, error) {
	be.Lock()
	defer be.Unlock()
	kekObj, err := be.getObject(kekID, ObjectTypeSymmetricKey)
	if err != nil {
		return nil, errors.Wrapf(err, "Error accessing unwrapping key, ID: %v", kekID)
	}
	if kekObj == nil {
		return nil, fmt.Errorf("Unwrapping key, ID: %v does not exist", kekID)
	}
	unwrappedKeyBytes, err := kw.AesKeyUnwrapWithPad(wrappedKey, kekObj.(*SymmetricKey).Key.([]byte))
	if err != nil {
		return nil, errors.Wrapf(err, "Error unwrapping private key ID: %v with key: %v", unwrappedKeyPairID, kekID)
	}
	key, err := x509.ParsePKCS8PrivateKey(unwrappedKeyBytes)
	if err != nil {
		return nil, errors.Wrapf(err, "Error parsing unwrapped key pair, ID: %v with key, ID: %v", unwrappedKeyPairID, kekID)
	}
	keyObj := NewKeyPairObject(unwrappedKeyPairID, key.(crypto.Signer))
	err = be.storeObject(keyObj)
	if err != nil {
		return nil, errors.Wrapf(err, "Error storing unwrapped keypair, ID: %v", unwrappedKeyPairID)
	}
	return keyObj, nil
}

// Close frees up the resources held by the backend
// It is client responsibility to call the function when done
func (be *GoCryptoBackend) Close() error {
	be.Lock()
	defer be.Unlock()
	return os.RemoveAll(be.workDir)
}

// NewGoCryptoBackend returns a new instance of GoCryptoBackend.
func NewGoCryptoBackend(dir string) (*GoCryptoBackend, error) {
	err := os.MkdirAll(dir, 0700)
	if err != nil {
		return nil, errors.Wrapf(err, "Unable to open working directory %s for writing", dir)
	}
	return &GoCryptoBackend{
		workDir: dir,
	}, nil
}
