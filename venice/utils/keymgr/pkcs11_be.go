// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// +build linux,!arm64

package keymgr

// pkcs11_be.go implements a PKCS#11 backend for KeyMgr

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rsa"
	"crypto/x509"
	"encoding/asn1"
	"encoding/binary"
	"fmt"
	"io"
	"math/big"
	"sync"

	"github.com/miekg/pkcs11"
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	invalidSessionHandle = pkcs11.SessionHandle(0)
	invalidObjectHandle  = pkcs11.ObjectHandle(0)
)

// A process gains access to a PKCS#11 device by loading the .so library exactly once and
// logging in to specific "tokens" (which are basically isolated partitions).
// ctx is the context for a module. It contains the path of the module to load and a mutex
// to synchronize access from multiple clients.
type ctx struct {
	sync.Mutex
	modulePath string
	*pkcs11.Ctx
}

// this map keeps track of the modules that have been loaded
var modulesMutex sync.Mutex
var modules = make(map[string]*ctx)

// pkcs11Signer is the Pkcs11Backend implementation of the crypto.Signer interface.
// It allows KeyMgr to perform sign operations (for certificates, TLS sessions, etc.)
// without having direct access to the private key. The private key is referred by a
// handle and the signing operations happens inside the PKCS#11 device.
type pkcs11Signer struct {
	backend          *Pkcs11Backend
	privateKeyHandle pkcs11.ObjectHandle
	publicKey        crypto.PublicKey
}

type pkcs11SymmetricKey struct {
	backend   *Pkcs11Backend
	keyHandle pkcs11.ObjectHandle
}

// Pkcs11Backend is a KeyMgr backend that stores keys and performs crypto ops inside
// a device that provides PKCS#11 interface, like an HSM or a smartcard
//
// A PKCS#11 device has a number of (physical or virtual) "slots", which can be empty or
// contain a single "token". If a slot contains a token, the token can be initialized or not.
// A token that is initialized is ready to be used: it can hold keys and perform operations.
// In order to use a token, the PKCS#11 client logs into the device, initializes it if
// it is not yet initialized and then opens a "session" to it. Command issued during
// That session can only manipulate objects stored on that particular token.
// Tokens, like most other PKCS#11 objects, have a label that can be set by the user.
// There is no set limit on the number of tokens or slots that a device can have.
// Some implementations, like SoftHSM, simply create new slots with uninitialized tokens
// as user uses existing ones.
//
// When creating a new instance of this backend, we look for a token with the given
// label and if we find it, we open a session to it. If we don't find it, we look for an
// unitialized token, we try to initialize it and then we set the label.
//
// Node that context (ctx) corresponds to an entire module (.so library), not a particular
// token or session. So a single ctx object is shared across all backends that use that module.
type Pkcs11Backend struct {
	tokenLabel    string
	pin           string
	ctx           *ctx
	slot          uint
	sessionHandle pkcs11.SessionHandle
}

// open a session to the token associated with the backend
func (be *Pkcs11Backend) startSession(slot uint) error {
	if be.sessionHandle != invalidSessionHandle {
		return fmt.Errorf("Session already started, handle: %v", be.sessionHandle)
	}

	sessionHandle, err := be.ctx.OpenSession(slot, pkcs11.CKF_SERIAL_SESSION|pkcs11.CKF_RW_SESSION)
	if err != nil {
		return errors.Wrap(err, "Error opening session")
	}
	err = be.ctx.Login(sessionHandle, pkcs11.CKU_USER, be.pin)
	if err != nil && err != pkcs11.Error(pkcs11.CKR_USER_ALREADY_LOGGED_IN) {
		be.ctx.CloseSession(sessionHandle)
		return errors.Wrap(err, "Error logging in")
	}
	be.sessionHandle = sessionHandle
	return nil
}

// initialize a previously uninitialized token
func (be *Pkcs11Backend) initializeToken(slot uint) error {
	// initialize the token, set SO pin and label
	err := be.ctx.InitToken(slot, be.pin, be.tokenLabel)
	if err != nil {
		return errors.Wrapf(err, "Error initializing token, slotID: %v", slot)
	}
	// Open a temporary session that we will use only to login as security officer and set user PIN
	sessionHandle, err := be.ctx.OpenSession(slot, pkcs11.CKF_SERIAL_SESSION|pkcs11.CKF_RW_SESSION)
	if err != nil {
		return errors.Wrap(err, "Error opening session")
	}
	err = be.ctx.Login(sessionHandle, pkcs11.CKU_SO, be.pin)
	if err != nil {
		be.ctx.CloseSession(sessionHandle)
		return errors.Wrap(err, "Error logging in as security officer")
	}
	err = be.ctx.InitPIN(sessionHandle, be.pin)
	if err != nil {
		be.ctx.CloseSession(sessionHandle)
		return errors.Wrap(err, "Error initializing user PIN")
	}
	be.ctx.CloseSession(sessionHandle)
	be.ctx.Logout(sessionHandle)
	return nil
}

