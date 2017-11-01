// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"crypto"
	"crypto/x509"
	"fmt"
	"strings"
	"sync"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/log"
)

// KeyMgr is a module that manages symmetric and asymmetric keys. It includes
// facilities for generating, storing, using, importing and exporting keys.
// It has an interface similar to an HSM that allows creation and use of private
// keys without exposing them to the client.
// The crypto ops are performed by a pluggable backend, which can be a real HSM,
// a KMIP server or a simple wrapper around Go's crypto library
type KeyMgr struct {
	sync.Mutex
	be      Backend // the implementer of the KeyMgr interface
	objects map[string]Object
}

func getBundleCertificateID(bundleID string, index int) string {
	return fmt.Sprintf("-B-%s-%d", bundleID, index)
}

func validateID(id string) error {
	if id == "" {
		return fmt.Errorf("ID cannot be empty")
	}
	if strings.HasPrefix(id, "-") {
		return fmt.Errorf("ID cannot start with a dash")
	}
	return nil
}

func (km *KeyMgr) storeCertificateBundle(bundleObj *CertificateBundle) error {
	for i, cert := range bundleObj.Certificates {
		err := km.storeObject(NewCertificateObject(getBundleCertificateID(bundleObj.ID(), i), cert))
		if err != nil {
			return errors.Wrapf(err, "Error storing certificate bundle, id: %s, index: %d", bundleObj.ID(), i)
		}
	}
	return nil
}

func (km *KeyMgr) getCertificateBundle(id string) (Object, error) {
	var certs []*x509.Certificate
	for i := 0; true; i++ {
		certID := getBundleCertificateID(id, i)
		intCert, err := km.getObject(certID, ObjectTypeCertificate)
		if err != nil {
			log.Errorf("Error retrieving intermediate certificate %s: %v", certID, err)
			// in case of error we bail out, otherwise we may get stuck in an infinite loop
			// if backend is unavailable or in a bad state
			return nil, errors.Wrapf(err, "Error reading certificate bundle, id: %v, index: %v", id, i)
		}
		if intCert != nil {
			certs = append(certs, intCert.(*Certificate).Certificate)
		} else {
			// nil cert but no error means we have read all certs in the bundle, we can break the loop and return
			break
		}
	}
	if len(certs) > 0 {
		return NewCertificateBundleObject(id, certs), nil
	}
	// no certs found, bundle does not exist
	return nil, nil
}

func (km *KeyMgr) destroyCertificateBundle(id string) error {
	// first read the bundle so that we know what we should try to delete
	bundle, err := km.getCertificateBundle(id)
	if err != nil {
		return errors.Wrapf(err, "Error accessing certificate bundle, id: %v", id)
	}
	if bundle == nil {
		return fmt.Errorf("Certificate bundle %v not found", id)
	}
	for i := range bundle.(*CertificateBundle).Certificates {
		err = km.destroyObject(getBundleCertificateID(id, i), ObjectTypeCertificate)
		if err != nil {
			return errors.Wrapf(err, "Error destroying bundle, id: %v index: %d", id, i)
		}
	}
	return nil
}

func (km *KeyMgr) storeObject(obj Object) error {
	switch obj.Type() {
	case ObjectTypeCertificateBundle:
		return km.storeCertificateBundle(obj.(*CertificateBundle))
	default:
		id := obj.ID()
		if km.objects[id] != nil {
			return fmt.Errorf("Object with ID %v already present", id)
		}
		err := km.be.StoreObject(obj)
		if err != nil {
			return errors.Wrapf(err, "Error storing object in backend, ID: %s", id)
		}
		km.objects[id] = obj
		return nil
	}
}

func (km *KeyMgr) getObject(id string, Type ObjectType) (Object, error) {
	switch Type {
	case ObjectTypeCertificateBundle:
		return km.getCertificateBundle(id)
	default:
		var err error
		obj := km.objects[id]
		if obj == nil {
			// not found in cache, attempt to retrieve from backend
			obj, err = km.be.GetObject(id, Type)
			if err != nil {
				return nil, errors.Wrapf(err, "Error retrieving object from backend, ID:%s", id)
			}
			if obj != nil {
				km.objects[id] = obj
			}
		}
		return obj, err
	}
}

func (km *KeyMgr) destroyObject(id string, Type ObjectType) error {
	switch Type {
	case ObjectTypeCertificateBundle:
		return km.destroyCertificateBundle(id)
	default:
		err := km.be.DestroyObject(id, Type)
		if err != nil {
			log.Errorf("Error destroying object %v: %v", id, err)
		}
		// we remove from the map even if there was an error because the key pair
		// might have become unusable
		delete(km.objects, id)
		return err
	}
}

// StoreObject stores an object in KeyMgr.
// If backend is stateless, Object is not persistent.
func (km *KeyMgr) StoreObject(obj Object) error {
	if obj == nil {
		return fmt.Errorf("Object cannot be nil")
	}
	id := obj.ID()
	if err := validateID(id); err != nil {
		return errors.Wrapf(err, "ID validation error")
	}
	km.Lock()
	defer km.Unlock()
	return km.storeObject(obj)
}

