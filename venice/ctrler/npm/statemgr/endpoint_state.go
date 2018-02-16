// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"sync"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// EndpointState is a wrapper for endpoint object
type EndpointState struct {
	network.Endpoint                                // embedding endpoint object
	sync.Mutex                                      // lock for the ep object
	groups           map[string]*SecurityGroupState // list of security groups
	stateMgr         *Statemgr                      // state manager
}

// endpointKey returns endpoint key
func (eps *EndpointState) endpointKey() string {
	return eps.ObjectMeta.Name
}

// EndpointStateFromObj conerts from memdb object to endpoint state
func EndpointStateFromObj(obj memdb.Object) (*EndpointState, error) {
	switch obj.(type) {
	case *EndpointState:
		epobj := obj.(*EndpointState)
		return epobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// AttributeExists returns true if an attribute is found
func (eps *EndpointState) AttributeExists(matchAttr string) bool {
	// walk all attributes and see if it matches
	for _, attr := range eps.Status.WorkloadAttributes {
		if attr == matchAttr {
			return true
		}
	}

	return false
}

// MatchAttributes returns true if all attributes are found
func (eps *EndpointState) MatchAttributes(attrs []string) bool {
	// walk all attributes and confirm all of them match
	for _, attr := range attrs {
		if !eps.AttributeExists(attr) {
			return false
		}
	}

	return true
}

// AddSecurityGroup adds a security group to an endpoint
func (eps *EndpointState) AddSecurityGroup(sgs *SecurityGroupState) error {
	// lock the endpoint
	eps.Lock()
	defer eps.Unlock()

	// add security group to the list
	eps.Status.SecurityGroups = append(eps.Status.SecurityGroups, sgs.Name)
	eps.groups[sgs.Name] = sgs

	// save it to api server
	err := eps.Write(false)
	if err != nil {
		log.Errorf("Error writing the endpoint state to api server. Err: %v", err)
		return err
	}

	return eps.stateMgr.memDB.UpdateObject(eps)
}

// DelSecurityGroup removes a security group from an endpoint
func (eps *EndpointState) DelSecurityGroup(sgs *SecurityGroupState) error {
	// lock the endpoint
	eps.Lock()
	defer eps.Unlock()

	// remove the security group from the list
	for i, sgname := range eps.Status.SecurityGroups {
		if sgname == sgs.Name {
			if i < (len(eps.Status.SecurityGroups) - 1) {
				eps.Status.SecurityGroups = append(eps.Status.SecurityGroups[:i], eps.Status.SecurityGroups[i+1:]...)
			} else {
				eps.Status.SecurityGroups = eps.Status.SecurityGroups[:i]
			}
		}
	}
	delete(eps.groups, sgs.Name)

	return eps.stateMgr.memDB.UpdateObject(eps)
}

// attachSecurityGroups attach all security groups
func (eps *EndpointState) attachSecurityGroups() error {
	// get a list of security groups
	sgs, err := eps.stateMgr.ListSecurityGroups()
	if err != nil {
		log.Errorf("Error getting the list of security groups. Err: %v", err)
		return err
	}

	// walk all sgs and see if endpoint matches the selector
	for _, sg := range sgs {
		if eps.MatchAttributes(sg.Spec.WorkloadSelector) {
			err = sg.AddEndpoint(eps)
			if err != nil {
				log.Errorf("Error adding ep %s to sg %s. Err: %v", eps.Name, sg.Name, err)
				return err
			}

			// add sg to endpoint
			eps.Status.SecurityGroups = append(eps.Status.SecurityGroups, sg.Name)
			eps.groups[sg.Name] = sg
		}
	}

	return nil
}

// Write writes the object to api server
func (eps *EndpointState) Write(update bool) error {
	return eps.stateMgr.writer.WriteEndpoint(&eps.Endpoint, update)
}

// Delete deletes all state associated with the endpoint
func (eps *EndpointState) Delete() error {
	// detach the endpoint from security group
	for _, sg := range eps.groups {
		err := sg.DelEndpoint(eps)
		if err != nil {
			log.Errorf("Error removing endpoint from sg. Err: %v", err)
		}
	}

	return nil
}

// NewEndpointState returns a new endpoint object
func NewEndpointState(epinfo network.Endpoint, stateMgr *Statemgr) (*EndpointState, error) {
	// build the endpoint state
	eps := EndpointState{
		Endpoint: epinfo,
		groups:   make(map[string]*SecurityGroupState),
		stateMgr: stateMgr,
	}

	// attach security groups
	err := eps.attachSecurityGroups()
	if err != nil {
		log.Errorf("Error attaching security groups to ep %v. Err: %v", eps.Name, err)
		return nil, err
	}

	// save it to api server
	err = eps.Write(false)
	if err != nil {
		log.Errorf("Error writing the endpoint state to api server. Err: %v", err)
		return nil, err
	}

	return &eps, nil
}

// FindEndpoint finds endpointy by name
func (sm *Statemgr) FindEndpoint(tenant, name string) (*EndpointState, error) {
	// find the object
	obj, err := sm.FindObject("Endpoint", tenant, name)
	if err != nil {
		return nil, err
	}

	return EndpointStateFromObj(obj)
}

// ListEndpoints lists all endpoints
func (sm *Statemgr) ListEndpoints() ([]*EndpointState, error) {
	objs := sm.memDB.ListObjects("Endpoint")

	var eps []*EndpointState
	for _, obj := range objs {
		ep, err := EndpointStateFromObj(obj)
		if err != nil {
			return eps, err
		}

		eps = append(eps, ep)
	}

	return eps, nil
}
