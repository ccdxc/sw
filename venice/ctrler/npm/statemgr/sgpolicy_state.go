// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// SgpolicyState security policy state
type SgpolicyState struct {
	network.Sgpolicy                                // embedded security policy object
	groups           map[string]*SecurityGroupState // list of groups this policy is attached to
	stateMgr         *Statemgr                      // pointer to state manager
}

// Write writes the object to api server
func (sgp *SgpolicyState) Write() error {
	return sgp.stateMgr.writer.WriteSgPolicy(&sgp.Sgpolicy)
}

// Delete cleans up all state associated with the sg
func (sgp *SgpolicyState) Delete() error {
	// inform all SGs to remove the policy
	for _, sg := range sgp.groups {
		err := sg.DeletePolicy(sgp)
		if err != nil {
			log.Errorf("Error deleting policy %s from sg %s. Err: %v", sgp.Name, sg.Name, err)
		}
	}

	return nil
}

// updateAttachedSgs update all attached sgs
func (sgp *SgpolicyState) updateAttachedSgs() error {
	// make sure the attached security group exists
	for _, sgname := range sgp.Spec.AttachGroups {
		sgs, err := sgp.stateMgr.FindSecurityGroup(sgp.Tenant, sgname)
		if err != nil {
			log.Errorf("Could not find the security group %s. Err: %v", sgname, err)
			return err
		}

		// add the policy to sg
		err = sgs.AddPolicy(sgp)
		if err != nil {
			log.Errorf("Error adding policy %s to sg %s. Err: %v", sgp.Name, sgname, err)
			return err
		}

		// link sgpolicy to sg
		sgp.groups[sgs.Name] = sgs
	}

	return nil
}

// SgpolicyStateFromObj converts from memdb object to sgpolicy state
func SgpolicyStateFromObj(obj memdb.Object) (*SgpolicyState, error) {
	switch obj.(type) {
	case *SgpolicyState:
		sgpobj := obj.(*SgpolicyState)
		return sgpobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewSgpolicyState creates a new security policy state object
func NewSgpolicyState(sgp *network.Sgpolicy, stateMgr *Statemgr) (*SgpolicyState, error) {
	// create sg state object
	sgps := SgpolicyState{
		Sgpolicy: *sgp,
		groups:   make(map[string]*SecurityGroupState),
		stateMgr: stateMgr,
	}

	return &sgps, nil
}

// FindSgpolicy finds sg policy by name
func (sm *Statemgr) FindSgpolicy(tenant, name string) (*SgpolicyState, error) {
	// find the object
	obj, err := sm.FindObject("Sgpolicy", tenant, name)
	if err != nil {
		return nil, err
	}

	return SgpolicyStateFromObj(obj)
}

// ListSgpolicies lists all sg policies
func (sm *Statemgr) ListSgpolicies() ([]*SgpolicyState, error) {
	objs := sm.memDB.ListObjects("Sgpolicy")

	var sgs []*SgpolicyState
	for _, obj := range objs {
		sg, err := SgpolicyStateFromObj(obj)
		if err != nil {
			return sgs, err
		}

		sgs = append(sgs, sg)
	}

	return sgs, nil
}

// CreateSgpolicy creates a sg policy
func (sm *Statemgr) CreateSgpolicy(sgp *network.Sgpolicy) error {
	// see if we already have it
	esgp, err := sm.FindObject("Sgpolicy", sgp.ObjectMeta.Tenant, sgp.ObjectMeta.Name)
	if err == nil {
		// FIXME: how do we handle an existing sg object changing?
		log.Errorf("Can not change existing sg policy {%+v}. New state: {%+v}", esgp, sgp)
		return fmt.Errorf("Can not change sg policy after its created")
	}

	// create new sg state
	sgps, err := NewSgpolicyState(sgp, sm)
	if err != nil {
		log.Errorf("Error creating new sg policy state. Err: %v", err)
		return err
	}

	// store it in local DB
	err = sm.memDB.AddObject(sgps)
	if err != nil {
		log.Errorf("Error storing the sg policy in memdb. Err: %v", err)
		return err
	}

	// make sure the attached security group exists
	err = sgps.updateAttachedSgs()
	if err != nil {
		log.Errorf("Error attching policy to sgs. Err: %v", err)
		return err
	}

	log.Infof("Created Sgpolicy state {%+v}", sgps)

	return nil
}

// DeleteSgpolicy deletes a sg policy
func (sm *Statemgr) DeleteSgpolicy(tenant, sgname string) error {
	// see if we already have it
	sgo, err := sm.FindObject("Sgpolicy", tenant, sgname)
	if err != nil {
		log.Errorf("Can not find the sg policy %s|%s", tenant, sgname)
		return fmt.Errorf("Sgpolicy not found")
	}

	// convert it to security group state
	sg, err := SgpolicyStateFromObj(sgo)
	if err != nil {
		return err
	}

	// cleanup sg state
	err = sg.Delete()
	if err != nil {
		log.Errorf("Error deleting the sg policy {%+v}. Err: %v", sg, err)
		return err
	}

	// delete it from the DB
	return sm.memDB.DeleteObject(sg)
}
