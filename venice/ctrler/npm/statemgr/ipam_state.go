// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"errors"
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

// IPAMState is a wrapper for ipam policy object
type IPAMState struct {
	sync.Mutex
	IPAMPolicy *ctkit.IPAMPolicy `json:"-"` // IPAMPolicy object
	stateMgr   *Statemgr         // pointer to the network manager
}

// IPAMPolicyStateFromObj converts from memdb object to IPAMPoliocy state
func IPAMPolicyStateFromObj(obj runtime.Object) (*IPAMState, error) {
	switch obj.(type) {
	case *ctkit.IPAMPolicy:
		policy := obj.(*ctkit.IPAMPolicy)
		switch policy.HandlerCtx.(type) {
		case *IPAMState:
			state := policy.HandlerCtx.(*IPAMState)
			return state, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// FindIPAMPolicy finds IPAM policy by name
func (sm *Statemgr) FindIPAMPolicy(tenant, ns, name string) (*IPAMState, error) {
	// find the object
	obj, err := sm.FindObject("IPAMPolicy", tenant, ns, name)
	if err != nil {
		return nil, err
	}

	return IPAMPolicyStateFromObj(obj)
}

//GetIPAMPolicyWatchOptions gets options
func (sm *Statemgr) GetIPAMPolicyWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	return &opts
}

func convertIPAMPolicy(ipam *IPAMState) *netproto.IPAMPolicy {
	server := &netproto.DHCPServer{}
	obj := &netproto.IPAMPolicy{
		TypeMeta:   ipam.IPAMPolicy.TypeMeta,
		ObjectMeta: agentObjectMeta(ipam.IPAMPolicy.ObjectMeta),
	}

	obj.Spec = netproto.IPAMPolicySpec{}
	obj.Spec.DHCPRelay = &netproto.DHCPRelayPolicy{}
	for _, srv := range ipam.IPAMPolicy.Spec.DHCPRelay.Servers {
		log.Error("convertIPAMPolicy: dhcp server info: ", srv)
		server.IPAddress = srv.IPAddress
		server.VirtualRouter = srv.VirtualRouter
		obj.Spec.DHCPRelay.Servers = append(obj.Spec.DHCPRelay.Servers, server)
	}
	//obj.Spec.DHCPRelay.Servers = servers
	log.Error("convertIPAMPolicy: returning:  ", *obj)
	return obj
}

// NewIPAMPolicyState creates a new IPAMState
func NewIPAMPolicyState(policy *ctkit.IPAMPolicy, sm *Statemgr) (*IPAMState, error) {
	ipam := &IPAMState{
		IPAMPolicy: policy,
		stateMgr:   sm,
	}
	policy.HandlerCtx = ipam
	return ipam, nil
}

// OnIPAMPolicyCreate creates local network state based on watch event
func (sm *Statemgr) OnIPAMPolicyCreate(obj *ctkit.IPAMPolicy) error {
	log.Info("OnIPAMPolicyCreate: received: ", obj.Spec)

	// create new network state
	ipam, err := NewIPAMPolicyState(obj, sm)
	if err != nil {
		log.Errorf("Error creating IPAM policy state. Err: %v", err)
		return err
	}

	// store it in local DB
	sm.mbus.AddObject(convertIPAMPolicy(ipam))

	return nil
}

// OnIPAMPolicyUpdate handles IPAMPolicy update
func (sm *Statemgr) OnIPAMPolicyUpdate(oldpolicy *ctkit.IPAMPolicy, newpolicy *network.IPAMPolicy) error {
	log.Info("OnIPAMPolicyUpdate: received: ", oldpolicy.Spec, newpolicy.Spec)

	// see if anything changed
	_, ok := ref.ObjDiff(oldpolicy.Spec, newpolicy.Spec)
	if (oldpolicy.GenerationID == newpolicy.GenerationID) && !ok {
		oldpolicy.ObjectMeta = newpolicy.ObjectMeta
		return nil
	}

	// update old state
	oldpolicy.ObjectMeta = newpolicy.ObjectMeta
	oldpolicy.Spec = newpolicy.Spec

	// find the policy state
	policy, err := IPAMPolicyStateFromObj(oldpolicy)
	if err != nil {
		log.Errorf("Can find an IPAM policy for updating {%+v}. Err: {%v}", oldpolicy.ObjectMeta, err)
		return fmt.Errorf("Can not find IPAM policy")
	}

	// store it in local DB
	err = sm.mbus.UpdateObject(convertIPAMPolicy(policy))
	if err != nil {
		log.Errorf("Error storing the IPAM policy in memdb. Err: %v", err)
		return err
	}

	return nil
}

// OnIPAMPolicyDelete deletes the IPAMPolicy
func (sm *Statemgr) OnIPAMPolicyDelete(obj *ctkit.IPAMPolicy) error {
	log.Info("OnIPAMPolicyDelete: received: ", obj.Spec)

	policy, err := sm.FindIPAMPolicy(obj.Tenant, obj.Namespace, obj.Name)

	if err != nil {
		log.Error("FindIPAMPolicy returned an error: ", err, "for: ", obj.Tenant, obj.Namespace, obj.Name)
		return errors.New("Object doesn't exist")
	}

	// delete it from the DB
	return sm.mbus.DeleteObject(convertIPAMPolicy(policy))
}