// perform the initialization steps required by the module before
// it can be used by the backend
func (be *Pkcs11Backend) initialize() error {
	// Find the token with the desired label or an uninitialized one
	// GetSlotList(true) obtains a list of slots in the system with a token present
	slots, err := be.ctx.GetSlotList(true)
	if err != nil {
		return errors.Wrap(err, "Error getting slot list")
	}
	availableToken := -1
	for _, slot := range slots {
		tokenInfo, err := be.ctx.GetTokenInfo(slot)
		if err != nil {
			log.Errorf("Error getting TokenInfo for slot %d, %v", slot, err)
			continue
		}
		if tokenInfo.Label == be.tokenLabel {
			// Found the token with the label we are looking for
			return be.startSession(slot)
		}
		if (availableToken < 0) && ((tokenInfo.Flags & pkcs11.CKF_TOKEN_INITIALIZED) == 0) {
			// Found the first slot with an unitialized token
			// We do not break out of the loop because we still need to see if we can find
			// an initialized token with the desired label. If not we will initialize this one.
			availableToken = int(slot)
		}
	}
	// Did not find an initialized token with the desired label
	// initialized the first available one, if any
	if availableToken >= 0 {
		slot := uint(availableToken)
		err = be.initializeToken(slot)
		if err != nil {
			return errors.Wrapf(err, "Error initializing token, slotID: %v, label: %v", availableToken, be.tokenLabel)
		}
		be.slot = slot
		return be.startSession(slot)
	}
	return fmt.Errorf("Error starting session. Could not find a slot with label %v or an uninitialized one", be.tokenLabel)
}

// read and return the specified attribute from the object with the given handle
func (be *Pkcs11Backend) getAttribute(objHandle pkcs11.ObjectHandle, attrType uint) ([]byte, error) {
	attrList := []*pkcs11.Attribute{pkcs11.NewAttribute(attrType, nil)}
	attrValues, err := be.ctx.GetAttributeValue(be.sessionHandle, objHandle, attrList)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading attribute: %v from object: %v", attrType, objHandle)
	}
	for _, a := range attrValues {
		if a.Type == attrType {
			return a.Value, nil
		}
	}
	return nil, fmt.Errorf("Error reading attribute: %v from object: %v. Attribute not found", attrType, objHandle)
}

// read a RSA public key from the device and return it as a Go crypto struct
func (be *Pkcs11Backend) readRsaPublicKey(handle pkcs11.ObjectHandle) (*rsa.PublicKey, error) {
	template := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PUBLIC_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_RSA),
		pkcs11.NewAttribute(pkcs11.CKA_PUBLIC_EXPONENT, nil),
		pkcs11.NewAttribute(pkcs11.CKA_MODULUS, nil),
	}
	attr, err := be.ctx.GetAttributeValue(be.sessionHandle, handle, template)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading RSA public key, handle: %+v, backend: %+v", handle, be)
	}
	mod := big.NewInt(0)
	exp := big.NewInt(0)
	for _, a := range attr {
		if a.Type == pkcs11.CKA_MODULUS {
			mod.SetBytes(a.Value)
		} else if a.Type == pkcs11.CKA_PUBLIC_EXPONENT {
			exp.SetBytes(a.Value)
		}
	}
	return &rsa.PublicKey{
		N: mod,
		E: int(exp.Int64()),
	}, nil
}

// generate an RSA public and private key pair inside the device and return the corresponding handles
func (be *Pkcs11Backend) newRsaPair(ID string, size int, extractable bool) (pkcs11.ObjectHandle, pkcs11.ObjectHandle, error) {
	publicKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PUBLIC_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_RSA),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_VERIFY, true),
		pkcs11.NewAttribute(pkcs11.CKA_PUBLIC_EXPONENT, []byte{1, 0, 1}),
		pkcs11.NewAttribute(pkcs11.CKA_MODULUS_BITS, size),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
	}
	privateKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PRIVATE_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_RSA),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_SIGN, true),
		pkcs11.NewAttribute(pkcs11.CKA_DERIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
		pkcs11.NewAttribute(pkcs11.CKA_SENSITIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_EXTRACTABLE, extractable),
	}
	publicKeyHandle, privateKeyHandle, err := be.ctx.GenerateKeyPair(be.sessionHandle,
		[]*pkcs11.Mechanism{pkcs11.NewMechanism(pkcs11.CKM_RSA_PKCS_KEY_PAIR_GEN, nil)},
		publicKeyTemplate,
		privateKeyTemplate)
	return publicKeyHandle, privateKeyHandle, err
}

func (be *Pkcs11Backend) storeRsaPublicKey(ID string, key *rsa.PublicKey) (pkcs11.ObjectHandle, error) {
	publicKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PUBLIC_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_RSA),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_VERIFY, true),
		pkcs11.NewAttribute(pkcs11.CKA_PUBLIC_EXPONENT, key.E),
		pkcs11.NewAttribute(pkcs11.CKA_MODULUS, key.N.Bytes()),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
	}
	publicKeyHandle, err := be.ctx.CreateObject(be.sessionHandle, publicKeyTemplate)
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error storing RSA public key")
	}
	return publicKeyHandle, nil
}

