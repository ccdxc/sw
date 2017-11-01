// Provides an RFC3394/RFC5649 implementation.

package keymgr

import (
	"bytes"
	"crypto/aes"
	"encoding/binary"
	"errors"
	"fmt"
)

// *** Adapted from https://github.com/paypal/go.crypto/blob/master/keystore/keystore.go *** //

var (
	defaultIV   = []byte{0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6}
	alternateIV = []byte{0xA6, 0x59, 0x59, 0xA6}
	// RFC 5649 allows key with size up to 2^32 bits. We enforce a more reasonable limit of 64KB.
	// This can be increased with no issues if needed.
	maxKeyLen = 65536 // 64KB
)

// AesKeyWrapWithPad takes a plaintext key and wraps it with the supplied KEK
func AesKeyWrapWithPad(clearKey []byte, kek []byte) ([]byte, error) {
	// clearKey consists of n 64-bit blocks, containing the key data being wrapped.
	// If the original size of clearKey is not a multiple of 8, clearKey gets padded with 0s on the right until
	// its size reaches the closest multiple of 8.
	// encryptedKey consists of (n + 1) 64-bit blocks containing the wrapped key plus a 4-bytes known value
	// (for integrity checks) and 4 bytes of actual key length, so that padding can be removed after unwrapping.
	clearKeyLen := len(clearKey)
	if clearKeyLen > maxKeyLen {
		return nil, fmt.Errorf("Key is too large. Requested: %d, maximum allowed: %d", clearKeyLen, maxKeyLen)
	}
	encryptedKeyLen := 8*((clearKeyLen+7)/8) + 8 // round up to next multiple of 8 and add 8 for integrity check and padding info
	encryptedKey := make([]byte, encryptedKeyLen)
	ret := aesWrapKeyWithpad(kek, encryptedKey, clearKey, uint(clearKeyLen))
	// aesWrapKeyWithpad returns the size of the wrapped key if it succeeds, or -1 if it fails
	// if it succeeds, the returned value must match the expected size of the wrapped key
	if ret != encryptedKeyLen {
		return nil, errors.New("unable to wrap key")
	}
	return encryptedKey, nil
}

// AesKeyUnwrapWithPad takes a wrapped key and unwraps it with the supplied KEK
func AesKeyUnwrapWithPad(wrappedKey []byte, kek []byte) ([]byte, error) {
	wrappedKeyLen := len(wrappedKey)
	// aesUnwrapKeyWithpad uses awsUnwrapKey (RFC3394), which assumes that the output buffer is always
	// at least 16 bytes long, so we always allocate at least 16 bytes. If the actual key is smaller,
	// the excess bytes are stripped in the return statement of this function
	outBufSize := wrappedKeyLen - 8
	if outBufSize < 16 {
		outBufSize = 16
	}
	key := make([]byte, outBufSize)
	ret := aesUnwrapKeyWithpad(kek, key, wrappedKey, uint(wrappedKeyLen))
	// aesUnwrapKeyWithpad returns the size of the unwrapped key if it succeeds, or -1 if it fails
	// if it succeeds, the returned value must match the expected size of the unwrapped key
	paddedKeyLen := 8 * ((ret + 7) / 8)
	if ret < 0 || (paddedKeyLen != wrappedKeyLen-8) {
		return nil, fmt.Errorf("failed to unwrap key, ret: %v", ret)
	}
	return key[:ret], nil
}

// RFC5649 implementation. Returns the len of encryptedKey, or -1 in case of failure.
func aesWrapKeyWithpad(key []byte, out []byte, in []byte, inlen uint) int {
	var ilen uint
	var ret int
	var input, iv []byte
	ret = -1

	if len(key) < 16 {
		return ret
	}
	cipher, err := aes.NewCipher(key)

	if inlen == 0 || err != nil || uint(len(in)) < inlen || uint(len(out)) < (inlen+8) {
		return ret
	}

	ilen = inlen
	if pad := inlen % 8; pad != 0 {
		ilen = ilen + 8 - pad
	}

	iv = make([]byte, 8)
	input = make([]byte, ilen+8)
	copy(iv, alternateIV[:4])
	binary.BigEndian.PutUint32(iv[4:], uint32(inlen))

	if ilen == 8 {
		copy(input, iv[:8])
		copy(input[8:], in[:inlen])
		cipher.Encrypt(out, input)
		ret = 8 + 8
	} else {
		copy(input, in[:inlen])
		ret = aesWrapKey(key, iv, out, input, ilen)
	}
	return ret
}

