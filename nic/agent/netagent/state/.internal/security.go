// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package _internal

import (
	"errors"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ErrSecurityGroupNotFound is returned when security group is not found
var ErrSecurityGroupNotFound = errors.New("Security group not found")

// CreateSecurityGroup creates a security group. ToDo Handle creates in datapath
func (na *state.Nagent) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	err := na.validateMeta(sg.Kind, sg.ObjectMeta)
	if err != nil {
		return err
	}
	// check if sg already exists
	oldSg, err := na.FindSecurityGroup(sg.ObjectMeta)
	if err == nil {
		// check if sg contents are same
		if !proto.Equal(oldSg, sg) {
			log.Errorf("Security group %+v already exists", oldSg)
			return errors.New("security group already exists")
		}

		log.Infof("Received duplicate sg create {%+v}", sg)
		return nil
	}

	// find the corresponding namespace
	_, err = na.FindNamespace(sg.ObjectMeta)
	if err != nil {
		return err
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if sg.Status.SecurityGroupID == 0 {
		sg.Status.SecurityGroupID, err = na.Store.GetNextID(types.SecurityGroupID)
	}

	if err != nil {
		log.Errorf("Could not allocate security group id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateSecurityGroup(sg)
	if err != nil {
		log.Errorf("Error creating security group in datapath. Sg {%+v}. Err: %v", sg.ObjectMeta, err)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(sg.ObjectMeta, sg.TypeMeta)
	na.Lock()
	na.SecgroupDB[key] = sg
	na.Unlock()
	err = na.Store.Write(sg)

	return err
}

// ListSecurityGroup returns the list of sgs
func (na *state.Nagent) ListSecurityGroup() []*netproto.SecurityGroup {

	var sgList []*netproto.SecurityGroup

	// lock the db
	na.Lock()
	defer na.Unlock()

	// walk all security groups
	for _, sg := range na.SecgroupDB {
		sgList = append(sgList, sg)
	}

	return sgList
}

// FindSecurityGroup finds a security group
func (na *state.Nagent) FindSecurityGroup(meta api.ObjectMeta) (*netproto.SecurityGroup, error) {
	typeMeta := api.TypeMeta{
		Kind: "SecurityGroup",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	sg, ok := na.SecgroupDB[key]
	if !ok {
		return nil, errors.New("Security group not found")
	}

	return sg, nil
}

// UpdateSecurityGroup updates an existing security group. ToDo implement updates in datapath
func (na *state.Nagent) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(sg.ObjectMeta)
	if err != nil {
		return err
	}
	// check if sg already exists
	esg, err := na.FindSecurityGroup(sg.ObjectMeta)
	if err != nil {
		log.Errorf("Security group %+v not found", sg.ObjectMeta)
		return ErrSecurityGroupNotFound
	}

	// carry over sg id
	sg.Status.SecurityGroupID = esg.Status.SecurityGroupID

	// update sg in datapath
	err = na.Datapath.UpdateSecurityGroup(sg)
	if err != nil {
		log.Errorf("Error updating security group in datapath. Sg{%+v} Err: %v", sg.ObjectMeta, err)
		return err
	}

	// update it in db
	key := na.Solver.ObjectKey(sg.ObjectMeta, sg.TypeMeta)
	na.Lock()
	na.SecgroupDB[key] = sg
	na.Unlock()
	err = na.Store.Write(sg)

	return err
}

// DeleteSecurityGroup deletes a security group. ToDo handle deletes in datapath
func (na *state.Nagent) DeleteSecurityGroup(tn, namespace, name string) error {
	sg := &netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(sg.Kind, sg.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	_, err = na.FindNamespace(sg.ObjectMeta)
	if err != nil {
		return err
	}

	// chek if sg already exists
	existingSecurityGrp, err := na.FindSecurityGroup(sg.ObjectMeta)
	if err != nil {
		log.Errorf("Security group %+v not found", sg.ObjectMeta)
		return ErrSecurityGroupNotFound
	}

	// check if the current security groups has any objects referring to it
	err = na.Solver.Solve(existingSecurityGrp)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingSecurityGrp.Name, err)
		return err
	}

	// delete the sg in datapath
	err = na.Datapath.DeleteSecurityGroup(existingSecurityGrp)
	if err != nil {
		log.Errorf("Error deleting network {%+v}. Err: %v", existingSecurityGrp, err)
	}

	// delete from db
	key := na.Solver.ObjectKey(sg.ObjectMeta, sg.TypeMeta)
	na.Lock()
	delete(na.SecgroupDB, key)
	na.Unlock()
	err = na.Store.Delete(sg)

	return err
}