func (be *Pkcs11Backend) storeRsaPrivateKey(ID string, key *rsa.PrivateKey, extractable bool) (pkcs11.ObjectHandle, error) {
	if len(key.Primes) != 2 {
		return invalidObjectHandle, fmt.Errorf("RSA key not supported, primes: %d", len(key.Primes))
	}
	privateKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PRIVATE_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_RSA),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_SIGN, true),
		pkcs11.NewAttribute(pkcs11.CKA_DERIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
		pkcs11.NewAttribute(pkcs11.CKA_SENSITIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_EXTRACTABLE, extractable),
		pkcs11.NewAttribute(pkcs11.CKA_MODULUS, key.PublicKey.N.Bytes()),
		pkcs11.NewAttribute(pkcs11.CKA_PRIVATE_EXPONENT, key.D.Bytes()),
		pkcs11.NewAttribute(pkcs11.CKA_PRIME_1, key.Primes[0].Bytes()),
		pkcs11.NewAttribute(pkcs11.CKA_PRIME_2, key.Primes[1].Bytes()),
		pkcs11.NewAttribute(pkcs11.CKA_PUBLIC_EXPONENT, key.PublicKey.E),
	}
	privateKeyHandle, err := be.ctx.CreateObject(be.sessionHandle, privateKeyTemplate)
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error storing RSA private key")
	}
	return privateKeyHandle, err
}

// take a Go crypto RSA key and store it inside the device.
func (be *Pkcs11Backend) storeRsaPair(ID string, key *rsa.PrivateKey, extractable bool) (pkcs11.ObjectHandle, pkcs11.ObjectHandle, error) {
	publicKeyHandle, err := be.storeRsaPublicKey(ID, key.Public().(*rsa.PublicKey))
	if err != nil || publicKeyHandle == invalidObjectHandle {
		return invalidObjectHandle, invalidObjectHandle, nil
	}
	privateKeyHandle, err := be.storeRsaPrivateKey(ID, key, extractable)
	if err != nil || privateKeyHandle == invalidObjectHandle {
		be.ctx.DestroyObject(be.sessionHandle, publicKeyHandle)
		return invalidObjectHandle, invalidObjectHandle, nil
	}
	return publicKeyHandle, privateKeyHandle, err
}

// generate an ECDSA public and private key pair inside the device and return the corresponding handles
func (be *Pkcs11Backend) newEcdsaPair(ID string, curve elliptic.Curve, extractable bool) (pkcs11.ObjectHandle, pkcs11.ObjectHandle, error) {
	ecParam, err := asn1.Marshal(curveOIDs[curve.Params().Name])
	if err != nil {
		return invalidObjectHandle, invalidObjectHandle, errors.Wrapf(err, "Error marshaling curve parameters, curve: %s", curve.Params().Name)
	}
	publicKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PUBLIC_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_EC),
		pkcs11.NewAttribute(pkcs11.CKA_EC_PARAMS, ecParam),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_VERIFY, true),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
	}
	privateKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PRIVATE_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_SIGN, true),
		pkcs11.NewAttribute(pkcs11.CKA_DERIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
		pkcs11.NewAttribute(pkcs11.CKA_SENSITIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_EXTRACTABLE, extractable),
	}
	publicKeyHandle, privateKeyHandle, err := be.ctx.GenerateKeyPair(be.sessionHandle,
		[]*pkcs11.Mechanism{pkcs11.NewMechanism(pkcs11.CKM_EC_KEY_PAIR_GEN, nil)},
		publicKeyTemplate, privateKeyTemplate)
	return publicKeyHandle, privateKeyHandle, err
}

func (be *Pkcs11Backend) storeEcdsaPublicKey(ID string, key *ecdsa.PublicKey) (pkcs11.ObjectHandle, error) {
	curve := key.Curve
	ecParam, err := asn1.Marshal(curveOIDs[curve.Params().Name])
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error marshaling curve parameters, curve: %s", curve.Params().Name)
	}
	point := elliptic.Marshal(curve, key.X, key.Y)
	asn1Point, err := asn1.Marshal(point)
	if err != nil {
		return invalidObjectHandle, errors.Wrap(err, "Error marshaling point coordinates")
	}
	publicKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PUBLIC_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_EC),
		pkcs11.NewAttribute(pkcs11.CKA_EC_PARAMS, ecParam),
		pkcs11.NewAttribute(pkcs11.CKA_EC_POINT, asn1Point),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_VERIFY, true),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
	}
	publicKeyHandle, err := be.ctx.CreateObject(be.sessionHandle, publicKeyTemplate)
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error storing ECDSA public key")
	}
	return publicKeyHandle, nil
}

func (be *Pkcs11Backend) storeEcdsaPrivateKey(ID string, key *ecdsa.PrivateKey, extractable bool) (pkcs11.ObjectHandle, error) {
	curve := key.Public().(*ecdsa.PublicKey).Curve
	ecParam, err := asn1.Marshal(curveOIDs[curve.Params().Name])
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error marshaling curve parameters, curve: %s", curve.Params().Name)
	}
	privateKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PRIVATE_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_EC),
		pkcs11.NewAttribute(pkcs11.CKA_EC_PARAMS, ecParam),
		pkcs11.NewAttribute(pkcs11.CKA_VALUE, key.D.Bytes()),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_SIGN, true),
		pkcs11.NewAttribute(pkcs11.CKA_DERIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
		pkcs11.NewAttribute(pkcs11.CKA_SENSITIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_EXTRACTABLE, extractable),
	}
	privateKeyHandle, err := be.ctx.CreateObject(be.sessionHandle, privateKeyTemplate)
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error storing ECDSA private key")
	}
	return privateKeyHandle, nil
}