// RFC5649 implementation. Returns the length of the key, or -1 in case of failure.
func aesUnwrapKeyWithpad(key []byte, out []byte, in []byte, inlen uint) int {
	var padlen, ilen uint
	var ret = -1
	var aIV, zeroIV []byte

	if len(key) < 16 {
		return ret
	}
	cipher, err := aes.NewCipher(key)
	aIV = make([]byte, 8)
	zeroIV = make([]byte, 8)

	if (inlen&0x7) != 0 || inlen < 16 || err != nil || uint(len(in)) < inlen || uint(len(out)) < 16 {
		return ret
	}

	if inlen == 16 {
		cipher.Decrypt(out, in)
		copy(aIV, out[:8])
		copy(out, out[8:16])
	} else {
		if aesUnwrapKey(key, nil, out, in, inlen, aIV) <= 0 {
			return ret
		}
	}

	if !bytes.Equal(aIV[:4], alternateIV[:4]) {
		return ret
	}

	ilen = uint(binary.BigEndian.Uint32(aIV[4:8]))
	inlen -= 8

	if ilen > inlen || ilen <= (inlen-8) {
		return ret
	}

	padlen = inlen - ilen

	if padlen != 0 && !bytes.Equal(zeroIV[:padlen], out[ilen:ilen+padlen]) {
		return ret
	}

	return int(ilen)
}

// RFC3394 implementation. Returns the length of the encrypted key, or -1 in case of failure.
func aesWrapKey(key []byte, iv []byte, out []byte, in []byte, inlen uint) int {
	var A, B, R []byte
	var i, j, t uint
	var ret = -1
	B = make([]byte, 16)

	if len(key) < 16 {
		return ret
	}
	cipher, err := aes.NewCipher(key)

	if (inlen&0x7) != 0 || (inlen < 8) || err != nil || uint(len(out)) < 16 || uint(len(in)) < inlen {
		return ret
	}

	A = B
	t = 1

	copy(out[8:], in[:inlen])
	if iv == nil {
		iv = defaultIV
	}
	copy(A, iv[:8])

	for j = 0; j < 6; j++ {
		R = out[8:]
		for i = 0; i < inlen; i, t, R = i+8, t+1, R[8:] {
			copy(B[8:], R[:8])
			cipher.Encrypt(B, B)
			A[7] ^= uint8(t & 0xff)
			if t > 0xff {
				A[6] ^= uint8((t >> 8) & 0xff)
				A[5] ^= uint8((t >> 16) & 0xff)
				A[4] ^= uint8((t >> 24) & 0xff)
			}
			copy(R, B[8:16])
		}
	}
	copy(out, A[:8])
	return int(inlen + 8)
}

// RFC3394 implementation. Returns the length of the key, or -1 in case of failure.
func aesUnwrapKey(key []byte, iv []byte, out []byte, in []byte, inlen uint, aIV []byte) int {
	var A, B, R []byte
	var i, j, t uint
	var ret = -1
	B = make([]byte, 16)

	if len(key) < 16 {
		return ret
	}
	cipher, err := aes.NewCipher(key)

	inlen -= 8
	if (inlen&0x7) != 0 || (inlen < 8) || err != nil || uint(len(out)) < (inlen-8) || uint(len(in)) < inlen {
		return ret
	}

	A = B
	t = 6 * (inlen >> 3)

	copy(A, in[:8])
	copy(out, in[8:inlen+8])

	for j = 0; j < 6; j++ {
		for i = 0; i < inlen; i, t = i+8, t-1 {
			R = out[inlen-8-i:]
			A[7] ^= uint8(t & 0xff)
			if t > 0xff {
				A[6] ^= uint8((t >> 8) & 0xff)
				A[5] ^= uint8((t >> 16) & 0xff)
				A[4] ^= uint8((t >> 24) & 0xff)
			}
			copy(B[8:], R[:8])
			cipher.Decrypt(B, B)
			copy(R, B[8:16])
		}
	}

	if aIV != nil {
		copy(aIV, A[:8])
	} else {
		if iv == nil {
			iv = defaultIV
		}
		if !bytes.Equal(A[:8], iv[:8]) {
			return ret
		}
	}
	return int(inlen)
}
