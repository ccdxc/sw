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
	"github.com/pensando/sw/venice/utils/featureflags"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

var smgrIPAM *SmIPAM

// SmIPAM is object statemgr for IPAM object
type SmIPAM struct {
	featureMgrBase
	sm *Statemgr
}

// CompleteRegistration is the callback function statemgr calls after init is done
func (sma *SmIPAM) CompleteRegistration() {

	if featureflags.IsOVerlayRoutingEnabled() == false {
		return
	}

	sma.sm.SetIPAMPolicyReactor(smgrIPAM)
}

func init() {
	mgr := MustGetStatemgr()
	smgrIPAM = &SmIPAM{
		sm: mgr,
	}

	mgr.Register("statemgripam", smgrIPAM)
}

// IPAMState is a wrapper for ipam policy object
type IPAMState struct {
	sync.Mutex
	IPAMPolicy *ctkit.IPAMPolicy `json:"-"` // IPAMPolicy object
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
			return nil, errors.New("incorrect object type")
		}
	default:
		return nil, errors.New("incorrect object type")
	}
}

// FindIPAMPolicy finds IPAM policy by name
func (sma *SmIPAM) FindIPAMPolicy(tenant, ns, name string) (*IPAMState, error) {
	// find it in db
	obj, err := sma.sm.FindObject("IPAMPolicy", tenant, ns, name)
	if err != nil {
		return nil, err
	}

	return IPAMPolicyStateFromObj(obj)
}

//GetIPAMPolicyWatchOptions gets options
func (sma *SmIPAM) GetIPAMPolicyWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	return &opts
}

func convertIPAMPolicy(ipam *IPAMState) *netproto.IPAMPolicy {
	server := &netproto.DHCPServer{}

	meta := api.ObjectMeta{
		Tenant:          ipam.IPAMPolicy.Tenant,
		Namespace:       ipam.IPAMPolicy.Namespace,
		Name:            ipam.IPAMPolicy.Name,
		GenerationID:    ipam.IPAMPolicy.GenerationID,
		ResourceVersion: ipam.IPAMPolicy.ResourceVersion,
		UUID:            ipam.IPAMPolicy.UUID,
	}

	obj := &netproto.IPAMPolicy{
		TypeMeta:   ipam.IPAMPolicy.TypeMeta,
		ObjectMeta: meta,
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
func NewIPAMPolicyState(policy *ctkit.IPAMPolicy, sma *SmIPAM) (*IPAMState, error) {
	ipam := &IPAMState{
		IPAMPolicy: policy,
	}
	policy.HandlerCtx = ipam
	return ipam, nil
}

// OnIPAMPolicyCreate creates local network state based on watch event
func (sma *SmIPAM) OnIPAMPolicyCreate(obj *ctkit.IPAMPolicy) error {
	log.Info("OnIPAMPolicyCreate: received: ", obj.Spec)

	// create new network state
	ipam, err := NewIPAMPolicyState(obj, sma)
	if err != nil {
		log.Errorf("Error creating IPAM policy state. Err: %v", err)
		return err
	}

	// store it in local DB
	sma.sm.AddObject(convertIPAMPolicy(ipam))

	return nil
}

// OnIPAMPolicyUpdate handles IPAMPolicy update
func (sma *SmIPAM) OnIPAMPolicyUpdate(oldpolicy *ctkit.IPAMPolicy, newpolicy *network.IPAMPolicy) error {
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
	err = sma.sm.UpdateObject(convertIPAMPolicy(policy))
	if err != nil {
		log.Errorf("Error storing the IPAM policy in memdb. Err: %v", err)
		return err
	}

	return nil
}

// OnIPAMPolicyDelete deletes the IPAMPolicy
func (sma *SmIPAM) OnIPAMPolicyDelete(obj *ctkit.IPAMPolicy) error {
	log.Info("OnIPAMPolicyDelete: received: ", obj.Spec)

	policy, err := sma.FindIPAMPolicy(obj.Tenant, obj.Namespace, obj.Name)

	if err != nil {
		log.Error("FindIPAMPolicy returned an error: ", err, "for: ", obj.Tenant, obj.Namespace, obj.Name)
		return errors.New("Object doesn't exist")
	}

	// delete it from the DB
	return sma.sm.DeleteObject(convertIPAMPolicy(policy))
}