// take a Go crypto ECDSA key and store it inside the device.
func (be *Pkcs11Backend) storeEcdsaPair(ID string, key *ecdsa.PrivateKey, extractable bool) (pkcs11.ObjectHandle, pkcs11.ObjectHandle, error) {
	publicKeyHandle, err := be.storeEcdsaPublicKey(ID, key.Public().(*ecdsa.PublicKey))
	if err != nil || publicKeyHandle == invalidObjectHandle {
		return invalidObjectHandle, invalidObjectHandle, nil
	}
	privateKeyHandle, err := be.storeEcdsaPrivateKey(ID, key, extractable)
	if err != nil || privateKeyHandle == invalidObjectHandle {
		be.ctx.DestroyObject(be.sessionHandle, publicKeyHandle)
		return invalidObjectHandle, invalidObjectHandle, nil
	}
	return publicKeyHandle, privateKeyHandle, err
}

// read an ECDSA public key from the device and return it as a Go crypto struct
func (be *Pkcs11Backend) readEcdsaPublicKey(handle pkcs11.ObjectHandle) (*ecdsa.PublicKey, error) {
	template := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PUBLIC_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_EC),
		pkcs11.NewAttribute(pkcs11.CKA_EC_PARAMS, nil),
		pkcs11.NewAttribute(pkcs11.CKA_EC_POINT, nil),
	}
	attrs, err := be.ctx.GetAttributeValue(be.sessionHandle, handle, template)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading ECDSA public key, handle: %+v, backend: %+v", handle, be)
	}
	curveOID := asn1.ObjectIdentifier{}
	point := asn1.RawValue{}
	for _, a := range attrs {
		if a.Type == pkcs11.CKA_EC_POINT {
			rest, err := asn1.Unmarshal(a.Value, &point)
			if err != nil {
				return nil, errors.Wrapf(err, "Error unmarshaling point coordinates. Value: %v", a.Value)
			}
			if len(rest) > 0 {
				return nil, fmt.Errorf("Error unmarshaling point coordinates. Value: %v, Bytes left: %d", a.Value, len(rest))
			}
		} else if a.Type == pkcs11.CKA_EC_PARAMS {
			rest, err := asn1.Unmarshal(a.Value, &curveOID)
			if err != nil {
				return nil, errors.Wrapf(err, "Error unmarshaling curve parameters. Value: %v", a.Value)
			}
			if len(rest) > 0 {
				return nil, fmt.Errorf("Error unmarshaling curve parameters.Value: %v, Bytes left: %d", a.Value, len(rest))
			}
		}
	}
	curve := namedCurveFromOID(curveOID)
	publicKey := UnmarshalEcdsaPublicKey(ecdsaCurveToKeyType[curve], point.Bytes)
	if publicKey == nil {
		return nil, fmt.Errorf("Error unmarshaling point coordinates")
	}
	return publicKey, nil
}

func (be *Pkcs11Backend) storePublicKey(ID string, key crypto.PublicKey) (pkcs11.ObjectHandle, error) {
	keyType := GetPublicKeyType(key)
	switch {
	case isRSAKey(keyType):
		return be.storeRsaPublicKey(ID, key.(*rsa.PublicKey))
	case isECDSAKey(keyType):
		return be.storeEcdsaPublicKey(ID, key.(*ecdsa.PublicKey))
	default:
		return invalidObjectHandle, fmt.Errorf("Unsupported KeyType: %v", keyType)
	}
}

// takes a KeyPair containing a Go crypto key (RSA or ECDSA) and stores it inside the device.
func (be *Pkcs11Backend) storeKeyPair(kp *KeyPair, extractable bool) (pkcs11.ObjectHandle, pkcs11.ObjectHandle, error) {
	switch {
	case isRSAKey(kp.KeyType):
		return be.storeRsaPair(kp.ID(), kp.Signer.(*rsa.PrivateKey), extractable)
	case isECDSAKey(kp.KeyType):
		return be.storeEcdsaPair(kp.ID(), kp.Signer.(*ecdsa.PrivateKey), extractable)
	default:
		return invalidObjectHandle, invalidObjectHandle, fmt.Errorf("Unsupported KeyType: %v", kp.KeyType)
	}
}

