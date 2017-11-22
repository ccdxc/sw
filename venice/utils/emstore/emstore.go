// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// abstract embedded data store interface

package emstore

import (
	"errors"
	"fmt"
	"sync"

	"github.com/boltdb/bolt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
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

// ErrTableNotFound table not found error
var ErrTableNotFound = errors.New("Table not found")

// Emstore is the embedded datastore interface
type Emstore interface {
	Read(obj Object) (Object, error)
	List(obj Object) ([]Object, error)
	Write(obj Object) error
	Delete(obj Object) error
	Close() error
	GetNextID(r ResourceID) (uint64, error)
}

// BoltdbStore hold bolt db instance members
type BoltdbStore struct {
	dbPath string   // file path
	boltDb *bolt.DB // boltdb instance
}

// ResourceID holds id for the a particular resource
type ResourceID struct {
	ResType string
	ID      uint64
}

// kindStore stores all keys for a kind
type kindStore struct {
	store map[string]Object // map of objects
}

// MemStore is in memory store to be used for unit testing
type MemStore struct {
	sync.Mutex
	kindStoreMap map[string]*kindStore // map of kinds
	resID        *resourceIDAllocator  //Map of resource id allocations
}

// getObjectKey return an object key for the object
func getObjectKey(obj Object) []byte {
	ometa := obj.GetObjectMeta()
	kstr := fmt.Sprintf("%s-%s-%s", obj.GetObjectKind(), ometa.GetTenant(), ometa.GetName())
	return []byte(kstr)
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

	// create boltdb instance
	boltdb := BoltdbStore{
		dbPath: dbPath,
		boltDb: db,
	}

	return &boltdb, nil
}

// Read reads an object from embedded db and returns it
func (bdb *BoltdbStore) Read(obj Object) (Object, error) {
	kstr := getObjectKey(obj)

	// read the data
	var data []byte
	err := bdb.boltDb.View(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(obj.GetObjectKind()))
		if b == nil {
			return ErrTableNotFound
		}
		data = b.Get(kstr)
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

// List returns a list of all objects of a kind
func (bdb *BoltdbStore) List(obj Object) ([]Object, error) {
	// read the data
	var list []Object
	err := bdb.boltDb.View(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(obj.GetObjectKind()))
		if b == nil {
			return ErrTableNotFound
		}
		c := b.Cursor()

		// loop thru all values
		for k, v := c.First(); k != nil; k, v = c.Next() {
			err := obj.Unmarshal(v)
			if err != nil {
				return err
			}
			newObj := ref.DeepCopy(obj)
			switch newObj.(type) {
			case Object:
				lobj := newObj.(Object)
				list = append(list, lobj)
			}
		}
		return nil
	})
	if err != nil {
		return nil, err
	}

	return list, err
}

// Create creates an object in emstore
func (bdb *BoltdbStore) Create(obj Object) error {
	return bdb.Write(obj)
}

// Update updates an object in emstore
func (bdb *BoltdbStore) Update(obj Object) error {
	return bdb.Write(obj)
}

// rawWrite raw write to db
func (bdb *BoltdbStore) rawWrite(kind string, key, data []byte) error {
	// write to db
	err := bdb.boltDb.Update(func(tx *bolt.Tx) error {
		var terr error
		b := tx.Bucket([]byte(kind))
		if b == nil {
			b, terr = tx.CreateBucket([]byte(kind))
			if terr != nil {
				return fmt.Errorf("Error creating bucket: %s", terr)
			}
		}
		perr := b.Put(key, data)
		return perr
	})
	if err != nil {
		return err
	}

	return nil
}

// Write writes an object into embedded db
func (bdb *BoltdbStore) Write(obj Object) error {
	// encode the data
	data, err := obj.Marshal()
	if err != nil {
		return err
	}

	// write to db
	return bdb.rawWrite(obj.GetObjectKind(), getObjectKey(obj), data)
}

// Delete deletes an object from db
func (bdb *BoltdbStore) Delete(obj Object) error {
	// delete from boltdb
	err := bdb.boltDb.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(obj.GetObjectKind()))
		if b == nil {
			return ErrTableNotFound
		}
		derr := b.Delete(getObjectKey(obj))
		return derr
	})
	if err != nil {
		return err
	}

	return nil
}

// GetNextID gets the next id for the resource boltdb resource type
func (bdb *BoltdbStore) GetNextID(r ResourceID) (uint64, error) {
	err := bdb.boltDb.Update(func(tx *bolt.Tx) error {
		_, err := tx.CreateBucketIfNotExists([]byte(r.ResType))
		if err != nil {
			return err
		}

		b := tx.Bucket([]byte(r.ResType))
		// NextSequence gets an auto incrementing sequence for the bucket. Each resource type is maintained as a separate bucket to allow
		// concurrency. IDs are unique (auto-incremented) per resource type.
		id, _ := b.NextSequence()

		r.ID = uint64(id)
		return nil
	})
	return r.ID, err
}

// Close closes the database
func (bdb *BoltdbStore) Close() error {
	return bdb.boltDb.Close()
}

// NewMemStore returns new memory based emstore
// This is used for unit testing purposes only
func NewMemStore() (*MemStore, error) {
	var res resourceIDAllocator
	// create memkv store instance
	mkv := MemStore{
		kindStoreMap: make(map[string]*kindStore),
		resID:        &res,
	}

	return &mkv, nil
}

// Read reads from memstore
func (mdb *MemStore) Read(robj Object) (Object, error) {
	// lock the memstore
	mdb.Lock()
	defer mdb.Unlock()

	// get kindStoe from kind
	ks, ok := mdb.kindStoreMap[robj.GetObjectKind()]
	if !ok {
		return nil, ErrTableNotFound
	}

	// read it from map
	obj, ok := ks.store[string(getObjectKey(robj))]
	if !ok {
		return nil, errors.New("Object not found")
	}
	return obj, nil
}

// List returns a list of objects of a kind
func (mdb *MemStore) List(obj Object) ([]Object, error) {
	// lock the memstore
	mdb.Lock()
	defer mdb.Unlock()

	// get kindStoe from kind
	ks, ok := mdb.kindStoreMap[obj.GetObjectKind()]
	if !ok {
		return nil, ErrTableNotFound
	}

	var lobj []Object
	for _, v := range ks.store {
		lobj = append(lobj, v)
	}

	return lobj, nil
}

// Update updates an object in memstore
func (mdb *MemStore) Write(obj Object) error {
	// lock the memstore
	mdb.Lock()
	defer mdb.Unlock()

	// get kindStoe from kind
	ks, ok := mdb.kindStoreMap[obj.GetObjectKind()]
	if !ok {
		ks = &kindStore{
			store: make(map[string]Object),
		}

		mdb.kindStoreMap[obj.GetObjectKind()] = ks
	}

	// save it in map
	ks.store[string(getObjectKey(obj))] = obj
	return nil
}

// Delete deletes from memstore
func (mdb *MemStore) Delete(obj Object) error {
	// lock the memstore
	mdb.Lock()
	defer mdb.Unlock()

	// get kindStoe from kind
	ks, ok := mdb.kindStoreMap[obj.GetObjectKind()]
	if !ok {
		return ErrTableNotFound
	}

	// delete from map
	delete(ks.store, string(getObjectKey(obj)))
	return nil
}

// GetNextID gets the next id for the resource memDB resource type
func (mdb *MemStore) GetNextID(r ResourceID) (uint64, error) {
	return mdb.resID.getNextID(r)
}

// Close closes memstore
func (mdb *MemStore) Close() error {
	return nil
}