// GetObject returns a pointer to the object with the supplied ID
// If the object is not found in cache, attempt to retrieve it from backend
func (km *KeyMgr) GetObject(id string, Type ObjectType) (Object, error) {
	km.Lock()
	defer km.Unlock()
	return km.getObject(id, Type)
}

// DestroyObject destroys the key pair with the given ID and cleans up the associated state
func (km *KeyMgr) DestroyObject(id string, Type ObjectType) error {
	km.Lock()
	defer km.Unlock()
	return km.destroyObject(id, Type)
}

// CreateKeyPair generates a (public key, private key) of the specified type with the supplied ID
func (km *KeyMgr) CreateKeyPair(id string, keytype KeyType) (*KeyPair, error) {
	if err := validateID(id); err != nil {
		return nil, errors.Wrapf(err, "ID validation error")
	}
	km.Lock()
	defer km.Unlock()
	if km.objects[id] != nil {
		return nil, fmt.Errorf("KeyPair with ID %v already exists", id)
	}
	kp, err := km.be.CreateKeyPair(id, keytype)
	if err != nil {
		return nil, errors.Wrapf(err, "Error generating KeyPair with ID %s", id)
	}
	km.objects[id] = kp
	return kp, nil
}

// DeriveKey derives a symmetric key using Diffie-Hellman key agreement.
// Requires a base private key and a peer public key.
// Peer will be able to derive the same key with peer private key and base public key
func (km *KeyMgr) DeriveKey(derivedKeyID string, derivedKeyType KeyType, baseKeyPairID string, peerPublicKey crypto.PublicKey) (*SymmetricKey, error) {
	if baseKeyPairID == "" {
		return nil, fmt.Errorf("Base key ID not specified")
	}
	if derivedKeyID == "" {
		return nil, fmt.Errorf("Derived key ID not specified")
	}
	if peerPublicKey == nil {
		return nil, fmt.Errorf("Peer public key not provided")
	}
	// Right now only ECDH (Elliptic Curve Diffie-Hellman) derivation of AES keys
	// is supported for all backends, so we can do all checks here
	peerKeyType := getPublicKeyType(peerPublicKey)
	if !isECDSAKey(peerKeyType) {
		return nil, fmt.Errorf("Unsupported peer key type: %v", peerKeyType)
	}
	if !isAESKey(derivedKeyType) {
		return nil, fmt.Errorf("Unsupported derived key type: %v", derivedKeyType)
	}
	km.Lock()
	defer km.Unlock()
	if km.objects[derivedKeyID] != nil {
		return nil, fmt.Errorf("Object with ID: %v already exists", derivedKeyID)
	}
	key, err := km.be.DeriveKey(derivedKeyID, derivedKeyType, baseKeyPairID, peerPublicKey)
	if err == nil {
		km.objects[derivedKeyID] = key
	}
	return key, err
}

// WrapKeyPair encrypts a KeyPair with an existing symmetric wrapping key.
// The wrapped key is returned as byte array and is not stored.
func (km *KeyMgr) WrapKeyPair(keyPairID, wrappingKeyID string) ([]byte, error) {
	if keyPairID == "" {
		return nil, fmt.Errorf("WrapKeyPair cannot be invoked with empty KeyPairID")
	}
	if wrappingKeyID == "" {
		return nil, fmt.Errorf("WrapKeyPair cannot be invoked with empty wrapping key ID")
	}
	km.Lock()
	defer km.Unlock()
	return km.be.WrapKeyPair(keyPairID, wrappingKeyID)
}

// UnwrapKeyPair takes a wrapped KeyPair and decrypts it with an unwrapping key.
func (km *KeyMgr) UnwrapKeyPair(targetKeyPairID string, wrappedKey []byte, publicKey crypto.PublicKey, unwrappingKeyID string) (*KeyPair, error) {
	if targetKeyPairID == "" {
		return nil, fmt.Errorf("UnwrapKeyPair cannot be invoked with empty target KeyPair ID")
	}
	if wrappedKey == nil {
		return nil, fmt.Errorf("UnwrapKeyPair cannot be invoked with nil wrapped key")
	}
	if publicKey == nil {
		return nil, fmt.Errorf("UnwrapKeyPair cannot be invoked with nil public key")
	}
	if unwrappingKeyID == "" {
		return nil, fmt.Errorf("UnwrapKeyPair cannot be invoked with empty unwrapping key ID")
	}
	km.Lock()
	defer km.Unlock()
	kp, err := km.be.UnwrapKeyPair(targetKeyPairID, wrappedKey, publicKey, unwrappingKeyID)
	if err != nil {
		return nil, errors.Wrapf(err, "Error unwrapping key pair, targetKeyPairID: %v, unwrappingKeyID: %v", targetKeyPairID, unwrappingKeyID)
	}
	km.objects[targetKeyPairID] = kp
	return kp, nil
}

// NewKeyMgr returns a new instance of KeyMgr using the supplied backend
func NewKeyMgr(backend Backend) (*KeyMgr, error) {
	if backend == nil {
		return nil, fmt.Errorf("Backend for KeyMgr cannot be nil")
	}
	return &KeyMgr{
		be:      backend,
		objects: make(map[string]Object),
	}, nil
}
