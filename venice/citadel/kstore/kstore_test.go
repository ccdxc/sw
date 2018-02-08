// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package kstore_test

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/venice/citadel/kstore"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestKstoreAddDelete(t *testing.T) {
	// create a temp dir
	path, err := ioutil.TempDir("", "kstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create a kstore
	db, err := kstore.NewKstore(kstore.BoltDBType, path)
	AssertOk(t, err, "Error creating kvstore")
	defer db.Close()

	// write some data
	kvs := []*tproto.KeyValue{{Key: []byte("testKey1"), Value: []byte("testVal1")}}
	err = db.Write("test", kvs)
	AssertOk(t, err, "Error writing kvs")

	// read the data back
	keys := []*tproto.Key{{Key: []byte("testKey1")}}
	values, err := db.Read("test", keys)
	AssertOk(t, err, "Error reading the keys")
	Assert(t, len(values) == 1, "Got invalid values", values)
	Assert(t, string(values[0].Value) == "testVal1", "Got invalid value", values[0])

	// list the values
	rkvs, err := db.List("test")
	AssertOk(t, err, "Error listing the kvs")
	Assert(t, len(rkvs) == 1, "Got invalid values during list", rkvs)
	Assert(t, string(rkvs[0].Key) == "testKey1", "Got invalid key", string(rkvs[0].Key))
	Assert(t, string(rkvs[0].Value) == "testVal1", "Got invalid key", string(rkvs[0].Value))

	// verify getting non-existing key returns an error
	invkeys := []*tproto.Key{{Key: []byte("testKey1")}, {Key: []byte("testKey2")}}
	values, err = db.Read("test", invkeys)
	Assert(t, err != nil, "Getting non-existent key suceeded", values)

	// try to list non-existent table
	rkvs, err = db.List("test2")
	Assert(t, err != nil, "Listing non-existing table suceeded", rkvs)

	// delete the value
	err = db.Delete("test", keys)
	AssertOk(t, err, "Error deleting keys")

	// verify key is gone
	_, err = db.Read("test", keys)
	Assert(t, err != nil, "key still found after deleting")

	// verify deleting non-existing table returns an error
	err = db.Delete("test2", keys)
	Assert(t, err != nil, "Deleting from non-existing table suceeded")
}

func TestKstoreBackupRestore(t *testing.T) {
	// create a temp dir
	path, err := ioutil.TempDir("", "kstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create a kstore
	db, err := kstore.NewKstore(kstore.BoltDBType, path)
	AssertOk(t, err, "Error creating kvstore")
	defer db.Close()

	// write some data
	kvs := []*tproto.KeyValue{{Key: []byte("testKey1"), Value: []byte("testVal1")}}
	err = db.Write("test", kvs)
	AssertOk(t, err, "Error writing kvs")

	// get shard info
	var sinfo tproto.SyncShardInfoMsg
	err = db.GetShardInfo(&sinfo)
	AssertOk(t, err, "Error getting shard info")
	jstr, _ := json.MarshalIndent(sinfo, "", "  ")
	log.Infof("Got shard info: %s", jstr)

	// backup chunk
	var buf bytes.Buffer
	err = db.BackupChunk(sinfo.ShardID, &buf)
	AssertOk(t, err, "Error backing up chunk")

	// create another db
	path2, err := ioutil.TempDir("", "kstore2-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path2)

	// create a kstore
	db2, err := kstore.NewKstore(kstore.BoltDBType, path2)
	AssertOk(t, err, "Error creating kvstore")
	defer db.Close()

	// restore shard info new db
	err = db2.RestoreShardInfo(&sinfo)
	AssertOk(t, err, "Error restoring shard info")

	err = db2.RestoreChunk(sinfo.ShardID, &buf)
	AssertOk(t, err, "Error restoring chunks")

	// read the data back from restored DB and verify its correst
	keys := []*tproto.Key{{Key: []byte("testKey1")}}
	values, err := db2.Read("test", keys)
	AssertOk(t, err, "Error reading the keys")
	Assert(t, len(values) == 1, "Got invalid values", values)
	Assert(t, string(values[0].Value) == "testVal1", "Got invalid value", values[0])
}

func TestKstoreBenchmark(t *testing.T) {
	var numIterations = 400
	var batchSize = 1000

	// create a temp dir
	path, err := ioutil.TempDir("", "kstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create a kstore
	db, err := kstore.NewKstore(kstore.BoltDBType, path)
	AssertOk(t, err, "Error creating kvstore")
	defer db.Close()

	type kvlist []*tproto.KeyValue
	type keylist []*tproto.Key
	var list = make([]kvlist, numIterations)
	var klist = make([]keylist, numIterations)

	// parse the key-value pairs
	startTime := time.Now()
	for iter := 0; iter < numIterations; iter++ {
		batch := make([]*tproto.KeyValue, batchSize)
		kbatch := make([]*tproto.Key, batchSize)
		for b := 0; b < batchSize; b++ {
			key := map[string]string{
				"tenant":    fmt.Sprintf("test-%d", iter),
				"namespace": "ns1",
				"name":      fmt.Sprintf("name%d-%d", iter, b),
			}
			value := map[string]string{
				"subnet":     fmt.Sprintf("10.1.%d.%d", iter, b),
				"gateway":    fmt.Sprintf("10.1.%d.254", iter),
				"test":       "foo",
				"blah":       "bar",
				"longstring": "this is a very long string that could go on on on on on on on on on on on on on on on on on on for a while",
			}
			k, _ := json.Marshal(key)
			val, _ := json.Marshal(value)
			batch[b] = &tproto.KeyValue{
				Key:   k,
				Value: val,
			}
			kbatch[b] = &tproto.Key{
				Key: k,
			}
		}
		list[iter] = batch
		klist[iter] = kbatch
	}
	log.Infof("%d iterations at batch size %d parsing key-value pairs took %v ", numIterations, batchSize, time.Since(startTime).String())

	// write the key-value pairs
	startTime = time.Now()
	for iter := 0; iter < numIterations; iter++ {
		err = db.Write("test", list[iter])
		AssertOk(t, err, "Error writing key-values")
	}
	log.Infof("%d iterations at batch size %d writing key-value pairs took %v ", numIterations, batchSize, time.Since(startTime).String())

	// read all the values back
	startTime = time.Now()
	for iter := 0; iter < numIterations; iter++ {
		_, err = db.Read("test", klist[iter])
		AssertOk(t, err, "Error reading keys")
	}
	log.Infof("%d iterations at batch size %d reading key-value pairs took %v ", numIterations, batchSize, time.Since(startTime).String())

	// delete all the values
	startTime = time.Now()
	for iter := 0; iter < numIterations; iter++ {
		err = db.Delete("test", klist[iter])
		AssertOk(t, err, "Error deleting keys")
	}
	log.Infof("%d iterations at batch size %d deleting key-value pairs took %v ", numIterations, batchSize, time.Since(startTime).String())

}
