// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package emstore

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/protos/nmd"

	"github.com/boltdb/bolt"
	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func testEmstoreAddDelete(t *testing.T, dbType DbType, dbPath string) {
	db, err := NewEmstore(dbType, dbPath)
	AssertOk(t, err, "Error opening db")

	config := nmd.DistributedServiceCard{
		ObjectMeta: api.ObjectMeta{
			Name: "DistributedServiceCardConfig",
		},
		TypeMeta: api.TypeMeta{
			Kind: "DistributedServiceCard",
		},
		Spec: nmd.DistributedServiceCardSpec{
			Mode:       nmd.MgmtMode_HOST.String(),
			PrimaryMAC: "baba:baba:baba",
			ID:         "baba:baba:baba",
			DSCProfile: "default",
			IPConfig: &cluster.IPConfig{
				IPAddress:  "",
				DefaultGW:  "",
				DNSServers: nil,
			},
		},
	}
	err = db.Write(&config)
	AssertOk(t, err, "Failed to write DSC Mode")
	o, err := db.Read(&config)
	AssertOk(t, err, "Failed to read DSC Mode")
	c := *o.(*nmd.DistributedServiceCard)
	AssertEquals(t, config.Spec.PrimaryMAC, c.Spec.PrimaryMAC, "Unmarshaled read resp did not match what was written")

	// test object
	obj := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "testObj"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "tenant",
			Name:   "testName",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
	}

	// write the object to db
	err = db.Write(&obj)
	AssertOk(t, err, "Error writing to db")

	// read the values back
	ometa := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "testObj"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "tenant",
			Name:   "testName",
		},
	}
	robj, err := db.Read(&ometa)
	AssertOk(t, err, "Error reading from db")
	nobj := robj.(*netproto.Network)
	AssertEquals(t, nobj.Spec.VlanID, obj.Spec.VlanID, "Read invalid network params")

	// write second object
	obj2 := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "testObj"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "tenant",
			Name:   "testName2",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
	}

	// write the object to db
	err = db.Write(&obj2)
	AssertOk(t, err, "Error writing to db")

	// verify list works
	lobj, err := db.List(&ometa)
	AssertOk(t, err, "Error listing from db")
	Assert(t, (len(lobj) == 2), "Got invalid list", lobj)

	// delete from the db
	err = db.Delete(&obj)
	AssertOk(t, err, "Error deleting the object")

	// read back to make sure its gone
	dobj, err := db.Read(&netproto.Network{TypeMeta: api.TypeMeta{Kind: "testObj"}})
	Assert(t, (err != nil), "Object still found in db after delete", dobj)

	// verify we get only one object in list
	lobj, err = db.List(&ometa)
	AssertOk(t, err, "Error listing from db")
	Assert(t, (len(lobj) == 1), "Got invalid list", lobj)

	// verify reading from non-existing kind will return an error
	ometa.TypeMeta.Kind = "testObj2"
	_, err = db.Read(&ometa)
	Assert(t, err != nil, "Reading from non-existing table succeeded")
	Assert(t, strings.Contains(err.Error(), ErrTableNotFound.Error()), "Incorrect error", err)

	// verify deleting from non-existing kind will return an error
	ometa.TypeMeta.Kind = "testObj2"
	err = db.Delete(&ometa)
	Assert(t, err != nil, "Deleting from non-existing table succeeded")
	Assert(t, strings.Contains(err.Error(), ErrTableNotFound.Error()), "Incorrect error", err)

	// delete from the db
	err = db.Delete(&obj2)
	AssertOk(t, err, "Error deleting the object")

	// close the db
	err = db.Close()
	AssertOk(t, err, "Error closing the db")
}

func TestBoltdbAddDelete(t *testing.T) {
	defer os.Remove("/tmp/boltdb_test.db")
	testEmstoreAddDelete(t, BoltDBType, "/tmp/boltdb_test.db")
}

func TestMemstoreAddDelete(t *testing.T) {
	testEmstoreAddDelete(t, MemStoreType, "")
}

