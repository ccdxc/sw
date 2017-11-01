// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// Tests for RFC3394/RFC5649 implementation.

package keymgr

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func testRFC3394Vector(t *testing.T, keyDataStr, kekStr, refWrappedKeyStr string) {
	// For RFC 3394 we use the low-level aesWrapKey and aesUnwrapKey API with the
	// default initialization vector
	kek, err := hex.DecodeString(kekStr)
	AssertOk(t, err, "Invalid kek string")
	keyData, err := hex.DecodeString(keyDataStr)
	AssertOk(t, err, "Invalid key data string")
	refWrappedKey, err := hex.DecodeString(refWrappedKeyStr)
	AssertOk(t, err, "Invalid reference wrapped key string")

	clearTextLen := len(keyData)
	n := clearTextLen / 8
	wrappedLen := (n + 1) * 8
	wrappedKey := make([]byte, wrappedLen)

	result := aesWrapKey(kek, defaultIV, wrappedKey, keyData, uint(clearTextLen))
	Assert(t, result == wrappedLen, fmt.Sprintf("Error wrapping key, have len: %v, want len: %v", result, wrappedLen))
	Assert(t, bytes.Equal(wrappedKey, refWrappedKey), fmt.Sprintf("Error wrapping key, have: %v, want: %v", hex.EncodeToString(wrappedKey), hex.EncodeToString(refWrappedKey)))

	unwrappedKey := make([]byte, clearTextLen)
	result = aesUnwrapKey(kek, defaultIV, unwrappedKey, wrappedKey, uint(wrappedLen), nil)
	Assert(t, result == clearTextLen, fmt.Sprintf("Error unwrapping key, have len: %v, want len: %v", result, clearTextLen))
	Assert(t, bytes.Equal(unwrappedKey, keyData), fmt.Sprintf("Error unwrapping key, have: %v, want: %v", hex.EncodeToString(unwrappedKey), hex.EncodeToString(keyData)))
}

func testRFC5649Vector(t *testing.T, keyDataStr, kekStr, refWrappedKeyStr string) {
	// For RFC 5649 we use the high-level AesKeyWrapWithPad and AesKeyUnwrapWithPad APIs
	// that hide the padding scheme and the choice of the initialization vector
	kek, err := hex.DecodeString(kekStr)
	AssertOk(t, err, "Invalid kek string")
	keyData, err := hex.DecodeString(keyDataStr)
	AssertOk(t, err, "Invalid key data string")
	refWrappedKey, err := hex.DecodeString(refWrappedKeyStr)
	AssertOk(t, err, "Invalid reference wrapped key string")

	wrap, err := AesKeyWrapWithPad(keyData, kek)
	Assert(t, bytes.Equal(wrap, refWrappedKey), "Error wrapping key, have: %v, want: %v", wrap, refWrappedKey)
	unwrap, err := AesKeyUnwrapWithPad(wrap, kek)
	AssertOk(t, err, fmt.Sprintf("Error unwrapping key %+v -> %+v", hex.EncodeToString(wrap), hex.EncodeToString(keyData)))
	Assert(t, bytes.Equal(unwrap, keyData), "Error wrapping key, have: %v, want: %v", unwrap, keyData)
}