// CreateKeyPair generates a (public key, private key) of the specified type with the supplied ID
func (be *Pkcs11Backend) CreateKeyPair(ID string, keyType KeyType) (*KeyPair, error) {
	be.ctx.Lock()
	defer be.ctx.Unlock()
	privateKeyHandle := invalidObjectHandle
	publicKeyHandle := invalidObjectHandle

	var err error
	var publicKey crypto.PublicKey
	extractable := true

	switch {
	case isRSAKey(keyType):
		publicKeyHandle, privateKeyHandle, err = be.newRsaPair(ID, rsaKeyTypeToBitSize[keyType], extractable)
		if err != nil {
			return nil, errors.Wrapf(err, "Error creating RSA key pair. Backend: %+v", be)
		}
		publicKey, err = be.readRsaPublicKey(publicKeyHandle)
		if err != nil {
			return nil, errors.Wrapf(err, "Error retrieving RSA public key. Backend: %+v", be)
		}
	case isECDSAKey(keyType):
		publicKeyHandle, privateKeyHandle, err = be.newEcdsaPair(ID, ecdsaKeyTypeToCurve[keyType], extractable)
		if err != nil {
			return nil, errors.Wrapf(err, "Error creating ECDSA key pair. Backend: %+v", be)
		}
		publicKey, err = be.readEcdsaPublicKey(publicKeyHandle)
		if err != nil {
			return nil, errors.Wrapf(err, "Error retrieving ECDSA public key. Backend: %+v", be)
		}
	default:
		return nil, fmt.Errorf("Unsupported KeyType: %v", keyType)
	}
	signer := &pkcs11Signer{
		backend:          be,
		privateKeyHandle: privateKeyHandle,
		publicKey:        publicKey,
	}
	kp := NewKeyPairObject(ID, signer)
	return kp, nil
}

// Public returns the public key associated with the pkcs11Signer instance
func (signer *pkcs11Signer) Public() crypto.PublicKey {
	return signer.publicKey
}

// Sign performs a signature operation using the associated PKCS #11 backend.
func (signer *pkcs11Signer) Sign(rand io.Reader, msg []byte, opts crypto.SignerOpts) ([]byte, error) {
	// Verify that the length of the hash is as expected
	hash := opts.HashFunc()
	hashLen := hash.Size()
	if len(msg) != hashLen {
		return nil, fmt.Errorf("Input size does not match hash function output size: %d vs %d", len(msg), hashLen)
	}
	// Add DigestInfo prefix
	var mechanism []*pkcs11.Mechanism
	switch kt := signer.Public().(type) {
	case *rsa.PublicKey:
		mechanism = []*pkcs11.Mechanism{pkcs11.NewMechanism(pkcs11.CKM_RSA_PKCS, nil)}
		prefix, ok := hashPrefixes[hash]
		if !ok {
			return nil, fmt.Errorf("Unknown hash function: %d", hash)
		}
		msg = append(prefix, msg...)
	case *ecdsa.PublicKey:
		mechanism = []*pkcs11.Mechanism{pkcs11.NewMechanism(pkcs11.CKM_ECDSA, nil)}
	default:
		return nil, fmt.Errorf("Unknown key type %T", kt)
	}
	be := signer.backend
	if be.sessionHandle == invalidSessionHandle {
		return nil, errors.New("Invalid session handle")
	}
	// Perform the sign operation
	be.ctx.Lock()
	defer be.ctx.Unlock()
	err := be.ctx.SignInit(be.sessionHandle, mechanism, signer.privateKeyHandle)
	if err != nil {
		return nil, errors.Wrapf(err, "Error invoking SignInit, signer: %+v", signer)
	}
	signature, err := be.ctx.Sign(be.sessionHandle, msg)
	if err != nil {
		return nil, errors.Wrapf(err, "Error signing message, signer:%+v", signer)
	}
	return rawToSignerSignature(signature, signer)
}

// Find a single object based on a set of attributes. It fails if multiple objects are found.
func (be *Pkcs11Backend) findObject(ID string, Type uint) (pkcs11.ObjectHandle, error) {
	template := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, Type),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
	}
	err := be.ctx.FindObjectsInit(be.sessionHandle, template)
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error initializing find function for object, template: %+v", template)
	}
	handles, more, err := be.ctx.FindObjects(be.sessionHandle, 1)
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error getting object, template: %+v", template)
	}
	if more {
		return invalidObjectHandle, fmt.Errorf("Found more than one object with template: %+v", template)
	}
	err = be.ctx.FindObjectsFinal(be.sessionHandle)
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error invoking FindObjectsFinal")
	}
	if len(handles) > 0 {
		return handles[0], nil
	}
	// object not found is not an error
	return invalidObjectHandle, nil
}

// store a certificate in the device
func (be *Pkcs11Backend) storeCertificate(ID string, cert *x509.Certificate) (pkcs11.ObjectHandle, error) {
	template := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_CERTIFICATE),
		pkcs11.NewAttribute(pkcs11.CKA_CERTIFICATE_TYPE, pkcs11.CKC_X_509),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_SUBJECT, ID),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
		pkcs11.NewAttribute(pkcs11.CKA_VALUE, cert.Raw),
	}
	return be.ctx.CreateObject(be.sessionHandle, template)
}

// read a certificate from the device
func (be *Pkcs11Backend) readCertificate(ID string) (*x509.Certificate, error) {
	objHandle, err := be.findObject(ID, pkcs11.CKO_CERTIFICATE)
	if err != nil {
		return nil, errors.Wrapf(err, "Error getting certificate handle, ID: %v", ID)
	}
	if objHandle == invalidObjectHandle {
		// not found, not an error
		return nil, nil
	}
	cert, err := be.getAttribute(objHandle, pkcs11.CKA_VALUE)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading certificate, ID: %v", ID)
	}
	return x509.ParseCertificate(cert)
}

