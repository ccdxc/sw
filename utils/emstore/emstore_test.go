// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package emstore

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	. "github.com/pensando/sw/utils/testutils"
)

func testEmstoreAddDelete(t *testing.T, dbType DbType, dbPath string) {
	db, err := NewEmstore(dbType, dbPath)
	AssertOk(t, err, "Error opening db")

	// test object
	obj := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "testObj"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "tenant",
			Name:   "testName",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet: "10.1.1.1/24",
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
	AssertEquals(t, nobj.Spec.IPv4Subnet, obj.Spec.IPv4Subnet, "Read invalid network params")

	// delete from the db
	err = db.Delete(&obj)
	AssertOk(t, err, "Error deleting the object")

	// read back to make sure its gone
	_, err = db.Read(&ometa)
	Assert(t, (err != nil), "Object still found in db after delete")

	// close the db
	err = db.Close()
	AssertOk(t, err, "Error closing the db")
}

func TestBoltdbAddDelete(t *testing.T) {
	testEmstoreAddDelete(t, BoltDBType, "/tmp/boltdb_test.db")
}

func TestMemstoreAddDelete(t *testing.T) {
	testEmstoreAddDelete(t, MemStoreType, "")
}
