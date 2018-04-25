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

	np.Status.NatPolicyID, err = na.store.GetNextID(NatPolicyID)

	if err != nil {
		log.Errorf("Could not allocate nat policy id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateNatPolicy(np)
	if err != nil {
		log.Errorf("Error creating nat policy in datapath. NatPolicy {%+v}. Err: %v", np, err)
		return err
	}

	// save it in db
	key := objectKey(np.ObjectMeta)
	na.Lock()
	na.natPolicyDB[key] = np
	na.Unlock()
	err = na.store.Write(np)

	return err
}

// FindNatPolicy finds a nat policy in local db
func (na *NetAgent) FindNatPolicy(meta api.ObjectMeta) (*netproto.NatPolicy, error) {
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta)
	tn, ok := na.natPolicyDB[key]
	if !ok {
		return nil, fmt.Errorf("nat policy not found %v", tn)
	}

	return tn, nil
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
	existingNp, err := na.FindNatPolicy(np.ObjectMeta)
	if err != nil {
		log.Errorf("NatPolicy %v not found", np.ObjectMeta)
		return err
	}

	if proto.Equal(np, existingNp) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.datapath.UpdateNatPolicy(np)
	key := objectKey(np.ObjectMeta)
	na.Lock()
	na.natPolicyDB[key] = np
	na.Unlock()
	err = na.store.Write(np)
	return err
}

// DeleteNatPolicy deletes a nat policy
func (na *NetAgent) DeleteNatPolicy(np *netproto.NatPolicy) error {

	existingNatPolicy, err := na.FindNatPolicy(np.ObjectMeta)
	if err != nil {
		log.Errorf("NatPolicy %+v not found", np.ObjectMeta)
		return errors.New("nat policy not found")
	}

	// delete it in the datapath
	err = na.datapath.DeleteNatPolicy(existingNatPolicy)
	if err != nil {
		log.Errorf("Error deleting nat policy {%+v}. Err: %v", np, err)
	}

	// delete from db
	key := objectKey(np.ObjectMeta)
	na.Lock()
	delete(na.natPolicyDB, key)
	na.Unlock()
	err = na.store.Delete(np)

	return err
}
