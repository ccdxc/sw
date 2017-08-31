// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"errors"

	"github.com/gogo/protobuf/proto"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/utils/log"
)

// addSgRules adds sg rules
func (ag *NetAgent) addSgRules(sg *netproto.SecurityGroup) error {
	for _, rule := range sg.Spec.Rules {
		// find peer sg
		if rule.PeerGroup != "" {
			peersg, err := ag.FindSecurityGroup(api.ObjectMeta{Tenant: sg.Tenant, Name: rule.PeerGroup})
			if err != nil {
				log.Errorf("Error finding peer group %s. Err: %v", rule.PeerGroup, err)
				return err
			}

			// set peer sg id
			rule.PeerGroupID = peersg.Status.SecurityGroupID
		}

		// FIXME: add references
	}

	return nil
}

// delSgRules removes SG rules
func (ag *NetAgent) delSgRules(sg *netproto.SecurityGroup) error {
	var err error

	for _, rule := range sg.Spec.Rules {
		// find peer sg
		if rule.PeerGroup != "" {
			_, err = ag.FindSecurityGroup(api.ObjectMeta{Tenant: sg.Tenant, Name: rule.PeerGroup})
			if err != nil {
				log.Errorf("Error finding peer group %s. Err: %v", rule.PeerGroup, err)
				return err
			}
		}

		// FIXME: remove references
	}

	return nil
}

// CreateSecurityGroup creates a security group
func (ag *NetAgent) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	// check if sg already exists
	oldSg, err := ag.FindSecurityGroup(sg.ObjectMeta)
	if err == nil {
		// check if sg contents are same
		if !proto.Equal(oldSg, sg) {
			log.Errorf("Security group %+v already exists", oldSg)
			return errors.New("Security group already exists")
		}

		log.Infof("Received duplicate sg create {%+v}", sg)
		return nil
	}

	// allocate an id
	sg.Status.SecurityGroupID = ag.currentSgID
	ag.currentSgID++

	// create it in datapath
	err = ag.datapath.CreateSecurityGroup(sg)
	if err != nil {
		log.Errorf("Error creating security group in datapath. Sg {%+v}. Err: %v", sg, err)
		return err
	}

	// add rules
	err = ag.addSgRules(sg)
	if err != nil {
		log.Errorf("Error adding sg rules. Err: %v", err)
		return err
	}

	// save it in db
	key := objectKey(sg.ObjectMeta)
	ag.Lock()
	ag.secgroupDB[key] = sg
	ag.Unlock()
	err = ag.store.Write(sg)

	return err
}

// FindSecurityGroup finds a security group
func (ag *NetAgent) FindSecurityGroup(meta api.ObjectMeta) (*netproto.SecurityGroup, error) {
	// lock the db
	ag.Lock()
	defer ag.Unlock()

	// lookup the database
	key := objectKey(meta)
	sg, ok := ag.secgroupDB[key]
	if !ok {
		return nil, errors.New("Security group not found")
	}

	return sg, nil
}

// UpdateSecurityGroup updates an existing security group
func (ag *NetAgent) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	// check if sg already exists
	esg, err := ag.FindSecurityGroup(sg.ObjectMeta)
	if err != nil {
		log.Errorf("Security group %+v not found", sg.ObjectMeta)
		return errors.New("Security group not found")
	}

	// carry over sg id
	sg.Status.SecurityGroupID = esg.Status.SecurityGroupID

	// update sg in datapath
	err = ag.datapath.UpdateSecurityGroup(sg)
	if err != nil {
		log.Errorf("Error updating security group in datapath. Sg{%+v} Err: %v", sg, err)
		return err
	}

	// FIXME: this needs to do a diff of what was previously there
	// remove old rules
	err = ag.delSgRules(esg)
	if err != nil {
		log.Errorf("Error removing sg rules. Err: %v", err)
		return err
	}

	// add new rules
	err = ag.addSgRules(sg)
	if err != nil {
		log.Errorf("Error adding sg rules. Err: %v", err)
		return err
	}

	// update it in db
	key := objectKey(sg.ObjectMeta)
	ag.Lock()
	ag.secgroupDB[key] = sg
	ag.Unlock()
	err = ag.store.Write(sg)

	return err
}

// DeleteSecurityGroup deletes a security group
func (ag *NetAgent) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
	// check if sg already exists
	sgrp, err := ag.FindSecurityGroup(sg.ObjectMeta)
	if err != nil {
		log.Errorf("Security group %+v not found", sg.ObjectMeta)
		return errors.New("Security group not found")
	}

	// delete the sg in datapath
	err = ag.datapath.DeleteSecurityGroup(sgrp)
	if err != nil {
		log.Errorf("Error deleting network {%+v}. Err: %v", sgrp, err)
	}

	// remove all rules
	err = ag.delSgRules(sg)
	if err != nil {
		log.Errorf("Error removing sg rules. Err: %v", err)
		return err
	}

	// delete from db
	key := objectKey(sg.ObjectMeta)
	ag.Lock()
	delete(ag.secgroupDB, key)
	ag.Unlock()
	err = ag.store.Delete(sg)

	return err
}