func TestRFC3394TestVectors(t *testing.T) {
	keks := map[int]string{
		128: "000102030405060708090A0B0C0D0E0F",
		192: "000102030405060708090A0B0C0D0E0F1011121314151617",
		256: "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F",
	}

	keyData := map[int]string{
		128: "00112233445566778899AABBCCDDEEFF",
		192: "00112233445566778899AABBCCDDEEFF0001020304050607",
		256: "00112233445566778899AABBCCDDEEFF000102030405060708090A0B0C0D0E0F",
	}

	refWrappedKeys := []string{
		"1FA68B0A8112B447AEF34BD8FB5A7B829D3E862371D2CFE5",
		"96778B25AE6CA435F92B5B97C050AED2468AB8A17AD84E5D",
		"64E8C3F9CE0F5BA263E9777905818A2A93C8191E7D6E8AE7",
		"031D33264E15D33268F24EC260743EDCE1C6C7DDEE725A936BA814915C6762D2",
		"A8F9BC1612C68B3FF6E6F4FBE30E71E4769C8B80A32CB8958CD5D17D6B254DA1",
		"28C9F404C4B810F4CBCCB35CFB87F8263F5786E2D80ED326CBC7F0E71A99F43BFB988B9B7A02DD21",
	}

	// 4.1 Wrap 128 bits of Key Data with a 128-bit KEK
	testRFC3394Vector(t, keyData[128], keks[128], refWrappedKeys[0])

	// 4.2 Wrap 128 bits of Key Data with a 192-bit KEK
	testRFC3394Vector(t, keyData[128], keks[192], refWrappedKeys[1])

	// 4.3 Wrap 128 bits of Key Data with a 256-bit KEK
	testRFC3394Vector(t, keyData[128], keks[256], refWrappedKeys[2])

	// 4.4 Wrap 192 bits of Key Data with a 192-bit KEK
	testRFC3394Vector(t, keyData[192], keks[192], refWrappedKeys[3])

	// 4.5 Wrap 192 bits of Key Data with a 256-bit KEK
	testRFC3394Vector(t, keyData[192], keks[256], refWrappedKeys[4])

	// 4.6 Wrap 256 bits of Key Data with a 256-bit KEK
	testRFC3394Vector(t, keyData[256], keks[256], refWrappedKeys[5])
}

func TestRFC5649TestVectors(t *testing.T) {
	// 6.  Padded Key Wrap Examples
	kek := "5840df6e29b02af1ab493b705bf16ea1ae8338f4dcc176a8"

	keyData := []string{
		"c37b7e6492584340bed12207808941155068f738",
		"466f7250617369",
	}

	refWrappedKey := []string{
		"138bdeaa9b8fa7fc61f97742e72248ee5ae6ae5360d1ae6a5f54f373fa543b6a",
		"afbeb0f07dfbf5419200f2ccb50bb24f",
	}

	for i := range keyData {
		testRFC5649Vector(t, keyData[i], kek, refWrappedKey[i])
	}
}

func doWrapUnwrapCompare(t *testing.T, keyData, kek []byte) {
	wrap, err := AesKeyWrapWithPad(keyData, kek)
	AssertOk(t, err, fmt.Sprintf("Error wrapping key data, len: %v", len(keyData)))
	unwrap, err := AesKeyUnwrapWithPad(wrap, kek)
	AssertOk(t, err, fmt.Sprintf("Error unwrapping key data, len: %v", len(keyData)))
	Assert(t, bytes.Equal(unwrap, keyData), fmt.Sprintf("Key data mismatch, len: %v", len(keyData)))
}

func TestMultipleKeyLen(t *testing.T) {
	// Use random keys and random keks
	kek128 := make([]byte, 128/8)
	_, err := rand.Read(kek128)
	AssertOk(t, err, "Error generating random AES key")
	kek192 := make([]byte, 192/8)
	_, err = rand.Read(kek192)
	AssertOk(t, err, "Error generating random AES key")
	kek256 := make([]byte, 256/8)
	_, err = rand.Read(kek256)
	AssertOk(t, err, "Error generating random AES key")

	keks := [][]byte{kek128, kek192, kek256}

	for i := uint(1); i < 1976; i++ {
		keyData := make([]byte, i)
		_, err = rand.Read(keyData)
		AssertOk(t, err, "Error generating random key")
		for _, k := range keks {
			doWrapUnwrapCompare(t, keyData, k)
		}
	}

	// Test max size key
	keyData := make([]byte, maxKeyLen)
	_, err = rand.Read(keyData)
	AssertOk(t, err, "Error generating random key")
	for _, k := range keks {
		doWrapUnwrapCompare(t, keyData, k)
	}
}