// find the private and public key handles for the supplied ID
// return invalidObjectHandle (but no error) if not found
func (be *Pkcs11Backend) getKeyPairHandles(ID string) (pkcs11.ObjectHandle, pkcs11.ObjectHandle, error) {
	privateKeyHandle, err := be.findObject(ID, pkcs11.CKO_PRIVATE_KEY)
	if err != nil {
		return invalidObjectHandle, invalidObjectHandle, errors.Wrapf(err, "Error retrieving private key handle, ID:", ID)
	}
	publicKeyHandle, err := be.findObject(ID, pkcs11.CKO_PUBLIC_KEY)
	if err != nil {
		return invalidObjectHandle, invalidObjectHandle, errors.Wrapf(err, "Error retrieving public key handle, ID:", ID)
	}
	return publicKeyHandle, privateKeyHandle, err
}

// read a key pair (RSA or ECDSA) from the device and return it as a Go crypto.Signer
func (be *Pkcs11Backend) readKeyPair(ID string) (crypto.Signer, error) {
	publicKeyHandle, privateKeyHandle, err := be.getKeyPairHandles(ID)
	if err != nil {
		return nil, errors.Wrapf(err, "Error getting key handles, ID: %v", ID)
	}
	if (publicKeyHandle == invalidObjectHandle) && (privateKeyHandle == invalidObjectHandle) {
		// object not found, not an error
		return nil, nil
	}
	keyTypeBytes, err := be.getAttribute(publicKeyHandle, pkcs11.CKA_KEY_TYPE)
	if err != nil {
		return nil, errors.Wrapf(err, "Error getting public key attribute, handle: %v, ID: %v", publicKeyHandle, ID)
	}
	keyType, _ := binary.Uvarint(keyTypeBytes)
	var publicKey crypto.PublicKey
	switch keyType {
	case uint64(pkcs11.CKK_RSA):
		publicKey, err = be.readRsaPublicKey(publicKeyHandle)
	case uint64(pkcs11.CKK_EC):
		publicKey, err = be.readEcdsaPublicKey(publicKeyHandle)
	default:
		return nil, errors.Wrapf(err, "Unsupported key type, ID: CKA_KEY_TYPE: %v", ID, keyType)
	}
	if err != nil {
		return nil, errors.Wrapf(err, "Error retrieving key pair, ID: %v, key type: %v, Backend: %+v", ID, be)
	}
	signer := &pkcs11Signer{
		backend:          be,
		privateKeyHandle: privateKeyHandle,
		publicKey:        publicKey,
	}
	return signer, nil
}

// GetObject returns an object stored in KeyMgr.
func (be *Pkcs11Backend) GetObject(ID string, objType ObjectType) (Object, error) {
	be.ctx.Lock()
	defer be.ctx.Unlock()
	switch objType {
	case ObjectTypeCertificate:
		cert, err := be.readCertificate(ID)
		if err != nil {
			return nil, errors.Wrapf(err, "Error reading certificate, ID: %v", ID)
		}
		if cert != nil {
			return NewCertificateObject(ID, cert), nil
		}
	case ObjectTypeKeyPair:
		signer, err := be.readKeyPair(ID)
		if err != nil {
			return nil, errors.Wrapf(err, "Error reading KeyPair, ID: %v", ID)
		}
		if signer != nil {
			return NewKeyPairObject(ID, signer), nil
		}
	default:
		return nil, fmt.Errorf("Unsupported object type: %v", objType)
	}
	return nil, nil // object not found
}

// StoreObject stores an object in KeyMgr.
func (be *Pkcs11Backend) StoreObject(obj Object) error {
	be.ctx.Lock()
	defer be.ctx.Unlock()
	var err error
	switch obj.Type() {
	case ObjectTypeKeyPair:
		_, _, err = be.storeKeyPair(obj.(*KeyPair), false)
	case ObjectTypeCertificate:
		_, err = be.storeCertificate(obj.ID(), obj.(*Certificate).Certificate)
	default:
		err = fmt.Errorf("Unsupported object type: %v, ID: %v", obj.Type(), obj.ID())
	}
	return err
}

// destroy a public, private key pair stored in the device
func (be *Pkcs11Backend) destroyKeyPair(ID string) error {
	publicKeyHandle, privateKeyHandle, err := be.getKeyPairHandles(ID)
	if err != nil {
		return errors.Wrapf(err, "Error getting key pair, ID: %v", ID)
	}
	if (privateKeyHandle == invalidObjectHandle) && (publicKeyHandle == invalidObjectHandle) {
		// object does not exist
		return fmt.Errorf("Error destroying key pair, ID: %v. Object not found", ID)
	}
	for _, handle := range []pkcs11.ObjectHandle{privateKeyHandle, publicKeyHandle} {
		err = be.ctx.DestroyObject(be.sessionHandle, handle)
		if err != nil {
			return errors.Wrapf(err, "Error destroying private key, handle: %v, ID: %v", handle, ID)
		}
	}
	return nil
}

