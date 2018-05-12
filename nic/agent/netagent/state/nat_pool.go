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

// CreateNatPool creates a nat pool
func (na *NetAgent) CreateNatPool(np *netproto.NatPool) error {
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
	ns, err := na.FindNamespace(np.Tenant, np.Namespace)
	if err != nil {
		return err
	}

	// validate nat pool message

	np.Status.NatPoolID, err = na.store.GetNextID(NatPoolID)

	if err != nil {
		log.Errorf("Could not allocate nat pool id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateNatPool(np, ns)
	if err != nil {
		log.Errorf("Error creating nat pool in datapath. NatPool {%+v}. Err: %v", np, err)
		return err
	}

	// save it in db
	key := objectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	na.natPoolDB[key] = np
	na.Unlock()
	err = na.store.Write(np)

	return err
}

// FindNatPool finds a nat pool in local db
func (na *NetAgent) FindNatPool(meta api.ObjectMeta) (*netproto.NatPool, error) {
	typeMeta := api.TypeMeta{
		Kind: "NatPool",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	tn, ok := na.natPoolDB[key]
	if !ok {
		return nil, fmt.Errorf("nat pool not found %v", meta.Name)
	}

	return tn, nil
}

// ListNatPool returns the list of nat pools
func (na *NetAgent) ListNatPool() []*netproto.NatPool {
	var natPoolList []*netproto.NatPool
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, np := range na.natPoolDB {
		natPoolList = append(natPoolList, np)
	}

	return natPoolList
}

// UpdateNatPool updates a nat pool
func (na *NetAgent) UpdateNatPool(np *netproto.NatPool) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(np.Tenant, np.Namespace)
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

	err = na.datapath.UpdateNatPool(np, ns)
	key := objectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	na.natPoolDB[key] = np
	na.Unlock()
	err = na.store.Write(np)
	return err
}

// DeleteNatPool deletes a nat pool
func (na *NetAgent) DeleteNatPool(np *netproto.NatPool) error {
	err := na.validateMeta(np.Kind, np.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(np.Tenant, np.Namespace)
	if err != nil {
		return err
	}

	existingNatPool, err := na.FindNatPool(np.ObjectMeta)
	if err != nil {
		log.Errorf("NatPool %+v not found", np.ObjectMeta)
		return errors.New("nat pool not found")
	}

	// delete it in the datapath
	err = na.datapath.DeleteNatPool(existingNatPool, ns)
	if err != nil {
		log.Errorf("Error deleting nat pool {%+v}. Err: %v", np, err)
	}

	// delete from db
	key := objectKey(np.ObjectMeta, np.TypeMeta)
	na.Lock()
	delete(na.natPoolDB, key)
	na.Unlock()
	err = na.store.Delete(np)

	return err
}
