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

// A PKCS#11 device has a number of independent "tokens", which can be accessed independently.
// A process gains access to a PKCS#11 device by loading the .so library exactly one and
// logging in to tokens. Once logged into a token, user can create multiple sessions.

// In this backend implementation, we ask client to specify a module path, a token label and
// a PIN. If the module is not loaded yet, we load it. Then we look for a token with the given
// label and if we find it, we open a session to it. If we don't find it, we look for an
// unitialized token, we try to initialize it and then we set the label.
// So one Pkcs11Backend corresponds to a session to a token that can be shared by multiple
// backends. The context for the same module is shared among all backends that use that module.
// We assume that modules support serial access only, so the context for a module includes
// a handle and a mutex.

// The context for a module
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

// Pkcs11Backend is a KeyMgr backend that stores keys and performs crypto ops inside
// a device that provides PKCS#11 interface, like an HSM or a smartcard
type Pkcs11Backend struct {
	tokenLabel    string
	pin           string
	ctx           *ctx
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

// take a Go crypto RSA key and store it inside the device.
// TODO: change this function to accept only wrapped keys
func (be *Pkcs11Backend) storeRsaPair(ID string, key *rsa.PrivateKey, extractable bool) (pkcs11.ObjectHandle, pkcs11.ObjectHandle, error) {
	// public key
	publicKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PUBLIC_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_RSA),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_VERIFY, true),
		pkcs11.NewAttribute(pkcs11.CKA_PUBLIC_EXPONENT, key.PublicKey.E),
		pkcs11.NewAttribute(pkcs11.CKA_MODULUS, key.PublicKey.N.Bytes()),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
	}
	publicKeyHandle, err := be.ctx.CreateObject(be.sessionHandle, publicKeyTemplate)
	if err != nil {
		return invalidObjectHandle, invalidObjectHandle, errors.Wrapf(err, "Error storing public key")
	}

	// private key
	if len(key.Primes) != 2 {
		return invalidObjectHandle, invalidObjectHandle, fmt.Errorf("RSA key not supported, primes: %d", len(key.Primes))
	}
	privateKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PRIVATE_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_RSA),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_SIGN, true),
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
		be.ctx.DestroyObject(be.sessionHandle, publicKeyHandle)
		return invalidObjectHandle, invalidObjectHandle, errors.Wrapf(err, "Error storing private key")
	}

	return publicKeyHandle, privateKeyHandle, err
}

// generate an ECDSA public and private key pair inside the device and return the corresponding handles
func (be *Pkcs11Backend) newEcdsaPair(ID string, curve elliptic.Curve, extractable bool) (pkcs11.ObjectHandle, pkcs11.ObjectHandle, error) {
	ecParam, err := asn1.Marshal(curveOIDs[curve.Params().Name])
	if err != nil {
		err = errors.Wrapf(err, "Error marshaling curve OID")
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
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
		pkcs11.NewAttribute(pkcs11.CKA_SENSITIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_EXTRACTABLE, extractable),
	}

	publicKeyHandle, privateKeyHandle, err := be.ctx.GenerateKeyPair(be.sessionHandle,
		[]*pkcs11.Mechanism{pkcs11.NewMechanism(pkcs11.CKM_EC_KEY_PAIR_GEN, nil)},
		publicKeyTemplate, privateKeyTemplate)

	return publicKeyHandle, privateKeyHandle, err
}

// take a Go crypto ECDSA key and store it inside the device.
// TODO: change this function to accept only wrapped keys.
func (be *Pkcs11Backend) storeEcdsaPair(ID string, key *ecdsa.PrivateKey, extractable bool) (pkcs11.ObjectHandle, pkcs11.ObjectHandle, error) {
	curve := key.PublicKey.Curve
	ecParam, err := asn1.Marshal(curveOIDs[curve.Params().Name])
	if err != nil {
		return invalidObjectHandle, invalidObjectHandle, errors.Wrapf(err, "Error marshaling curve parameters, curve: %s", curve.Params().Name)
	}

	point := elliptic.Marshal(curve, key.X, key.Y)
	asn1Point, err := asn1.Marshal(point)
	if err != nil {
		return invalidObjectHandle, invalidObjectHandle, errors.Wrap(err, "Error marshaling point coordinates")
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
		return invalidObjectHandle, invalidObjectHandle, errors.Wrapf(err, "Error storing public key")
	}

	privateKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PRIVATE_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_KEY_TYPE, pkcs11.CKK_EC),
		pkcs11.NewAttribute(pkcs11.CKA_EC_PARAMS, ecParam),
		pkcs11.NewAttribute(pkcs11.CKA_VALUE, key.D.Bytes()),
		pkcs11.NewAttribute(pkcs11.CKA_TOKEN, true), // persistent
		pkcs11.NewAttribute(pkcs11.CKA_SIGN, true),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
		pkcs11.NewAttribute(pkcs11.CKA_SENSITIVE, true),
		pkcs11.NewAttribute(pkcs11.CKA_EXTRACTABLE, extractable),
	}
	privateKeyHandle, err := be.ctx.CreateObject(be.sessionHandle, privateKeyTemplate)
	if err != nil {
		be.ctx.DestroyObject(be.sessionHandle, publicKeyHandle)
		return invalidObjectHandle, invalidObjectHandle, errors.Wrapf(err, "Error storing private key")
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
				log.Errorf("Error unmarshaling point: %v", err)
			}
			if len(rest) > 0 {
				log.Errorf("Error unmarshaling point: %d bytes left", len(rest))
			}
		} else if a.Type == pkcs11.CKA_EC_PARAMS {
			rest, err := asn1.Unmarshal(a.Value, &curveOID)
			if err != nil {
				log.Errorf("Error unmarshaling point: %v", err)
			}
			if len(rest) > 0 {
				log.Errorf("Error unmarshaling point: %d bytes left", len(rest))
			}
		}
	}
	curve := namedCurveFromOID(curveOID)
	x, y := elliptic.Unmarshal(curve, point.Bytes)
	if x == nil || y == nil {
		return nil, fmt.Errorf("error parsing public session attributes")
	}
	publicKey := &ecdsa.PublicKey{
		Curve: curve,
		X:     x,
		Y:     y,
	}
	return publicKey, nil
}

