// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package storage

import (
	"bytes"
	"context"
	"crypto/rand"
	"encoding/base64"
	"fmt"
	"io"
	mathrand "math/rand"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func checkRoundTrip(t *testing.T, st ValueTransformer, preTx []byte) {
	sst := st.(*secretValueTransformer)
	postTx, err := sst.TransformToStorage(context.Background(), preTx)
	AssertOk(t, err, "Error transforming to storage")
	// verify that output is base64 encoded
	decPostTx, err := base64.StdEncoding.DecodeString(string(postTx))
	AssertOk(t, err, "Transformed string failed base64 decode")
	Assert(t, !bytes.Equal(decPostTx, preTx), "transformation did not alter string")
	minDecLen := len(preTx) + sst.aeadCipher.NonceSize()
	Assert(t, len(decPostTx) >= minDecLen, fmt.Sprintf("Transformed string too small. Actual: %v, Min: %v", len(decPostTx), minDecLen))
	invTx, err := sst.TransformFromStorage(context.Background(), postTx)
	AssertOk(t, err, "Error transforming from storage")
	Assert(t, bytes.Equal(invTx, preTx), "string does not match after transforming from storage")
}

func TestSecretValueTransformer(t *testing.T) {
	sst, err := NewSecretValueTransformer()
	AssertOk(t, err, "Error instantiating secret transformer")

	// empty string
	checkRoundTrip(t, sst, []byte{})

	// All-zeroes string
	allZeroesString := make([]byte, mathrand.Intn(4096))
	checkRoundTrip(t, sst, allZeroesString)

	// random strings
	for i := 0; i < 100; i++ {
		preTx := make([]byte, mathrand.Intn(1024))
		_, err = io.ReadFull(rand.Reader, preTx)
		checkRoundTrip(t, sst, preTx)
	}

	// big string
	bigString := make([]byte, 4*1024*1024)
	_, err = io.ReadFull(rand.Reader, bigString)
	AssertOk(t, err, fmt.Sprintf("Error reading %d random bytes", len(bigString)))
	checkRoundTrip(t, sst, bigString)

	// NEGATIVE TEST-CASES

	// nil data
	_, err = sst.TransformToStorage(context.Background(), nil)
	Assert(t, err != nil, "TransformToStorage did not reject nil data")
	_, err = sst.TransformFromStorage(context.Background(), nil)
	Assert(t, err != nil, "TransformFromStorage did not reject nil data")

	// garbage data
	garbageString := make([]byte, 256)
	_, err = io.ReadFull(rand.Reader, garbageString)
	AssertOk(t, err, fmt.Sprintf("Error reading %d random bytes", len(garbageString)))
	_, err = sst.TransformFromStorage(context.Background(), garbageString)
	Assert(t, err != nil, "TransformFromStorage did not reject unencoded garbage data")
	_, err = sst.TransformFromStorage(context.Background(), []byte(base64.StdEncoding.EncodeToString(garbageString)))
	Assert(t, err != nil, "TransformFromStorage did not reject garbage data")
}
