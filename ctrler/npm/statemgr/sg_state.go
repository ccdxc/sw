// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/ctrler/npm/writer"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/memdb"
)

// SecurityGroupState security group state
type SecurityGroupState struct {
	network.SecurityGroup                           // embedded security group object
	policies              map[string]*SgpolicyState // list of policies attached to this security group
	endpoints             map[string]*EndpointState // list of all matching endpoints in this security group
	stateMgr              *Statemgr                 // pointer to network manager
}

// Write writes the object to api server
func (sg *SecurityGroupState) Write() error {
	return writer.WriteObject(sg)
}

// Delete cleans up all state associated with the sg
func (sg *SecurityGroupState) Delete() error {
	// check if sg still has policies
	if len(sg.policies) != 0 {
		log.Errorf("Can not delete the sg, still has policies attached {%+v}", sg)
		return fmt.Errorf("Security group still has policies")
	}

	return nil
}

// attachEndpoints finds all the matching endpoints based on selector
func (sg *SecurityGroupState) attachEndpoints() error {
	// get a list of all endpoints
	eps, err := sg.stateMgr.ListEndpoints()
	if err != nil {
		log.Errorf("Error getting endpoint list. Err: %v", err)
		return err
	}

	// walk the endpoints and see which ones match
	for _, ep := range eps {
		if ep.Tenant == sg.Tenant {
			if ep.MatchAttributes(sg.Spec.WorkloadSelector) {
				err = ep.AddSecurityGroup(sg)
				if err != nil {
					log.Errorf("Error attaching endpoint %s to sg %s. Err: %v", ep.Name, sg.Name, err)
					return err
				}

				// add the ep to local list
				sg.endpoints[ep.Name] = ep
				sg.Status.Workloads = append(sg.Status.Workloads, ep.Name)
			}

		}
	}

	return nil
}

// AddEndpoint adds an endpoint to sg
func (sg *SecurityGroupState) AddEndpoint(ep *EndpointState) error {
	// add the ep to local list
	sg.endpoints[ep.Name] = ep
	sg.Status.Workloads = append(sg.Status.Workloads, ep.Name)

	return sg.stateMgr.memDB.UpdateObject(sg)
}

// DelEndpoint removes an endpoint from sg
func (sg *SecurityGroupState) DelEndpoint(ep *EndpointState) error {
	// delete the endpoint
	delete(sg.endpoints, ep.Name)
	for i, epname := range sg.Status.Workloads {
		if epname == ep.Name {
			sg.Status.Workloads = append(sg.Status.Workloads[:i], sg.Status.Workloads[i+1:]...)
		}
	}

	return sg.stateMgr.memDB.UpdateObject(sg)
}

// AddPolicy adds a policcy to sg
func (sg *SecurityGroupState) AddPolicy(sgp *SgpolicyState) error {
	// add to db
	sg.policies[sgp.Name] = sgp
	sg.Status.Policies = append(sg.Status.Policies, sgp.Name)

	// trigger an update
	return sg.stateMgr.memDB.UpdateObject(sg)
}

// DeletePolicy deletes a policcy from sg
func (sg *SecurityGroupState) DeletePolicy(sgp *SgpolicyState) error {
	// delete from db
	delete(sg.policies, sgp.Name)
	for i, pname := range sg.Status.Policies {
		if pname == sgp.Name {
			sg.Status.Policies = append(sg.Status.Policies[:i], sg.Status.Policies[i+1:]...)
		}
	}
	// trigger an update
	return sg.stateMgr.memDB.UpdateObject(sg)
}

// SecurityGroupStateFromObj conerts from memdb object to network state
func SecurityGroupStateFromObj(obj memdb.Object) (*SecurityGroupState, error) {
	switch obj.(type) {
	case *SecurityGroupState:
		sgobj := obj.(*SecurityGroupState)
		return sgobj, nil
	default:
		return nil, errors.New("Incorrect object type")
	}
}

// NewSecurityGroupState creates a new security group state object
func NewSecurityGroupState(sg *network.SecurityGroup, stateMgr *Statemgr) (*SecurityGroupState, error) {
	// create sg state object
	sgs := SecurityGroupState{
		SecurityGroup: *sg,
		policies:      make(map[string]*SgpolicyState),
		endpoints:     make(map[string]*EndpointState),
		stateMgr:      stateMgr,
	}

	return &sgs, nil
}

// FindSecurityGroup finds security group by name
func (sm *Statemgr) FindSecurityGroup(tenant, name string) (*SecurityGroupState, error) {
	// find the object
	obj, err := sm.FindObject("SecurityGroup", tenant, name)
	if err != nil {
		return nil, err
	}

	return SecurityGroupStateFromObj(obj)
}

// ListSecurityGroups lists all security groups
func (sm *Statemgr) ListSecurityGroups() ([]*SecurityGroupState, error) {
	objs := sm.memDB.ListObjects("SecurityGroup")

	var sgs []*SecurityGroupState
	for _, obj := range objs {
		sg, err := SecurityGroupStateFromObj(obj)
		if err != nil {
			return sgs, err
		}

		sgs = append(sgs, sg)
	}

	return sgs, nil
}

// CreateSecurityGroup creates a security group
func (sm *Statemgr) CreateSecurityGroup(sg *network.SecurityGroup) error {
	// see if we already have it
	esg, err := sm.FindObject("SecurityGroup", sg.ObjectMeta.Tenant, sg.ObjectMeta.Name)
	if err == nil {
		// FIXME: how do we handle an existing sg object changing?
		log.Errorf("Can not change existing sg {%+v}. New state: {%+v}", esg, sg)
		return fmt.Errorf("Can not change sg after its created")
	}

	// create new sg state
	sgs, err := NewSecurityGroupState(sg, sm)
	if err != nil {
		log.Errorf("Error creating new sg state. Err: %v", err)
		return err
	}

	// attach all matching endpoints
	err = sgs.attachEndpoints()
	if err != nil {
		log.Errorf("Error attaching endpoints to security group %s. Err: %v", sgs.Name, err)
		return err
	}

	log.Infof("Created Security Group state {%+v}", sgs)

	// store it in local DB
	return sm.memDB.AddObject(sgs)
}

// DeleteSecurityGroup deletes a security group
func (sm *Statemgr) DeleteSecurityGroup(tenant, sgname string) error {
	// see if we already have it
	sgo, err := sm.FindObject("SecurityGroup", tenant, sgname)
	if err != nil {
		log.Errorf("Can not find the sg %s|%s", tenant, sgname)
		return fmt.Errorf("SecurityGroup not found")
	}

	// convert it to security group state
	sg, err := SecurityGroupStateFromObj(sgo)
	if err != nil {
		return err
	}

	// walk all endpoints and remove the sg
	for _, ep := range sg.endpoints {
		err = ep.DelSecurityGroup(sg)
		if err != nil {
			log.Errorf("Error removing security group %s from endpoint %s. Err: %v", sg.Name, ep.Name, err)
		}
	}

	// cleanup sg state
	err = sg.Delete()
	if err != nil {
		log.Errorf("Error deleting the sg {%+v}. Err: %v", sg, err)
		return err
	}

	// delete it from the DB
	return sm.memDB.DeleteObject(sg)
}
