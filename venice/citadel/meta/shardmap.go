// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package meta

import (
	"errors"
	"fmt"
	"hash/crc32"

	"github.com/pensando/sw/venice/citadel/tproto"
)

// NewShardMap creates a new shard map instance
func NewShardMap(numShards, desiredReplicas uint32, cluster *TscaleCluster) (*ShardMap, error) {
	// create a shard map instance
	sm := ShardMap{
		NumShards:       numShards,
		DesiredReplicas: desiredReplicas,
		Shards:          make([]*Shard, numShards),
		cluster:         cluster,
	}

	// init shards
	for idx := uint32(0); idx < numShards; idx++ {
		sm.LastShardID++
		shard := Shard{
			ShardID:        sm.LastShardID,
			NumReplicas:    0,
			PrimaryReplica: 0,
			Replicas:       make(map[uint64]*Replica),
			smap:           &sm,
		}
		sm.Shards[idx] = &shard
	}

	return &sm, nil
}

// computeHashForPoint returns a CRC32 hash of db + measurement
func (sm *ShardMap) computeHashForPoint(keys string) uint32 {
	crc32q := crc32.MakeTable(0xD5828281)
	return crc32.Checksum([]byte(keys), crc32q)
}

// GetShardForPoint returns a shard for the measurement
func (sm *ShardMap) GetShardForPoint(kind, measurement, tags string) (*Shard, error) {
	var hash uint32

	// calculate the hash
	if measurement == "Fwlogs" {
		hash = sm.computeHashForPoint(kind + "|" + measurement + "|" + tags)
	} else {
		hash = sm.computeHashForPoint(kind + "|" + measurement)
	}

	// calculate modulo
	mod := hash % sm.NumShards

	// find the shard
	shard := sm.Shards[mod]
	if shard == nil {
		return nil, errors.New("Shard not found")
	}

	return shard, nil
}

// GetShardFromID returns a shard from id
func (sm *ShardMap) GetShardFromID(shardNum int) (*Shard, error) {
	// calculate modulo
	mod := uint32(shardNum) % sm.NumShards

	// find the shard
	shard := sm.Shards[mod]
	if shard == nil {
		return nil, errors.New("shard not found")
	}

	return shard, nil
}

// computeHashForPoint returns a CRC32 hash of db + measurement
func (sm *ShardMap) computeHashForKey(table string, key tproto.Key) uint32 {
	crc32q := crc32.MakeTable(0xD5828281)
	return crc32.Checksum(append([]byte(table), key.Key...), crc32q)
}

// GetShardForKey returns the shard for the key
func (sm *ShardMap) GetShardForKey(table string, key tproto.Key) (*Shard, error) {
	// calculate the hash
	hash := sm.computeHashForKey(table, key)

	// calculate modulo
	mod := hash % sm.NumShards

	// find the shard
	shard := sm.Shards[mod]
	if shard == nil {
		return nil, errors.New("Shard not found")
	}

	return shard, nil
}

// Write save the shard map to kvstore
func (sm *ShardMap) Write() error {
	return sm.cluster.mclient.WriteMetadata(sm.cluster)
}

// FindShardByID returns a shard by id
func (sm *ShardMap) FindShardByID(shardID uint64) (*Shard, error) {
	// walk all shards and find it
	for _, shard := range sm.Shards {
		if shard.ShardID == shardID {
			return shard, nil
		}
	}

	return nil, fmt.Errorf("Could not find shard %d", shardID)
}

// NewReplica creates a new replica
func (sm *ShardMap) NewReplica(nodeUUID string) (*Replica, error) {
	// increment replica id running counter
	sm.LastReplicaID++

	// create replica
	shard := Replica{
		ReplicaID: sm.LastReplicaID,
		NodeUUID:  nodeUUID,
		State:     ReplicaStateInit,
	}

	// write shard map
	err := sm.cluster.mclient.WriteMetadata(sm.cluster)
	if err != nil {
		return nil, err
	}

	return &shard, nil
}

// GetReplica finds a replica by id
func (sm *ShardMap) GetReplica(replID uint64) (*Replica, error) {
	// walk all shards
	for _, shard := range sm.Shards {
		// walk all replicas in the shard
		for _, shard := range shard.Replicas {
			if shard.ReplicaID == replID {
				return shard, nil
			}
		}
	}

	return nil, errors.New("Shard not found")
} // NewShardMap creates a new shard map instance
