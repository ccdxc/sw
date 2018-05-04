// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"

	"github.com/gogo/protobuf/proto"

	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateNatPolicy creates a nat policy
func (na *NetAgent) CreateNatPolicy(np *netproto.NatPolicy) error {
	oldNp, err := na.FindNatPolicy(np.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldNp, np) {
			log.Errorf("NatPolicy %+v already exists", oldNp)
			return errors.New("nat policy already exists")
		}

		log.Infof("Received duplicate nat policy create {%+v}", np)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(np.Tenant, np.Namespace)
	if err != nil {
		return err
	}

	for _, rule := range np.Spec.Rules {
		rule.ID, err = na.store.GetNextID(NatRuleID)
		natPool, err := na.findNatPool(np.ObjectMeta, rule.NatPool)
		if err != nil {
			log.Errorf("could not find nat pool for the rule. Rule: {%v}. Err: %v", rule, err)
			return err
		}
		natPoolNS, err := na.FindNamespace(np.Tenant, natPool.Namespace)
		if err != nil {
			log.Errorf("could not find the nat pool namespace. NatPool Namespace: {%v}. Err: %v", natPoolNS, err)
			return err
		}
		poolID := &NatPoolRef{
			NamespaceID: natPoolNS.Status.NamespaceID,
			PoolID:      natPool.Status.NatPoolID,
		}
		na.natPoolLUT[rule.NatPool] = poolID
	}

	np.Status.NatPolicyID, err = na.store.GetNextID(NatPolicyID)

	if err != nil {
		log.Errorf("Could not allocate nat policy id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateNatPolicy(np, na.natPoolLUT, ns)
	if err != nil {
		log.Errorf("Error creating nat policy in datapath. NatPolicy {%+v}. Err: %v", np, err)
		return err
	}

	// save it in db
	key := objectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	na.natPolicyDB[key] = np
	na.Unlock()
	err = na.store.Write(np)

	return err
}

// FindNatPolicy finds a nat policy in local db
func (na *NetAgent) FindNatPolicy(meta api.ObjectMeta) (*netproto.NatPolicy, error) {
	typeMeta := api.TypeMeta{
		Kind: "NatPolicy",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	np, ok := na.natPolicyDB[key]
	if !ok {
		return nil, fmt.Errorf("nat policy not found %v", np)
	}

	return np, nil
}

// ListNatPolicy returns the list of nat policys
func (na *NetAgent) ListNatPolicy() []*netproto.NatPolicy {
	var natPolicyList []*netproto.NatPolicy
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, np := range na.natPolicyDB {
		natPolicyList = append(natPolicyList, np)
	}

	return natPolicyList
}

// UpdateNatPolicy updates a nat policy
func (na *NetAgent) UpdateNatPolicy(np *netproto.NatPolicy) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(np.Tenant, np.Namespace)
	if err != nil {
		return err
	}
	existingNp, err := na.FindNatPolicy(np.ObjectMeta)
	if err != nil {
		log.Errorf("NatPolicy %v not found", np.ObjectMeta)
		return err
	}

	if proto.Equal(np, existingNp) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.datapath.UpdateNatPolicy(np, ns)
	key := objectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	na.natPolicyDB[key] = np
	na.Unlock()
	err = na.store.Write(np)
	return err
}

// DeleteNatPolicy deletes a nat policy
func (na *NetAgent) DeleteNatPolicy(np *netproto.NatPolicy) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(np.Tenant, np.Namespace)
	if err != nil {
		return err
	}

	existingNatPolicy, err := na.FindNatPolicy(np.ObjectMeta)
	if err != nil {
		log.Errorf("NatPolicy %+v not found", np.ObjectMeta)
		return errors.New("nat policy not found")
	}

	// delete it in the datapath
	err = na.datapath.DeleteNatPolicy(existingNatPolicy, ns)
	if err != nil {
		log.Errorf("Error deleting nat policy {%+v}. Err: %v", np, err)
	}

	// delete from db
	key := objectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	delete(na.natPolicyDB, key)
	na.Unlock()
	err = na.store.Delete(np)

	return err
}