// takes a KeyPair containing a Go crypto key (RSA or ECDSA) and stores it inside the device.
// TODO: change this function to accept only wrapped keys
func (be *Pkcs11Backend) storeKeyPair(kp *KeyPair, extractable bool) (pkcs11.ObjectHandle, pkcs11.ObjectHandle, error) {
	switch kp.KeyType {
	case RSA1024, RSA2048, RSA4096:
		return be.storeRsaPair(kp.ID(), kp.Signer.(*rsa.PrivateKey), extractable)
	case ECDSA224, ECDSA256, ECDSA384, ECDSA521:
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
	extractable := false
	var publicKey crypto.PublicKey

	switch keyType {
	case RSA1024, RSA2048, RSA4096:
		publicKeyHandle, privateKeyHandle, err = be.newRsaPair(ID, rsaKeyTypeToBitSize[keyType], extractable)
		if err != nil {
			return nil, errors.Wrapf(err, "Error creating RSA key pair. Backend: %+v", be)
		}
		publicKey, err = be.readRsaPublicKey(publicKeyHandle)
		if err != nil {
			return nil, errors.Wrapf(err, "Error retrieving RSA public key. Backend: %+v", be)
		}
	case ECDSA224, ECDSA256, ECDSA384, ECDSA521:
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
func (be *Pkcs11Backend) findObject(template []*pkcs11.Attribute) (pkcs11.ObjectHandle, error) {
	err := be.ctx.FindObjectsInit(be.sessionHandle, template)
	if err != nil {
		return invalidObjectHandle, errors.Wrapf(err, "Error initializing find function for certificate, template: %+v", template)
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
	template := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_CERTIFICATE),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
	}
	objHandle, err := be.findObject(template)
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
	privateKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PRIVATE_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
	}
	privateKeyHandle, err := be.findObject(privateKeyTemplate)
	if err != nil {
		return invalidObjectHandle, invalidObjectHandle, errors.Wrapf(err, "Error retrieving private key handle, ID:", ID)
	}
	publicKeyTemplate := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_PUBLIC_KEY),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
	}
	publicKeyHandle, err := be.findObject(publicKeyTemplate)
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

// destroy a certificate stored in the device
func (be *Pkcs11Backend) destroyCertificate(ID string) error {
	template := []*pkcs11.Attribute{
		pkcs11.NewAttribute(pkcs11.CKA_CLASS, pkcs11.CKO_CERTIFICATE),
		pkcs11.NewAttribute(pkcs11.CKA_LABEL, ID),
	}
	objHandle, err := be.findObject(template)
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
	case ObjectTypeCertificate:
		return be.destroyCertificate(ID)
	default:
		return fmt.Errorf("Unsupported object type: %v, ID: %v", objType, ID)
	}
}

// GetInfo returns info about the pkcs11 module
func (be *Pkcs11Backend) GetInfo() (pkcs11.Info, error) {
	return be.ctx.GetInfo()
}

// Close terminates the session and performs a logout
func (be *Pkcs11Backend) Close() error {
	// we do not logout because logout is a global operation that affects all sessions
	return be.ctx.CloseSession(be.sessionHandle)
}

// NewPkcs11Backend creates a new instance of the backend.
// It requires the path to the PKCS#11 .so library and a PIN.
// During initialization it checks all slots with an available token and chooses the first
// one that has a label matching tokenLabel and tries to login with the given PIN.
// If not found, it tries to initialize an uninitialized token and set label and PIN.
func NewPkcs11Backend(modulePath, tokenLabel, pin string) (*Pkcs11Backend, error) {
	if modulePath == "" {
		return nil, fmt.Errorf("modulePath cannot be empty")
	}
	// empty token label and pin can be ok, depending on the PKCS#11 device

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
