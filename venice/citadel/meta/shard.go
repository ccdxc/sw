// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package meta

import (
	"errors"

	"github.com/pensando/sw/venice/utils/log"
)

// AddReplicas adds replicas to shard
func (shard *Shard) AddReplicas(replicas []*Replica) error {
	// verify shards are not on same node
	for _, repl := range replicas {
		for _, se := range replicas {
			if se.ReplicaID != repl.ReplicaID && se.NodeUUID == repl.NodeUUID {
				log.Errorf("Shard %d: Replicas %+v and %+v are on same node", repl.ShardID, repl, se)
				return errors.New("Replicas on same node")
			}
		}
	}

	// If shard has no primary, we have no data to sync
	if shard.PrimaryReplica == 0 {
		for idx, repl := range replicas {
			repl.shard = shard
			repl.ShardID = shard.ShardID
			repl.State = ReplicaStateReady
			shard.Replicas[repl.ReplicaID] = repl

			// set first shard as the primary
			if idx == 0 {
				shard.PrimaryReplica = repl.ReplicaID
				repl.IsPrimary = true
				err := shard.smap.cluster.mclient.SetPrimaryReplica(repl)
				if err != nil {
					log.Errorf("Error setting shard %+v as primary. Err: %v", shard, err)
					delete(shard.Replicas, repl.ReplicaID)
					return err
				}
			}
		}

		// set num replicas
		shard.NumReplicas = uint32(len(replicas))

		// write the shard map
		return shard.smap.Write()
	}

	// add each shard as secondary
	for _, repl := range replicas {
		err := shard.AddSecondaryReplica(repl)
		if err != nil {
			return err
		}
	}

	return nil
}

// AddSecondaryReplica adds non-primary replica
// 1. set new replica as candidate
// 2. copy data from primary replica to candidate
// 3. mark the replica as ready
func (shard *Shard) AddSecondaryReplica(repl *Replica) error {
	// verify this shard is not on same node as other shards
	for _, sr := range shard.Replicas {
		if sr.NodeUUID == repl.NodeUUID {
			log.Errorf("Shard %d: Replica %+v is on same node as existing shard %+v", shard.ShardID, repl, sr)
			return errors.New("Replicas on same node")
		}
	}

	// set shard state
	repl.State = ReplicaStateReady
	repl.ShardID = shard.ShardID
	repl.shard = shard
	shard.Replicas[repl.ReplicaID] = repl
	shard.NumReplicas++

	// get primary shard
	prepl, err := shard.GetPrimaryreplica()
	if err != nil {
		// remove the replica from the shard
		delete(shard.Replicas, repl.ReplicaID)
		shard.NumReplicas--
		return err
	}

	// copy data from primary to secondary
	err = shard.smap.cluster.mclient.CopyDataFromPrimaryReplica(prepl, repl)
	if err != nil {
		log.Errorf("Error copying data from primary replica %+v to secondary %+v. Err: %v", prepl, repl, err)

		// remove the replica from the shard
		delete(shard.Replicas, repl.ReplicaID)
		shard.NumReplicas--
		return err
	}

	// write the shardmap
	return shard.smap.Write()
}

// ReplicaUnreachable marks the replica as unreachble. If this is a primary replica, it'll elect a new primary
func (shard *Shard) ReplicaUnreachable(repl *Replica) error {
	// change replica state
	repl.State = ReplicaStateUnreachable
	repl.IsPrimary = false

	// check if this is the primary replica
	if shard.PrimaryReplica == repl.ReplicaID {
		// pick one of the remaining replicas as primary
		for _, se := range shard.Replicas {
			if se.ReplicaID != repl.ReplicaID && se.State == ReplicaStateReady {
				shard.PrimaryReplica = se.ReplicaID
				se.IsPrimary = true
				err := shard.smap.cluster.mclient.SetPrimaryReplica(se)
				if err != nil {
					log.Errorf("Error setting shard %+v as primary. Err: %v", se, err)
					return err
				}

				// write the shardmap
				return shard.smap.Write()
			}
		}

		// if we reached here, we did not find any replica to make a primary
		// FIXME: what should we do if shard has no primary
		shard.PrimaryReplica = 0
	}

	// write the shardmap
	return shard.smap.Write()
}

// ReplicaActivate marks a replica as back alive and copies data from primary
func (shard *Shard) ReplicaActivate(repl *Replica) error {
	// if this shard had no primary replica, this becomes the primary replica
	if shard.PrimaryReplica == 0 {
		// change replica state
		repl.State = ReplicaStateReady
		shard.PrimaryReplica = repl.ReplicaID
		repl.IsPrimary = true

		// write the shardmap
		return shard.smap.Write()
	}

	// find the primary replica for the node
	prepl, ok := shard.Replicas[shard.PrimaryReplica]
	if !ok {
		log.Errorf("Could not find the primary replica for shard %+v", shard)
		return errors.New("Primary replica not found")
	}

	// sync data from primary. We do this as a background task since it could take a while to complete
	err := shard.smap.cluster.mclient.CopyDataFromPrimaryReplica(prepl, repl)
	if err != nil {
		log.Errorf("Error copying data from primary shard to secondary shard")
		return err
	}

	// change replica state
	repl.State = ReplicaStateReady

	// write the shardmap
	return shard.smap.Write()
}

// RemoveReplica removes a replica from the shard
func (shard *Shard) RemoveReplica(repl *Replica) error {
	// if replica was in ready state, mark it down first
	if repl.State == ReplicaStateReady {
		err := shard.ReplicaUnreachable(repl)
		if err != nil {
			return err
		}
	}

	// double check this is not the primary replica
	if shard.PrimaryReplica == repl.ReplicaID {
		log.Errorf("Replica %+v is still primary replica for shard %+v", repl, shard)
		return errors.New("Replica is still primary replica")
	}

	// remove the shard
	delete(shard.Replicas, repl.ReplicaID)
	shard.NumReplicas--

	// write the shardmap
	return shard.smap.Write()
}

// GetPrimaryreplica returns the primary replica
func (shard *Shard) GetPrimaryreplica() (*Replica, error) {
	repl, ok := shard.Replicas[shard.PrimaryReplica]
	if !ok {
		return nil, errors.New("No primary replica")
	}

	return repl, nil
}

// HasReplicaOnNode checkf if the shard has any replica on the node
func (shard *Shard) HasReplicaOnNode(nodeUUID string) bool {
	for _, repl := range shard.Replicas {
		if repl.NodeUUID == nodeUUID {
			return true
		}
	}

	return false
}
