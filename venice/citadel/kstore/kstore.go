// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// kev-value store backend for citadel

package kstore

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"os"

	"github.com/boltdb/bolt"

	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
)

// DbType is embedded key-value store types
type DbType string

// db type enums
const (
	BoltDBType DbType = "boltdb"
)

// ErrTableNotFound table not found error
var ErrTableNotFound = errors.New("Table not found")

// Kstore is the key-value store interface
type Kstore interface {
	// CRUD apis
	Read(table string, keys []*tproto.Key) ([]*tproto.KeyValue, error)
	List(table string) ([]*tproto.KeyValue, error)
	Write(table string, kvs []*tproto.KeyValue) error
	Delete(table string, keys []*tproto.Key) error
	Close() error

	// store api expected by datanode
	GetShardInfo(sinfo *tproto.SyncShardInfoMsg) error
	RestoreShardInfo(sinfo *tproto.SyncShardInfoMsg) error
	BackupChunk(chunkID uint64, w io.Writer) error
	RestoreChunk(chunkID uint64, r io.Reader) error
}

// BoltdbStore is the
type BoltdbStore struct {
	dbPath string   // file path
	boltDb *bolt.DB // boltdb instance
}

// NewKstore returns a kev-value store
func NewKstore(dbType DbType, dbPath string) (Kstore, error) {
	switch dbType {
	case BoltDBType:
		return NewBoltdbStore(dbPath)
	default:
		log.Fatalf("Unknown db type: %v", dbType)
		return nil, errors.New("Unknown db type")
	}
}

// NewBoltdbStore returns a boltdb store
func NewBoltdbStore(dbPath string) (*BoltdbStore, error) {
	// create the directory if it doesnt exist
	err := os.MkdirAll(dbPath, 0755)
	if err != nil {
		return nil, err
	}

	// open the database
	filePath := fmt.Sprintf("%s/bolt.db", dbPath)
	db, err := bolt.Open(filePath, 0600, nil)
	if err != nil {
		log.Fatal(err)
		return nil, err
	}

	// create boltdb instance
	boltdb := BoltdbStore{
		dbPath: filePath,
		boltDb: db,
	}

	return &boltdb, nil
}

// Read reads a list of keys and returns the value
func (bdb *BoltdbStore) Read(table string, keys []*tproto.Key) ([]*tproto.KeyValue, error) {
	var values = make([]*tproto.KeyValue, len(keys))

	// read the data
	err := bdb.boltDb.View(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(table))
		if b == nil {
			return ErrTableNotFound
		}
		for idx, key := range keys {
			val := b.Get(key.Key)
			if val == nil {
				return errors.New("Object not found")
			}
			values[idx] = &tproto.KeyValue{
				Key:   key.Key,
				Value: val,
			}
		}
		return nil
	})
	if err != nil {
		return nil, err
	}

	return values, nil
}

// List returns a list of all kv-pairs in a table
func (bdb *BoltdbStore) List(table string) ([]*tproto.KeyValue, error) {
	var kvs []*tproto.KeyValue

	// read the data
	err := bdb.boltDb.View(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(table))
		if b == nil {
			return ErrTableNotFound
		}
		c := b.Cursor()

		// loop thru all values
		for k, v := c.First(); k != nil; k, v = c.Next() {
			if k != nil && v != nil {
				kv := &tproto.KeyValue{
					Key:   k,
					Value: v,
				}
				kvs = append(kvs, kv)
			}
		}
		return nil
	})
	if err != nil {
		return nil, err
	}

	return kvs, nil
}

// Write writes a list of kv-pairs to db
func (bdb *BoltdbStore) Write(table string, kvs []*tproto.KeyValue) error {
	// write to db
	err := bdb.boltDb.Update(func(tx *bolt.Tx) error {
		var terr error
		b := tx.Bucket([]byte(table))
		if b == nil {
			b, terr = tx.CreateBucket([]byte(table))
			if terr != nil {
				return fmt.Errorf("Error creating bucket: %s", terr)
			}
		}
		for _, kv := range kvs {
			perr := b.Put(kv.Key, kv.Value)
			if perr != nil {
				return perr
			}
		}
		return nil
	})
	if err != nil {
		return err
	}

	// write to db
	return nil
}

// Delete deletes a list of keys
func (bdb *BoltdbStore) Delete(table string, keys []*tproto.Key) error {
	// delete from boltdb
	err := bdb.boltDb.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(table))
		if b == nil {
			return ErrTableNotFound
		}
		for _, key := range keys {
			derr := b.Delete(key.Key)
			if derr != nil {
				return derr
			}
		}
		return nil
	})
	if err != nil {
		return err
	}

	return nil
}

// GetShardInfo returns shard info
func (bdb *BoltdbStore) GetShardInfo(sinfo *tproto.SyncShardInfoMsg) error {
	// set the chunk ids
	sinfo.Chunks = []*tproto.ChunkInfo{
		{
			ChunkID:  sinfo.ShardID,
			Database: "default",
		},
	}
	return nil
}

// RestoreShardInfo restres a shard
func (bdb *BoltdbStore) RestoreShardInfo(sinfo *tproto.SyncShardInfoMsg) error {
	// nothing to do, all restoration will happen during chunk restore
	return nil
}

// BackupChunk backs up a chunk of database
func (bdb *BoltdbStore) BackupChunk(chunkID uint64, w io.Writer) error {
	// create a read-only txn and ask it to write to a writer
	err := bdb.boltDb.View(func(tx *bolt.Tx) error {
		_, werr := tx.WriteTo(w)
		return werr
	})

	return err
}

// RestoreChunk restores a chunk of database
func (bdb *BoltdbStore) RestoreChunk(chunkID uint64, r io.Reader) error {
	// FIXME: to be implemented
	// close the current db
	err := bdb.boltDb.Close()
	if err != nil {
		return err
	}

	// open the boltdb file
	file, err := os.OpenFile(bdb.dbPath, os.O_RDWR|os.O_CREATE, 0600)
	if err != nil {
		return err
	}

	// write the incoming contents into db file
	rdr := bufio.NewReader(r)
	_, err = rdr.WriteTo(file)
	if err != nil {
		log.Errorf("Error writing to file %s. Err: %v", bdb.dbPath, err)
		return err
	}

	// open the db using newly written file
	db, err := bolt.Open(bdb.dbPath, 0600, nil)
	if err != nil {
		log.Fatal(err)
		return err
	}
	bdb.boltDb = db

	return nil
}

// Close closes the database
func (bdb *BoltdbStore) Close() error {
	return bdb.boltDb.Close()
}
