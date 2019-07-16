// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"reflect"
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
	case kvstore.Created:
		log.Infof("SetNodeState - %s\n", node.Name)
		err := sm.SetNodeState(node)
		if err != nil {
			log.Errorf("Error SetNodeState Node {%+v}. Err: %v", node, err)
			return
		}
	case kvstore.Updated:
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
		return fmt.Errorf("unexpected object kind %s", n.GetObjectKind())
	}
	var nodeState *NodeState

	// print the Log only when the health changes or if object is created
	printMsg := false

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
		printMsg = true
	}

	nodeState.Mutex.Lock()

	if !printMsg {
		if len(nodeState.Status.Conditions) != len(n.Status.Conditions) {
			printMsg = true
		}
		if len(n.Status.Conditions) > 0 && len(nodeState.Status.Conditions) > 0 &&
			nodeState.Status.Conditions[0].Status != n.Status.Conditions[0].Status {
			printMsg = true
		}
		if !printMsg && !reflect.DeepEqual(n.Labels, nodeState.Labels) {
			printMsg = true
		}
	}
	if printMsg {
		if len(n.Status.Conditions) > 0 {
			log.Infof("SetNodeState - %s Labels:%s Condition[%s]=%s\n", n.Name, n.Labels, n.Status.Conditions[0].Type, n.Status.Conditions[0].Status)
		} else {
			log.Infof("SetNodeState - %s Labels:%s Nil Conditions\n", n.Name, n.Labels)
		}
	}

	nodeState.Node = n
	nodeState.Mutex.Unlock()

	sm.memDB.AddObject(nodeState)
	return nil
}

// NodeStateFromObj converts from memdb object to Node state
func NodeStateFromObj(obj memdb.Object) (*NodeState, error) {
	switch nsobj := obj.(type) {
	case *NodeState:
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
	// TODO: may be set state to deleted and leave it db till all the watchers have come to reasonable state

	// delete node state from DB
	_ = sm.memDB.DeleteObject(nodeState)
}
