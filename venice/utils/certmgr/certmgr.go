// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certmgr

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/x509"
	"fmt"
	"sync"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/keymgr"
	"github.com/pensando/sw/venice/utils/log"
)

// CertMgr needs to be able to transport CA signing keys from one instance to another to achieve high-availability.
// Two instances agree on a symmetric AES keys using ECDH. The instance that has the "master" signing key then wraps
// the signing key with the wrapping key and sends it over. The receiving instance unwraps the key and bootstraps the CA.
// A similar process happens when the key has to be rotated.
const (
	caKeyAgreementKeyPrefix = "CaKeyAgreementKey"
	caKeyAgreementKeyType   = keymgr.ECDSA384 // TODO: change to 521 once we upgrade SoftHSM
	caKeyWrappingKeyPrefix  = "CaKeyWrappingKey"
	caKeyWrappingKeyType    = keymgr.AES256
)

// CertificateMgr is the service for storing, issuing and rotating certificates
type CertificateMgr struct {
	sync.Mutex
	ca     *CertificateAuthority // Internal certificate authority
	keyMgr *keymgr.KeyMgr
	ready  bool
}

func getKeyAgreementKeyID(peerID string) string {
	return fmt.Sprintf("%s-%s", caKeyAgreementKeyPrefix, peerID)
}

func getKeyWrappingKeyID(peerID string) string {
	return fmt.Sprintf("%s-%s", caKeyWrappingKeyPrefix, peerID)
}

// GetKeyAgreementKey creates and return a key pair that can be used to derive a key-encrypting-key using ECDH
// The key is persisted and each subsequent call of this function with the same peerID will return the same key
func (cm *CertificateMgr) GetKeyAgreementKey(peerID string) (crypto.PublicKey, error) {
	cm.Lock()
	defer cm.Unlock()
	kakID := getKeyAgreementKeyID(peerID)
	keyPair, err := cm.keyMgr.GetObject(kakID, keymgr.ObjectTypeKeyPair)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading existing key-agreement-key from CA backend")
	}
	if keyPair == nil {
		keyPair, err = cm.keyMgr.CreateKeyPair(kakID, caKeyAgreementKeyType)
		if err != nil {
			return nil, errors.Wrapf(err, "Error generating key-agreement-key")
		}
	}
	return keyPair.(*keymgr.KeyPair).Public().(*ecdsa.PublicKey), nil
}

// GetWrappedCaKey derives a symmetric key-agreement-key based on the provided peer's public key using ECDH,
// uses it to wrap the CA signing key and returns the result as []byte
// The wrapping key is deleted after wrapping
func (cm *CertificateMgr) GetWrappedCaKey(peerID string, peerKeyAgreementKey crypto.PublicKey) ([]byte, error) {
	cm.Lock()
	defer cm.Unlock()
	kakID := getKeyAgreementKeyID(peerID)
	kwkID := getKeyWrappingKeyID(peerID)
	_, err := cm.keyMgr.DeriveKey(kwkID, caKeyWrappingKeyType, kakID, peerKeyAgreementKey)
	defer cm.keyMgr.DestroyObject(kwkID, keymgr.ObjectTypeSymmetricKey)
	if err != nil {
		return nil, errors.Wrapf(err, "Error deriving CA key-wrapping-key")
	}
	wrappedKey, err := cm.keyMgr.WrapKeyPair(caKeyID, kwkID)
	if err != nil {
		return nil, errors.Wrapf(err, "Error wrapping CA key")
	}
	return wrappedKey, nil
}

// UnwrapCaKey derives a symmetric key-agreement-key based on the provided peer's public key using ECDH,
// uses it to unwrap the supplied CA signing key. The wrapping key is deleted after unwrapping
func (cm *CertificateMgr) UnwrapCaKey(wrappedKey []byte, caPublicKey crypto.PublicKey, peerID string, peerKeyAgreementKey crypto.PublicKey) error {
	cm.Lock()
	defer cm.Unlock()
	kakID := getKeyAgreementKeyID(peerID)
	kwkID := getKeyWrappingKeyID(peerID)
	if cm.ready {
		return fmt.Errorf("UnwrapCaKey cannot be called while CA is running")
	}
	_, err := cm.keyMgr.DeriveKey(kwkID, caKeyWrappingKeyType, kakID, peerKeyAgreementKey)
	defer cm.keyMgr.DestroyObject(kwkID, keymgr.ObjectTypeSymmetricKey)
	if err != nil {
		return errors.Wrapf(err, "Error deriving CA key-wrapping-key")
	}
	_, err = cm.keyMgr.UnwrapKeyPair(caKeyID, wrappedKey, caPublicKey, kwkID)
	if err != nil {
		return errors.Wrapf(err, "Error wrapping CA key")
	}
	return nil
}

// DestroyKeyAgreementKey destroys the current key-agreement-key
func (cm *CertificateMgr) DestroyKeyAgreementKey(peerID string) error {
	cm.Lock()
	defer cm.Unlock()
	kakID := getKeyAgreementKeyID(peerID)
	return cm.keyMgr.DestroyObject(kakID, keymgr.ObjectTypeKeyPair)
}

// MarshalKeyAgreementKey takes a key-agreement-key and returns the marshalled version
func (cm *CertificateMgr) MarshalKeyAgreementKey(key crypto.PublicKey) []byte {
	// we are not accessing any state, so no need to Lock/Unlock
	k, _ := key.(*ecdsa.PublicKey)
	if k == nil || keymgr.GetPublicKeyType(key) != caKeyAgreementKeyType {
		return nil
	}
	return keymgr.MarshalEcdsaPublicKey(k)
}

