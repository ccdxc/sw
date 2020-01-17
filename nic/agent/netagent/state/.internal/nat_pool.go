// +build ignore

// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package _internal

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateNatPool creates a nat pool
func (na *state.Nagent) CreateNatPool(np *netproto.NatPool) error {
	err := na.validateMeta(np.Kind, np.ObjectMeta)
	if err != nil {
		return err
	}
	oldNp, err := na.FindNatPool(np.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldNp, np) {
			log.Errorf("NatPool %+v already exists", oldNp)
			return errors.New("nat pool already exists")
		}

		log.Infof("Received duplicate nat pool create {%+v}", np)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(np.ObjectMeta)
	if err != nil {
		return err
	}

	// find the corresponding vrf for the nat binding
	vrf, err := na.ValidateVrf(np.Tenant, np.Namespace, np.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", np.Spec.VrfName)
		return err
	}

	// validate nat pool message

	// Allocate ID only on first object creates and use existing ones during config replay
	if np.Status.NatPoolID == 0 {
		np.Status.NatPoolID, err = na.Store.GetNextID(types.NatPoolID)
	}

	if err != nil {
		log.Errorf("Could not allocate nat pool id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateNatPool(np, vrf)
	if err != nil {
		log.Errorf("Error creating nat pool in datapath. NatPool {%+v}. Err: %v", np, err)
		return err
	}

	// Add the current Nat Pool as a dependency to the namespace.
	err = na.Solver.Add(ns, np)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, np)
		return err
	}

	// Add the current nat pool as a dependency to the vrf.
	err = na.Solver.Add(vrf, np)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", vrf, np)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	na.NatPoolDB[key] = np
	na.Unlock()
	err = na.Store.Write(np)

	return err
}

// FindNatPool finds a nat pool in local db
func (na *state.Nagent) FindNatPool(meta api.ObjectMeta) (*netproto.NatPool, error) {
	typeMeta := api.TypeMeta{
		Kind: "NatPool",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	tn, ok := na.NatPoolDB[key]
	if !ok {
		return nil, fmt.Errorf("nat pool not found %v", meta.Name)
	}

	return tn, nil
}

// ListNatPool returns the list of nat pools
func (na *state.Nagent) ListNatPool() []*netproto.NatPool {
	var natPoolList []*netproto.NatPool
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, np := range na.NatPoolDB {
		natPoolList = append(natPoolList, np)
	}

	return natPoolList
}

// UpdateNatPool updates a nat pool
func (na *state.Nagent) UpdateNatPool(np *netproto.NatPool) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(np.ObjectMeta)
	if err != nil {
		return err
	}
	existingNp, err := na.FindNatPool(np.ObjectMeta)
	if err != nil {
		log.Errorf("NatPool %v not found", np.ObjectMeta)
		return err
	}

	if proto.Equal(np, existingNp) {
		log.Infof("Nothing to update.")
		return nil
	}
	// find the corresponding vrf for the nat binding
	vrf, err := na.ValidateVrf(existingNp.Tenant, existingNp.Namespace, existingNp.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingNp.Spec.VrfName)
		return err
	}

	err = na.Datapath.UpdateNatPool(existingNp, vrf)
	if err != nil {
		log.Errorf("Error updating the nat pool {%+v} in datapath. Err: %v", existingNp, err)
		return err
	}
	key := na.Solver.ObjectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	na.NatPoolDB[key] = np
	na.Unlock()
	err = na.Store.Write(np)
	return err
}

// DeleteNatPool deletes a nat pool
func (na *state.Nagent) DeleteNatPool(tn, namespace, name string) error {
	np := &netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(np.Kind, np.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(np.ObjectMeta)
	if err != nil {
		return err
	}

	existingNatPool, err := na.FindNatPool(np.ObjectMeta)
	if err != nil {
		log.Errorf("NatPool %+v not found", np.ObjectMeta)
		return errors.New("nat pool not found")
	}

	// find the corresponding vrf for the nat binding
	vrf, err := na.ValidateVrf(existingNatPool.Tenant, existingNatPool.Namespace, existingNatPool.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingNatPool.Spec.VrfName)
		return err
	}

	// check if the current nat pool has any objects referring to it
	err = na.Solver.Solve(existingNatPool)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingNatPool.Name, err)
		return err
	}

	// delete it in the datapath
	err = na.Datapath.DeleteNatPool(existingNatPool, vrf)
	if err != nil {
		log.Errorf("Error deleting nat pool {%+v}. Err: %v", np, err)
	}

	err = na.Solver.Remove(vrf, existingNatPool)
	if err != nil {
		log.Errorf("Could not remove the reference to the vrf: %v. Err: %v", existingNatPool.Spec.VrfName, err)
		return err
	}

	// update parent references
	err = na.Solver.Remove(ns, existingNatPool)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	delete(na.NatPoolDB, key)
	na.Unlock()
	err = na.Store.Delete(np)

	return err
}
