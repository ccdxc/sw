// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// abstract embedded data store interface

package emstore

import (
	"errors"
	"fmt"
	"strings"
	"sync"

	"github.com/boltdb/bolt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
)

// Object is the interface all objects have to implement
type Object interface {
	GetObjectKind() string          // returns the object kind
	GetObjectAPIVersion() string    // api version
	GetObjectMeta() *api.ObjectMeta // returns the object meta
	Marshal() ([]byte, error)       // encode data
	Unmarshal(dAtA []byte) error    // decode data
}

// DbType is embedded store types
type DbType string

// db type enums
const (
	BoltDBType   DbType = "boltdb"
	MemStoreType DbType = "memstore"
)

// Emstore is the embedded datastore interface
type Emstore interface {
	Read(obj Object) (Object, error)
	Write(obj Object) error
	Delete(obj Object) error
	Close() error
}

// BoltdbStore is the
type BoltdbStore struct {
	dbPath  string   // file path
	boltDb  *bolt.DB // boltdb instance
	boltBkt *bolt.Bucket
}

// MemStore is in memory store to be used for unit testing
type MemStore struct {
	sync.Mutex
	store map[string]Object
}

// getObjectKey return an object key for the object
func getObjectKey(obj Object) []byte {
	ometa := obj.GetObjectMeta()
	return []byte(fmt.Sprintf("%s|%s|%s", obj.GetObjectKind(), ometa.GetTenant(), ometa.GetName()))
}

// NewEmstore returns an embedded data store
func NewEmstore(dbType DbType, dbPath string) (Emstore, error) {
	switch dbType {
	case BoltDBType:
		return NewBoltdbStore(dbPath)
	case MemStoreType:
		return NewMemStore()
	default:
		log.Fatalf("Unknown db type: %v", dbType)
		return nil, errors.New("Unknown db type")
	}
}

// NewBoltdbStore returns a boltdb store
func NewBoltdbStore(dbPath string) (*BoltdbStore, error) {
	// open the database
	db, err := bolt.Open(dbPath, 0600, nil)
	if err != nil {
		log.Fatal(err)
		return nil, err
	}

	// create a bucket for storing objects
	var bkt *bolt.Bucket
	err = db.Update(func(tx *bolt.Tx) error {
		bkt, err = tx.CreateBucket([]byte("default"))
		if err != nil {
			return fmt.Errorf("Error creating bucket: %s", err)
		}
		return nil
	})
	if err != nil && !strings.Contains(err.Error(), bolt.ErrBucketExists.Error()) {
		log.Errorf("Error cretaing boltdb bucket. Err: %v", err)
		return nil, err
	}

	// create boltdb instance
	boltdb := BoltdbStore{
		dbPath:  dbPath,
		boltDb:  db,
		boltBkt: bkt,
	}

	return &boltdb, nil
}

// Read reads an object from embedded db and returns it
func (bdb *BoltdbStore) Read(obj Object) (Object, error) {
	// read the data
	var data []byte
	err := bdb.boltDb.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte("default"))
		data = b.Get(getObjectKey(obj))
		return nil
	})
	if err != nil {
		return nil, err
	}

	// check if we got anything
	if data == nil {
		return nil, errors.New("Object not found")
	}

	// unmarshall data
	err = obj.Unmarshal(data)
	if err != nil {
		return nil, err
	}

	return obj, nil
}

// Create creates an object in emstore
func (bdb *BoltdbStore) Create(obj Object) error {
	return bdb.Write(obj)
}

// Update updates an object in emstore
func (bdb *BoltdbStore) Update(obj Object) error {
	return bdb.Write(obj)
}

// Write writes an object into embedded db
func (bdb *BoltdbStore) Write(obj Object) error {
	// encode the data
	data, err := obj.Marshal()
	if err != nil {
		return err
	}

	// write to db
	err = bdb.boltDb.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte("default"))
		perr := b.Put(getObjectKey(obj), data)
		return perr
	})
	if err != nil {
		return err
	}

	return nil
}

// Delete deletes an object from db
func (bdb *BoltdbStore) Delete(obj Object) error {
	// delete from boltdb
	err := bdb.boltDb.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte("default"))
		derr := b.Delete(getObjectKey(obj))
		return derr
	})
	if err != nil {
		return err
	}

	return nil
}

// Close closes the database
func (bdb *BoltdbStore) Close() error {
	return bdb.boltDb.Close()
}

// NewMemStore returns new memory based emstore
// This is used for unit testing purposes only
func NewMemStore() (*MemStore, error) {
	// create memkv store instance
	mkv := MemStore{
		store: make(map[string]Object),
	}

	return &mkv, nil
}

// Read reads from memstore
func (mdb *MemStore) Read(robj Object) (Object, error) {
	// lock the memstore
	mdb.Lock()
	defer mdb.Unlock()

	// read it from map
	obj, ok := mdb.store[string(getObjectKey(robj))]
	if !ok {
		return nil, errors.New("Object not found")
	}
	return obj, nil
}

// Update updates an object in memstore
func (mdb *MemStore) Write(obj Object) error {
	// lock the memstore
	mdb.Lock()
	defer mdb.Unlock()

	// save it in map
	mdb.store[string(getObjectKey(obj))] = obj
	return nil
}

// Delete deletes from memstore
func (mdb *MemStore) Delete(obj Object) error {
	// lock the memstore
	mdb.Lock()
	defer mdb.Unlock()

	// delete from map
	delete(mdb.store, string(getObjectKey(obj)))
	return nil
}

// Close closes memstore
func (mdb *MemStore) Close() error {
	return nil
}