// destroy a symmetric key stored in the device
func (be *Pkcs11Backend) destroySymmetricKey(ID string) error {
	objHandle, err := be.findObject(ID, pkcs11.CKO_SECRET_KEY)
	if err != nil {
		return errors.Wrapf(err, "Error destroying key, ID: %v", ID)
	}
	if objHandle == invalidObjectHandle {
		return fmt.Errorf("Error destroying key, ID: %v. Object not found", ID)
	}
	return be.ctx.DestroyObject(be.sessionHandle, objHandle)
}

// destroy a certificate stored in the device
func (be *Pkcs11Backend) destroyCertificate(ID string) error {
	objHandle, err := be.findObject(ID, pkcs11.CKO_CERTIFICATE)
	if err != nil {
		return errors.Wrapf(err, "Error destroying certificate, ID: %v", ID)
	}
	if objHandle == invalidObjectHandle {
		return fmt.Errorf("Error destroying certificate, ID: %v. Object not found", ID)
	}
	return be.ctx.DestroyObject(be.sessionHandle, objHandle)
}

// DestroyObject cleans up the state associated with the key pair with the given ID
func (be *Pkcs11Backend) DestroyObject(ID string, objType ObjectType) error {
	be.ctx.Lock()
	defer be.ctx.Unlock()
	switch objType {
	case ObjectTypeKeyPair:
		return be.destroyKeyPair(ID)
	case ObjectTypeSymmetricKey:
		return be.destroySymmetricKey(ID)
	case ObjectTypeCertificate:
		return be.destroyCertificate(ID)
	default:
		return fmt.Errorf("Unsupported object type: %v, ID: %v", objType, ID)
	}
}

func (be *Pkcs11Backend) ecdhDeriveKey(derivedKeyID string, derivedKeyLen uint, privateKeyHandle pkcs11.ObjectHandle, peerPublicKey *ecdsa.PublicKey) (pkcs11.ObjectHandle, error) {
	point := elliptic.Marshal(peerPublicKey.Curve, peerPublicKey.X, peerPublicKey.Y)
	// According to PKCS#11, tokens that support CKM_ECDH1_DERIVE mechanism MUST accept
	// raw encoding (Sec. A.5.2 of ANSI X9.62) of public key for CKM_ECDH1_DERIVE mechanism,
	// and may optionally accept DER encoding.
	// Older versions of SoftHSM, however, accept DER-encoding but not raw encoding, so we use DER
	// https://github.com/opendnssec/SoftHSMv2/pull/288
	asn1Point, _ := asn1.Marshal(point)
	params, free := ecdh1DeriveParamBytes(asn1Point)
	defer free()
	mechanism := []*pkcs11.Mechanism{pkcs11.NewMechanism(pkcs11.CKM_ECDH1_DERIVE, params)}
	keyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_SECRET_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_AES),
		pkcs11.NewAttribute(pkcs11.CKA_VALUE_LEN, derivedKeyLen/8),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, derivedKeyID),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
	}
	key, err := be.ctx.DeriveKey(be.sessionHandle, mechanism, privateKeyHandle, keyTemplate)
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error deriving AES key using ECDH, base key handle: %v, derived key id: %v", privateKeyHandle, derivedKeyLen)
	}
	return key, nil
}

// DeriveKey derives a symmetric key using Diffie-Hellman key agreement.
// Right now only ECDH-based derivation of an AES key is supported
// The derived key is stored in the backend.
func (be *Pkcs11Backend) DeriveKey(derivedKeyID string, derivedKeyType KeyType, baseKeyPairID string, peerPublicKey crypto.PublicKey) (*SymmetricKey, error) {
	be.ctx.Lock()
	defer be.ctx.Unlock()
	derivedKeyLen := symmetricKeyTypeToBitSize[derivedKeyType]
	baseKeySigner, err := be.readKeyPair(baseKeyPairID)
	if err != nil {
		return nil, errors.Wrapf(err, "Error reading key pair, ID: %v", baseKeyPairID)
	}
	if baseKeySigner == nil {
		return nil, fmt.Errorf("Key pair does not exist, ID: %v", baseKeyPairID)
	}
	baseKeyPair := baseKeySigner.(*pkcs11Signer)
	baseKeyType := GetPublicKeyType(baseKeyPair.publicKey)
	if !isECDSAKey(baseKeyType) {
		return nil, fmt.Errorf("Unsupported base key type: %v", baseKeyType)
	}
	handle, err := be.ecdhDeriveKey(derivedKeyID, derivedKeyLen, baseKeyPair.privateKeyHandle, peerPublicKey.(*ecdsa.PublicKey))
	if err != nil || handle == invalidObjectHandle {
		return nil, err
	}
	key := &pkcs11SymmetricKey{
		backend:   be,
		keyHandle: handle,
	}
	return NewSymmetricKeyObject(derivedKeyID, derivedKeyType, key), nil
}

