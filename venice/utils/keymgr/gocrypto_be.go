// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/rsa"
	"fmt"
	"os"
	"path"
	"sync"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/certs"
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
	switch keyType {
	case RSA1024, RSA2048, RSA4096:
		kp, err = be.newRsaKeyPair(keyType, rsaKeyTypeToBitSize[keyType])
	case ECDSA224, ECDSA256, ECDSA384, ECDSA521:
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

// GetObject returns an object stored in KeyMgr.
func (be *GoCryptoBackend) GetObject(ID string, Type ObjectType) (Object, error) {
	if ID == "" {
		return nil, fmt.Errorf("Object ID not specified")
	}
	be.Lock()
	defer be.Unlock()
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
	default:
		return nil, fmt.Errorf("Unsupported object type: %v", Type)
	}
}

// StoreObject stores an object in KeyMgr.
func (be *GoCryptoBackend) StoreObject(obj Object) error {
	if obj == nil {
		return fmt.Errorf("Object is nil")
	}
	id := obj.ID()
	if id == "" {
		return fmt.Errorf("Object does not have a valid ID")
	}
	be.Lock()
	defer be.Unlock()
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
	default:
		return fmt.Errorf("Unsupported object type: %v, ID: %v", obj.Type(), id)
	}
	return nil
}

// DestroyObject cleans up the state associated with the key pair with the given ID
func (be *GoCryptoBackend) DestroyObject(ID string, Type ObjectType) error {
	be.Lock()
	defer be.Unlock()
	return os.Remove(be.getObjectPath(ID, Type))
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
