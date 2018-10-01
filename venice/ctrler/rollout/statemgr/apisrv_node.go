// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sort"
	"sync"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// NodeState - Internal state for Node
type NodeState struct {
	Mutex sync.Mutex
	*cluster.Node
	*Statemgr

	// Local information
}

type nodeStates []*NodeState

func (s nodeStates) Len() int           { return len(s) }
func (s nodeStates) Less(i, j int) bool { return s[i].Name < s[j].Name }
func (s nodeStates) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }

func (sm *Statemgr) handleNodeEvent(et kvstore.WatchEventType, node *cluster.Node) {
	switch et {
	case kvstore.Created, kvstore.Updated:
		log.Infof("SetNodeState - %s\n", node.Name)
		err := sm.SetNodeState(node)
		if err != nil {
			log.Errorf("Error SetNodeState Node {%+v}. Err: %v", node, err)
			return
		}
	case kvstore.Deleted:
		log.Infof("DeleteNodeState - %s\n", node.Name)
		sm.DeleteNodeState(node)
	}
}

// SetNodeState to create a Node Object/update node if it already exists in statemgr
func (sm *Statemgr) SetNodeState(n *cluster.Node) error {
	if n.GetObjectKind() != kindNode {
		return fmt.Errorf("Unexpected object kind %s", n.GetObjectKind())
	}
	var nodeState *NodeState

	// All parameters are validated (using apiserver hooks) by the time we get here
	obj, err := sm.FindObject(kindNode, n.Tenant, n.Name)
	if err == nil {
		nodeState, err = NodeStateFromObj(obj)
		if err != nil {
			return err
		}
	} else {
		nodeState = &NodeState{
			Statemgr: sm,
		}
	}

	nodeState.Mutex.Lock()
	nodeState.Node = n
	nodeState.Mutex.Unlock()

	sm.memDB.AddObject(nodeState)
	return nil
}

// NodeStateFromObj converts from memdb object to Node state
func NodeStateFromObj(obj memdb.Object) (*NodeState, error) {
	switch obj.(type) {
	case *NodeState:
		nsobj := obj.(*NodeState)
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// ListNodes lists all Node objects
func (sm *Statemgr) ListNodes() ([]*NodeState, error) {
	objs := sm.memDB.ListObjects(kindNode)

	var ns []*NodeState
	for _, obj := range objs {
		nso, err := NodeStateFromObj(obj)
		if err != nil {
			return ns, err
		}
		ns = append(ns, nso)
	}
	sort.Sort(nodeStates(ns))

	return ns, nil
}

// GetNodeState : Get the specified node state
func (sm *Statemgr) GetNodeState(tenant, name string) (*NodeState, error) {
	dbMs, err := sm.FindObject(kindNode, tenant, name)
	if err != nil {
		return nil, err
	}
	return dbMs.(*NodeState), nil
}

// DeleteNodeState - delete node
func (sm *Statemgr) DeleteNodeState(node *cluster.Node) {
	nodeState, err := sm.GetNodeState(node.Tenant, node.Name)
	if err != nil {
		log.Debugf("Error deleting non-existent node {%+v}. Err: %v", node, err)
		return
	}

	log.Infof("Deleting Node %v", nodeState.Node.Name)
	nodeState.Mutex.Lock()
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state
	nodeState.Mutex.Unlock()

	// delete node state from DB
	_ = sm.memDB.DeleteObject(nodeState)
}