// WrapKeyPair encrypts a target key stored in the device with a supplied key-encrypting-key (KEK)
// The target key can be any, the kek must be AES.
// The wrapped key is returned as []byte and is not stored in the device
func (be *Pkcs11Backend) WrapKeyPair(keyPairID, kekID string) ([]byte, error) {
	be.ctx.Lock()
	defer be.ctx.Unlock()
	targetKeyHandle, err := be.findObject(keyPairID, pkcs11.CKO_PRIVATE_KEY)
	if err != nil {
		return nil, errors.Wrapf(err, "Error getting key pair handle, ID: %v", keyPairID)
	}
	if targetKeyHandle == invalidObjectHandle {
		return nil, fmt.Errorf("Key pair, ID: %v does not exist", keyPairID)
	}
	kekHandle, err := be.findObject(kekID, pkcs11.CKO_SECRET_KEY)
	if err != nil {
		return nil, errors.Wrapf(err, "Error getting wrapping key handle, ID: %v", kekID)
	}
	if kekHandle == invalidObjectHandle {
		return nil, fmt.Errorf("Wrapping key, ID: %v does not exist", kekID)
	}
	mechanism := []*pkcs11.Mechanism{pkcs11.NewMechanism(pkcs11.CKM_AES_KEY_WRAP_PAD, nil)}
	// WrapKey call will fail if the key is not extractable
	return be.ctx.WrapKey(be.sessionHandle, mechanism, kekHandle, targetKeyHandle)
}

// UnwrapKeyPair takes an encrypted key and decrypts it with an AES wrapping key stored in the device
func (be *Pkcs11Backend) UnwrapKeyPair(targetKeyID string, wrappedKey []byte, publicKey crypto.PublicKey, kekID string) (*KeyPair, error) {
	be.ctx.Lock()
	defer be.ctx.Unlock()
	extractable := false
	// retrieve unwrapping key handle
	kekHandle, err := be.findObject(kekID, pkcs11.CKO_SECRET_KEY)
	if err != nil {
		return nil, errors.Wrapf(err, "Error getting unwrapping key handle, ID: %v", kekID)
	}
	if kekHandle == invalidObjectHandle {
		return nil, fmt.Errorf("Error getting unwrapping key handle, ID: %v", kekID)
	}
	// store public key first
	publicKeyHandle, err := be.storePublicKey(targetKeyID, publicKey)
	if err != nil {
		return nil, errors.Wrapf(err, "Error storing public key with ID: %v", targetKeyID)
	}
	// form template for unwrapped private key
	keyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PRIVATE_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, getKeyTypeAttributeValue(GetPublicKeyType(publicKey))),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_SIGN, true),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, targetKeyID),
		pkcs11.NewAttribute(pkcs11.CKA_SENSITIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_EXTRACTABLE, extractable),
	}
	mechanism := []*pkcs11.Mechanism{pkcs11.NewMechanism(pkcs11.CKM_AES_KEY_WRAP_PAD, nil)}
	privateKeyHandle, err := be.ctx.UnwrapKey(be.sessionHandle, mechanism, kekHandle, wrappedKey, keyTemplate)
	if err != nil || publicKeyHandle == invalidObjectHandle {
		be.ctx.DestroyObject(be.sessionHandle, publicKeyHandle)
		return nil, errors.Wrapf(err, "Error storing public key with ID: %v", targetKeyID)
	}
	signer := &pkcs11Signer{
		backend:          be,
		privateKeyHandle: privateKeyHandle,
		publicKey:        publicKey,
	}
	return NewKeyPairObject(targetKeyID, signer), nil
}

// GetInfo returns library and device info
func (be *Pkcs11Backend) GetInfo() (pkcs11.Info, error) {
	return be.ctx.GetInfo()
}

// Close terminates the session and re-initialize the token
// Re-initializing the token wipes out all the objects
func (be *Pkcs11Backend) Close() error {
	be.ctx.CloseSession(be.sessionHandle)
	err := be.initializeToken(be.slot)
	if err != nil {
		return errors.Wrapf(err, "Error closing PKCS#11 backend: %+v", be)
	}
	return nil
}

// NewPkcs11Backend creates a new instance of the backend.
// It requires the path to the PKCS#11 .so library and a PIN.
// During initialization it checks all slots with an available token and chooses the first
// one that has a label matching tokenLabel and tries to login with the given PIN.
// If not found, it tries to initialize an uninitialized token and set label and PIN.
func NewPkcs11Backend(modulePath, tokenLabel, pin string) (*Pkcs11Backend, error) {
	// empty token label and pin can be ok, depending on the PKCS#11 device
	if modulePath == "" {
		return nil, fmt.Errorf("modulePath cannot be empty")
	}
	// load the module if needed
	modulesMutex.Lock()
	defer modulesMutex.Unlock()
	module := modules[modulePath]
	if module == nil {
		// load PKCS#11 Module
		libModule := pkcs11.New(modulePath)
		if libModule == nil {
			return nil, fmt.Errorf("Error loading module %s", modulePath)
		}
		module = &ctx{
			modulePath: modulePath,
			Ctx:        libModule,
		}
		err := module.Initialize()
		if err != nil {
			return nil, errors.Wrapf(err, "Error initializing module %s", modulePath)
		}
		modules[modulePath] = module
	}
	// create and initialize the backend instance
	newInstance := &Pkcs11Backend{
		tokenLabel:    tokenLabel,
		pin:           pin,
		ctx:           module,
		sessionHandle: invalidSessionHandle,
	}
	module.Lock()
	defer module.Unlock()
	err := newInstance.initialize()
	if err != nil {
		newInstance.ctx.Destroy()
		return nil, errors.Wrapf(err, "Error initializing module")
	}
	return newInstance, nil
}
