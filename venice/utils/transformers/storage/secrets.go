// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package storage

import (
	"context"
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"encoding/base64"
	"io"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/ctxutils"
)

// secretValueTransformer is a storage transformer used to keep secrets safe in persistent storage
type secretValueTransformer struct {
	aeadCipher cipher.AEAD
}

// NewSecretValueTransformer returns a new initialized instance of SecretValueTransformer
func NewSecretValueTransformer() (ValueTransformer, error) {
	// TODO -- replace hard-coded key with properly managed keys
	aesCipher, err := aes.NewCipher([]byte("0123456789abcdef"))
	if err != nil {
		return nil, errors.Wrapf(err, "NewSecretValueTransformer: error creating new AES Cipher")
	}
	gcmCipher, err := cipher.NewGCM(aesCipher)
	if err != nil {
		return nil, errors.Wrapf(err, "NewSecretValueTransformer: error creating new AEAD Cipher")
	}
	return &secretValueTransformer{
		aeadCipher: gcmCipher,
	}, nil
}

func (sst *secretValueTransformer) encrypt(plaintext, associatedData []byte) ([]byte, error) {
	nonce := make([]byte, sst.aeadCipher.NonceSize())
	if _, err := io.ReadFull(rand.Reader, nonce); err != nil {
		return nil, errors.Wrap(err, "SecretValueTransformer: error generating nonce")
	}
	ciphertext := sst.aeadCipher.Seal(nil, nonce, plaintext, associatedData)
	return append(nonce, ciphertext...), nil
}

func (sst *secretValueTransformer) decrypt(nonceAndCiphertext, associatedData []byte) ([]byte, error) {
	nonceSize := sst.aeadCipher.NonceSize()
	if len(nonceAndCiphertext) < nonceSize {
		return nil, errors.New("ciphertext length too small")
	}
	plaintext, err := sst.aeadCipher.Open(nil, nonceAndCiphertext[0:nonceSize], nonceAndCiphertext[nonceSize:], associatedData)
	if err != nil {
		return nil, err
	}
	return plaintext, nil
}

// TransformToStorage receives a plaintext secret and returns the encrypted and encoded
// version suitable for persistent storage.
// The original plaintext is unchanged, so caller should zeroize it when it is no longer needed.
func (sst *secretValueTransformer) TransformToStorage(ctx context.Context, plaintext []byte) ([]byte, error) {
	if plaintext == nil {
		return nil, errors.New("TransformToStorage: secret cannot be nil")
	}
	ciphertext, err := sst.encrypt(plaintext, nil)
	if err != nil {
		return nil, errors.Wrap(err, "TransformToStorage: error encrypting secret")
	}
	return []byte(base64.StdEncoding.EncodeToString(ciphertext)), nil
}

// TransformFromStorage receives a secret read from persistent storage and returns the
// original plaintext version
// Caller should zeroize the plaintext when no longer needed.
func (sst *secretValueTransformer) TransformFromStorage(ctx context.Context, storedData []byte) ([]byte, error) {
	if storedData == nil {
		return nil, errors.New("TransformFromStorage: data cannot be nil")
	}
	if ctxutils.GetPeerID(ctx) == globals.APIGw {
		return nil, nil
	}
	ciphertext, err := base64.StdEncoding.DecodeString(string(storedData))
	if err != nil {
		return nil, errors.Wrapf(err, "TransformFromStorage: error decoding secret")
	}
	plaintext, err := sst.decrypt(ciphertext, nil)
	if err != nil {
		return nil, errors.Wrap(err, "TransformFromStorage: error decrypting secret")
	}
	return plaintext, nil
}