// UnmarshalKeyAgreementKey takes a marshalled key-agreement-key and returns the original version
func (cm *CertificateMgr) UnmarshalKeyAgreementKey(bytes []byte) crypto.PublicKey {
	// we are not accessing any state, so no need to Lock/Unlock
	key := keymgr.UnmarshalEcdsaPublicKey(caKeyAgreementKeyType, bytes)
	if key == nil || keymgr.GetPublicKeyType(key) != caKeyAgreementKeyType {
		return nil
	}
	return key
}

// StartCa instantiates and start the certification authority
// If no existing keys are found and bootstrap is true, the CA is bootstrapped with a
// generated private key and a self-signed certificate, otherwise an error is returned
func (cm *CertificateMgr) StartCa(bootstrap bool) error {
	cm.Lock()
	defer cm.Unlock()
	if cm.ca != nil && cm.ca.IsReady() {
		// nothing to do
		return nil
	}
	log.Infof("Starting CA")
	ca, err := NewCertificateAuthority(cm.keyMgr, bootstrap)
	if err != nil {
		return errors.Wrapf(err, "Error starting certificate manager CA")
	}
	cm.ca = ca
	cm.ready = true
	return nil
}

// LoadCaTrustChain loads the certificates from the CA to the root
func (cm *CertificateMgr) LoadCaTrustChain(certs []*x509.Certificate) error {
	cm.Lock()
	defer cm.Unlock()
	if cm.ready {
		return fmt.Errorf("LoadCaTrustChain cannot be called while CA is running")
	}
	if len(certs) < 1 {
		return fmt.Errorf("Invalid trust chain")
	}
	// First certificate should be the one associated with the CA signing key
	err := cm.keyMgr.StoreObject(keymgr.NewCertificateObject(caCertificateID, certs[0]))
	if err != nil {
		return errors.Wrapf(err, "Error storing CA certificate")
	}
	if len(certs) > 1 {
		err = cm.keyMgr.StoreObject(keymgr.NewCertificateBundleObject(intermediatesBundleName, certs[1:]))
		if err != nil {
			cm.keyMgr.DestroyObject(caCertificateID, keymgr.ObjectTypeCertificate)
			return errors.Wrapf(err, "Error storing CA trust chain")
		}
	}
	return nil
}

// LoadTrustRoots loads the additional trust roots in the CA
func (cm *CertificateMgr) LoadTrustRoots(certs []*x509.Certificate) error {
	cm.Lock()
	defer cm.Unlock()
	if cm.ready {
		return fmt.Errorf("LoadTrustRoots cannot be called while CA is running")
	}
	if len(certs) < 1 { // at the minimum we should have our own certificate if it is self-signed
		return fmt.Errorf("Invalid trust roots set")
	}
	err := cm.keyMgr.StoreObject(keymgr.NewCertificateBundleObject(trustRootsBundleName, certs))
	if err != nil {
		return errors.Wrapf(err, "Error storing trust roots")
	}
	return nil
}

// Ca returns a handle to the CA that is used by Certificate Mgr to issue certificates
func (cm *CertificateMgr) Ca() *CertificateAuthority {
	cm.Lock()
	defer cm.Unlock()
	return cm.ca
}

// IsReady returns true when instance is ready to serve requests
func (cm *CertificateMgr) IsReady() bool {
	cm.Lock()
	defer cm.Unlock()
	return cm.ready
}

// Close stops the CA and cleans up state
func (cm *CertificateMgr) Close() error {
	cm.Lock()
	defer cm.Unlock()
	log.Infof("Closing CertMgr instance")
	cm.ready = false
	cm.ca = nil
	err := cm.keyMgr.Close()
	return err
}

// NewCertificateMgr provides a new instance of Certificate Manager
func NewCertificateMgr(km *keymgr.KeyMgr) (*CertificateMgr, error) {
	if km == nil {
		return nil, errors.New("KeyMgr instance is required")
	}

	svc := &CertificateMgr{
		keyMgr: km,
		ready:  false,
	}

	return svc, nil
}

// NewDefaultCertificateMgr creates a CertMgr instance with a properly initialized KeyMgr instance.
// The created CertMgr instance is not ready yet because the CA hasn't been started.
func NewDefaultCertificateMgr() (*CertificateMgr, error) {
	// create a keymgr backend
	be, err := keymgr.NewDefaultBackend("certmgr")
	if err != nil {
		return nil, errors.Wrapf(err, "Error creating default CertificateMgr instance")
	}
	// create a KeyMgr instance
	km, err := keymgr.NewKeyMgr(be)
	if err != nil {
		return nil, errors.Wrapf(err, "Error creating default CertificateMgr instance")
	}
	// start CertMgr
	cm, err := NewCertificateMgr(km)
	if err != nil {
		return nil, errors.Wrapf(err, "Error creating default CertificateMgr instance")
	}
	return cm, nil
}

// NewGoCryptoCertificateMgr creates a CertMgr backed by a GoCrypto backend
// that stores keys in the provided directory
// The created CertMgr instance is not ready yet because the CA hasn't been started.
func NewGoCryptoCertificateMgr(dir string) (*CertificateMgr, error) {
	// create a keymgr backend
	be, err := keymgr.NewGoCryptoBackend(dir)
	if err != nil {
		return nil, errors.Wrapf(err, "Error creating default CertificateMgr instance, KeyMgr dir: %v", dir)
	}
	// create a KeyMgr instance
	km, err := keymgr.NewKeyMgr(be)
	if err != nil {
		return nil, errors.Wrapf(err, "Error creating default CertificateMgr instance")
	}
	// start CertMgr
	cm, err := NewCertificateMgr(km)
	if err != nil {
		return nil, errors.Wrapf(err, "Error creating default CertificateMgr instance")
	}
	log.Infof("Created CertificateMgr instance with Go Crypto backend, dir: %v", dir)
	return cm, nil
}