func TestBoltDBRawRead_RawList(t *testing.T) {
	dbPath, err := ioutil.TempFile("", "")
	defer os.Remove(dbPath.Name())
	AssertOk(t, err, "Failed to create bolt DB")
	db, err := NewEmstore(BoltDBType, dbPath.Name())
	AssertOk(t, err, "Error opening db")

	vrf1 := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default-infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
		Status: netproto.VrfStatus{
			VrfID: 42,
		},
	}

	vrf2 := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default-customer",
		},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: 65,
		},
	}

	vrf1RawBytes, _ := vrf1.Marshal()
	vrf2RawBytes, _ := vrf2.Marshal()

	err = db.RawWrite(vrf1.Kind, vrf1.GetKey(), vrf1RawBytes)
	AssertOk(t, err, "Failed to raw write vrf1")
	err = db.RawWrite(vrf2.Kind, vrf2.GetKey(), vrf2RawBytes)
	AssertOk(t, err, "Failed to raw write vrf2")

	// Perform Raw Read
	dat1, err := db.RawRead(vrf1.Kind, vrf1.GetKey())
	AssertOk(t, err, "Failed to find vrf1")
	AssertEquals(t, vrf1RawBytes, dat1, "Write to Read inconsistency detected for vrf1")

	dat2, err := db.RawRead(vrf2.Kind, vrf2.GetKey())
	AssertOk(t, err, "Failed to find vrf2")
	AssertEquals(t, vrf2RawBytes, dat2, "Write to Read inconsistency detected for vrf2")

	rawBytesList, err := db.RawList("Vrf")
	AssertOk(t, err, "Failed to list raw bytes")
	AssertEquals(t, 2, len(rawBytesList), "Expect two entries in the raw list")

}

func TestBoltdbBenchmark(t *testing.T) {
	var numIteration = 3 * 1000
	var batchSize = 100

	// create a temp dir
	path, err := ioutil.TempDir("", "emstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	db, err := NewEmstore(BoltDBType, filepath.Join(path, "bolt.db"))
	AssertOk(t, err, "Error opening db")
	defer db.Close()

	// create the objects
	objs := make([]*netproto.Network, numIteration)
	rawBytes := make(map[string][]byte)
	for idx := 0; idx < numIteration; idx++ {
		objs[idx] = &netproto.Network{
			TypeMeta: api.TypeMeta{Kind: "testObj"},
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant",
				Name:   fmt.Sprintf("testName%d", idx+1),
			},
			Spec: netproto.NetworkSpec{
				VlanID: uint32(idx + 1),
			},
		}

		// pre-marshall the object and keep it
		data, merr := objs[idx].Marshal()
		AssertOk(t, merr, "Error marshalling obj")
		rawBytes[string(getObjectKey(objs[idx]))+"tmp"] = data
	}

	// measure how long writing all objects takes
	startTime := time.Now()
	for idx := 0; idx < numIteration; idx++ {
		err = db.Write(objs[idx])
		AssertOk(t, err, "Error writing to db")
	}
	logrus.Infof("Writing %d objects took %v ", numIteration, time.Since(startTime).String())

	// read all objects and see how long it takes
	startTime = time.Now()
	for idx := 0; idx < numIteration; idx++ {
		_, err = db.Read(objs[idx])
		AssertOk(t, err, "Error reading from db")
	}
	logrus.Infof("Reading %d objects took %v ", numIteration, time.Since(startTime).String())

	// write raw bytes and see how long it takes
	startTime = time.Now()
	for k, v := range rawBytes {
		err = db.(*BoltdbStore).RawWrite("testObj", k, v)
		AssertOk(t, err, "Error writing to db")
	}
	logrus.Infof("Raw Writing %d objects took %v ", numIteration, time.Since(startTime).String())

	// measure batch write performance
	startTime = time.Now()
	for idx := 0; idx < numIteration/batchSize; idx++ {
		err := db.(*BoltdbStore).boltDb.Update(func(tx *bolt.Tx) error {
			for i := 0; i < batchSize; i++ {
				obj := objs[(idx*batchSize)+i]
				data, merr := obj.Marshal()
				if merr != nil {
					return merr
				}
				var terr error
				b := tx.Bucket([]byte("testObj"))
				if b == nil {
					b, terr = tx.CreateBucket([]byte("testObj"))
					if terr != nil {
						return fmt.Errorf("Error creating bucket: %s", terr)
					}
				}
				kstr := string(getObjectKey(obj)) + "batchtmp"
				perr := b.Put([]byte(kstr), data)
				if perr != nil {
					return perr
				}
			}
			return nil
		})
		AssertOk(t, err, "Error writing to db")
	}
	logrus.Infof("Batch Writing %d objects at batch size %d took %v ", numIteration, batchSize, time.Since(startTime).String())

}
