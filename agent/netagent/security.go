// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"errors"

	"github.com/Sirupsen/logrus"
	"github.com/gogo/protobuf/proto"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
)

// addSgRules adds sg rules
func (ag *NetAgent) addSgRules(sg *netproto.SecurityGroup) error {
	var err error

	for _, rule := range sg.Spec.Rules {
		peersg := &netproto.SecurityGroup{}
		// find peer sg
		if rule.PeerGroup != "" {
			peersg, err = ag.FindSecurityGroup(api.ObjectMeta{Tenant: sg.Tenant, Name: rule.PeerGroup})
			if err != nil {
				logrus.Errorf("Error finding peer group %s. Err: %v", rule.PeerGroup, err)
				return err
			}
		}

		// add rule
		err = ag.datapath.AddSecurityRule(sg, &rule, peersg)
		if err != nil {
			logrus.Errorf("Error adding rule in datapath. Rule{%+v}, Err: %v", rule, err)
			return err
		}
	}

	return nil
}

// delSgRules removes SG rules
func (ag *NetAgent) delSgRules(sg *netproto.SecurityGroup) error {
	var err error

	for _, rule := range sg.Spec.Rules {
		peersg := &netproto.SecurityGroup{}

		// find peer sg
		if rule.PeerGroup != "" {
			peersg, err = ag.FindSecurityGroup(api.ObjectMeta{Tenant: sg.Tenant, Name: rule.PeerGroup})
			if err != nil {
				logrus.Errorf("Error finding peer group %s. Err: %v", rule.PeerGroup, err)
				return err
			}
		}

		// delete rule
		err = ag.datapath.DeleteSecurityRule(sg, &rule, peersg)
		if err != nil {
			logrus.Errorf("Error deleting the rule. rule {%+v} err: %v", rule, err)
		}
	}

	return nil
}

// CreateSecurityGroup creates a security group
func (ag *NetAgent) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	// check if sg already exists
	key := objectKey(sg.ObjectMeta)
	oldSg, ok := ag.secgroupDB[key]
	if ok {
		// check if sg contents are same
		if !proto.Equal(oldSg, sg) {
			logrus.Errorf("Security group %+v already exists", oldSg)
			return errors.New("Security group already exists")
		}

		logrus.Infof("Received duplicate sg create {%+v}", sg)
		return nil
	}

	// allocate an id
	sg.Status.SecurityGroupID = ag.currentSgID
	ag.currentSgID++

	// create it in datapath
	err := ag.datapath.CreateSecurityGroup(sg)
	if err != nil {
		logrus.Errorf("Error creating security group in datapath. Sg {%+v}. Err: %v", sg, err)
		return err
	}

	// add rules
	err = ag.addSgRules(sg)
	if err != nil {
		logrus.Errorf("Error adding sg rules. Err: %v", err)
		return err
	}

	// save it in db
	ag.secgroupDB[key] = sg
	return nil
}

// FindSecurityGroup finds a security group
func (ag *NetAgent) FindSecurityGroup(meta api.ObjectMeta) (*netproto.SecurityGroup, error) {
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
	key := objectKey(sg.ObjectMeta)
	esg, ok := ag.secgroupDB[key]
	if !ok {
		logrus.Errorf("Security group %+v not found", sg.ObjectMeta)
		return errors.New("Security group not found")
	}

	// carry over sg id
	sg.Status.SecurityGroupID = esg.Status.SecurityGroupID

	// update sg in datapath
	err := ag.datapath.UpdateSecurityGroup(sg)
	if err != nil {
		logrus.Errorf("Error updating security group in datapath. Sg{%+v} Err: %v", sg, err)
		return err
	}

	// FIXME: this needs to do a diff of what was previously there
	// remove old rules
	err = ag.delSgRules(esg)
	if err != nil {
		logrus.Errorf("Error removing sg rules. Err: %v", err)
		return err
	}

	// add new rules
	err = ag.addSgRules(sg)
	if err != nil {
		logrus.Errorf("Error adding sg rules. Err: %v", err)
		return err
	}

	// update it in db
	ag.secgroupDB[key] = sg

	return nil
}

// DeleteSecurityGroup deletes a security group
func (ag *NetAgent) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
	// check if sg already exists
	key := objectKey(sg.ObjectMeta)
	sgrp, ok := ag.secgroupDB[key]
	if !ok {
		logrus.Errorf("Security group %+v not found", sg.ObjectMeta)
		return errors.New("Security group not found")
	}

	// delete the sg in datapath
	err := ag.datapath.DeleteSecurityGroup(sgrp)
	if err != nil {
		logrus.Errorf("Error deleting network {%+v}. Err: %v", sgrp, err)
	}

	// remove all rules
	err = ag.delSgRules(sg)
	if err != nil {
		logrus.Errorf("Error removing sg rules. Err: %v", err)
		return err
	}

	// delete from db
	delete(ag.secgroupDB, key)
	return nil
}
