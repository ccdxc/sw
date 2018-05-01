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

// CreateNatBinding creates a nat binding
func (na *NetAgent) CreateNatBinding(nb *netproto.NatBinding) error {
	oldNp, err := na.FindNatBinding(nb.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldNp, nb) {
			log.Errorf("NatBinding %+v already exists", oldNp)
			return errors.New("nat binding already exists")
		}

		log.Infof("Received duplicate nat binding create {%+v}", nb)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(nb.Tenant, nb.Namespace)
	if err != nil {
		return err
	}

	nb.Status.NatBindingID, err = na.store.GetNextID(NatBindingID)

	if err != nil {
		log.Errorf("Could not allocate nat binding id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateNatBinding(nb, ns)
	if err != nil {
		log.Errorf("Error creating nat binding in datapath. NatBinding {%+v}. Err: %v", nb, err)
		return err
	}

	// save it in db
	key := objectKey(nb.ObjectMeta, nb.TypeMeta)
	na.Lock()
	na.natBindingDB[key] = nb
	na.Unlock()
	err = na.store.Write(nb)

	return err
}

// FindNatBinding finds a nat binding in local db
func (na *NetAgent) FindNatBinding(meta api.ObjectMeta) (*netproto.NatBinding, error) {
	typeMeta := api.TypeMeta{
		Kind: "NatBinding",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	nb, ok := na.natBindingDB[key]
	if !ok {
		return nil, fmt.Errorf("nat binding not found %v", nb)
	}

	return nb, nil
}

// ListNatBinding returns the list of nat bindings
func (na *NetAgent) ListNatBinding() []*netproto.NatBinding {
	var natBindingList []*netproto.NatBinding
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, nb := range na.natBindingDB {
		natBindingList = append(natBindingList, nb)
	}

	return natBindingList
}

// UpdateNatBinding updates a nat binding
func (na *NetAgent) UpdateNatBinding(nb *netproto.NatBinding) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(nb.Tenant, nb.Namespace)
	if err != nil {
		return err
	}
	existingNp, err := na.FindNatBinding(nb.ObjectMeta)
	if err != nil {
		log.Errorf("NatBinding %v not found", nb.ObjectMeta)
		return err
	}

	if proto.Equal(nb, existingNp) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.datapath.UpdateNatBinding(nb, ns)
	key := objectKey(nb.ObjectMeta, nb.TypeMeta)
	na.Lock()
	na.natBindingDB[key] = nb
	na.Unlock()
	err = na.store.Write(nb)
	return err
}

// DeleteNatBinding deletes a nat binding
func (na *NetAgent) DeleteNatBinding(nb *netproto.NatBinding) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(nb.Tenant, nb.Namespace)
	if err != nil {
		return err
	}

	existingNatBinding, err := na.FindNatBinding(nb.ObjectMeta)
	if err != nil {
		log.Errorf("NatBinding %+v not found", nb.ObjectMeta)
		return errors.New("nat binding not found")
	}

	// delete it in the datapath
	err = na.datapath.DeleteNatBinding(existingNatBinding, ns)
	if err != nil {
		log.Errorf("Error deleting nat binding {%+v}. Err: %v", nb, err)
	}

	// delete from db
	key := objectKey(nb.ObjectMeta, nb.TypeMeta)
	na.Lock()
	delete(na.natBindingDB, key)
	na.Unlock()
	err = na.store.Delete(nb)

	return err
}
