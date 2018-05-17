// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateNatPolicy creates a nat policy
func (na *Nagent) CreateNatPolicy(np *netproto.NatPolicy) error {
	err := na.validateMeta(np.Kind, np.ObjectMeta)
	if err != nil {
		return err
	}
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
		rule.ID, err = na.Store.GetNextID(types.NatRuleID)
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
		poolID := &types.NatPoolRef{
			NamespaceID: natPoolNS.Status.NamespaceID,
			PoolID:      natPool.Status.NatPoolID,
		}
		na.NatPoolLUT[rule.NatPool] = poolID
	}

	np.Status.NatPolicyID, err = na.Store.GetNextID(types.NatPolicyID)

	if err != nil {
		log.Errorf("Could not allocate nat policy id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateNatPolicy(np, na.NatPoolLUT, ns)
	if err != nil {
		log.Errorf("Error creating nat policy in datapath. NatPolicy {%+v}. Err: %v", np, err)
		return err
	}

	// save it in db
	key := objectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	na.NatPolicyDB[key] = np
	na.Unlock()
	err = na.Store.Write(np)

	return err
}

// FindNatPolicy finds a nat policy in local db
func (na *Nagent) FindNatPolicy(meta api.ObjectMeta) (*netproto.NatPolicy, error) {
	typeMeta := api.TypeMeta{
		Kind: "NatPolicy",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	np, ok := na.NatPolicyDB[key]
	if !ok {
		return nil, fmt.Errorf("nat policy not found %v", meta.Name)
	}

	return np, nil
}

// ListNatPolicy returns the list of nat policys
func (na *Nagent) ListNatPolicy() []*netproto.NatPolicy {
	var natPolicyList []*netproto.NatPolicy
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, np := range na.NatPolicyDB {
		natPolicyList = append(natPolicyList, np)
	}

	return natPolicyList
}

// UpdateNatPolicy updates a nat policy
func (na *Nagent) UpdateNatPolicy(np *netproto.NatPolicy) error {
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

	err = na.Datapath.UpdateNatPolicy(np, ns)
	key := objectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	na.NatPolicyDB[key] = np
	na.Unlock()
	err = na.Store.Write(np)
	return err
}

// DeleteNatPolicy deletes a nat policy
func (na *Nagent) DeleteNatPolicy(np *netproto.NatPolicy) error {
	err := na.validateMeta(np.Kind, np.ObjectMeta)
	if err != nil {
		return err
	}
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
	err = na.Datapath.DeleteNatPolicy(existingNatPolicy, ns)
	if err != nil {
		log.Errorf("Error deleting nat policy {%+v}. Err: %v", np, err)
	}

	// delete from db
	key := objectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	delete(na.NatPolicyDB, key)
	na.Unlock()
	err = na.Store.Delete(np)

	return err
}
