// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cache

import (
	"context"
	"errors"
	"fmt"
	"sync"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// NodeState security policy state
type NodeState struct {
	*sync.Mutex
	*cluster.Node      // node object
	dirty         bool // dirty is true when there are uncommitted updates to apiserver
}

// NodeStateFromObj converts from memdb object to Node state
func NodeStateFromObj(obj memdb.Object) (*NodeState, error) {
	switch obj.(type) {
	case *NodeState:
		snobj := obj.(*NodeState)
		return snobj, nil

	default:
		return nil, errors.New("Incorrect object type")
	}
}

// NewNodeState creates a new security policy state object
func NewNodeState(sn *cluster.Node) (*NodeState, error) {
	// create node state object
	sns := NodeState{
		Node:  sn,
		Mutex: new(sync.Mutex),
	}
	return &sns, nil
}

// FindNode finds Node object by name
func (sm *Statemgr) FindNode(name string) (*NodeState, error) {
	// find the object
	obj, err := sm.FindObject("Node", "", name)
	if err != nil {
		return nil, err
	}
	return NodeStateFromObj(obj)
}

// ListNodes lists all Node objects
func (sm *Statemgr) ListNodes() ([]*NodeState, error) {
	var sgs []*NodeState
	objs := sm.memDB.ListObjects("Node", nil)
	for _, obj := range objs {
		sg, err := NodeStateFromObj(obj)
		if err != nil {
			return sgs, err
		}

		sgs = append(sgs, sg)
	}
	return sgs, nil
}

// CreateNode creates a Node object
func (sm *Statemgr) CreateNode(node *cluster.Node) (*NodeState, error) {
	// see if we already have it
	ehs, err := sm.FindNode(node.ObjectMeta.Name)
	if err == nil {
		// Object exists in cache, but we got a watcher event with event-type:Created
		// and this can happen if there is a watcher error/reset and we need to update
		// the cache to handle it gracefully as an Update.
		log.Infof("Objects exists, updating Node OldState: {%+v}. New state: {%+v}", ehs, node)
		ehs.Lock()
		defer ehs.Unlock()
		return ehs, sm.UpdateNode(node, false)
	}

	// create new Node state
	nodeState, err := NewNodeState(node)
	if err != nil {
		log.Errorf("Error creating new Node state. Err: %v", err)
		return nil, err
	}

	// store it in local DB
	err = sm.memDB.AddObject(nodeState)
	if err != nil {
		log.Errorf("Error storing the Node state in memdb. Err: %v", err)
		return nil, err
	}

	// Node creates are never written back to ApiServer
	log.Infof("Created Node state {%+v}", node.ObjectMeta)
	return nodeState, nil
}

// UpdateNode updates a Node object
// Caller is responsible for acquiring the lock before invocation and releasing it afterwards
func (sm *Statemgr) UpdateNode(node *cluster.Node, writeback bool) error {
	obj, err := sm.FindObject("Node", "", node.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Can not find the Node %s err: %v", node.ObjectMeta.Name, err)
		return fmt.Errorf("Node not found")
	}

	nodeState, err := NodeStateFromObj(obj)
	if err != nil {
		log.Errorf("Wrong object type in memdb! Expected Node, got %T", obj)
		return err
	}
	nodeState.Node = node

	// store it in local DB
	err = sm.memDB.UpdateObject(nodeState)
	if err != nil {
		log.Errorf("Error storing Node in memdb. Err: %v", err)
		return err
	}

	if writeback || nodeState.dirty {
		nodeObj := node
		ok := false
		for i := 0; i < maxAPIServerWriteRetries; i++ {
			ctx, cancel := context.WithTimeout(context.Background(), apiServerRPCTimeout)
			_, err = sm.APIClient().Node().Update(ctx, nodeObj)
			if err == nil {
				ok = true
				nodeState.dirty = false
				log.Infof("Updated Node object in ApiServer: %+v", nodeObj)
				cancel()
				break
			}
			log.Errorf("Error updating Node object %+v: %v", nodeObj.ObjectMeta, err)
			// Write error -- fetch updated Spec + Meta and retry
			updObj, err := sm.APIClient().Node().Get(ctx, &nodeObj.ObjectMeta)
			if err == nil {
				// retain Status as that's what we are trying to update
				updObj.Status = nodeObj.Status
				nodeObj = updObj
			}
			cancel()
		}
		if !ok {
			nodeState.dirty = true
			log.Errorf("Error updating Node object %+v in ApiServer, retries exhausted", nodeObj.ObjectMeta)
		}
	}

	log.Debugf("Updated Node state {%+v}", node)
	return nil
}

// DeleteNode deletes a Node state
func (sm *Statemgr) DeleteNode(node *cluster.Node) error {
	// delete it from the DB
	err := sm.memDB.DeleteObject(node)
	if err != nil {
		log.Errorf("Error deleting Node state %+v: %v", node.ObjectMeta, err)
		return err
	}

	// Node deletes are never written back to ApiServer
	log.Infof("Deleted Node state {%+v}", node.ObjectMeta)
	return nil
}
