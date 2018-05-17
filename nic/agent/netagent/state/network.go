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

// CreateNetwork creates a network
func (na *Nagent) CreateNetwork(nt *netproto.Network) error {
	err := na.validateMeta(nt.Kind, nt.ObjectMeta)
	if err != nil {
		return err
	}
	// check if network already exists
	oldNt, err := na.FindNetwork(nt.ObjectMeta)
	if err == nil {
		// check if network contents are same
		if !proto.Equal(oldNt, nt) {
			log.Errorf("Network %+v already exists", oldNt)
			return errors.New("network already exists")
		}

		log.Infof("Received duplicate network create for ep {%+v}", nt)
		return nil
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(nt.Tenant, nt.Namespace)
	if err != nil {
		return err
	}

	nt.Status.NetworkID, err = na.Store.GetNextID(types.NetworkID)
	if err != nil {
		log.Errorf("Could not allocate network id. {%+v}", err)
		return err
	}

	uplinks := na.getUplinks()

	// create it in datapath
	err = na.Datapath.CreateNetwork(nt, uplinks, ns)
	if err != nil {
		log.Errorf("Error creating network in datapath. Nw {%+v}. Err: %v", nt, err)
		return err
	}

	// save it in db
	key := objectKey(nt.ObjectMeta, nt.TypeMeta)
	na.Lock()
	na.NetworkDB[key] = nt
	na.Unlock()
	err = na.Store.Write(nt)

	return err
}

// ListNetwork returns the list of networks
func (na *Nagent) ListNetwork() []*netproto.Network {
	var netlist []*netproto.Network

	// lock the db
	na.Lock()
	defer na.Unlock()

	// walk all networks
	for _, nw := range na.NetworkDB {
		netlist = append(netlist, nw)
	}

	return netlist
}

// FindNetwork dins a network in local db
func (na *Nagent) FindNetwork(meta api.ObjectMeta) (*netproto.Network, error) {
	typeMeta := api.TypeMeta{
		Kind: "Network",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	nt, ok := na.NetworkDB[key]
	if !ok {
		return nil, fmt.Errorf("network not found %v", meta.Name)
	}

	return nt, nil
}

// UpdateNetwork updates a network. ToDo implement network updates in datapath
func (na *Nagent) UpdateNetwork(nt *netproto.Network) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(nt.Tenant, nt.Namespace)
	if err != nil {
		return err
	}

	oldNt, err := na.FindNetwork(nt.ObjectMeta)
	if err != nil {
		log.Errorf("Network %v not found", nt.ObjectMeta)
		return err
	}

	if proto.Equal(nt, oldNt) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.Datapath.UpdateNetwork(nt, ns)
	key := objectKey(nt.ObjectMeta, nt.TypeMeta)
	na.Lock()
	na.NetworkDB[key] = nt
	na.Unlock()
	err = na.Store.Write(nt)
	return err
}

// DeleteNetwork deletes a network. ToDo implement network deletes in datapath
func (na *Nagent) DeleteNetwork(nt *netproto.Network) error {
	err := na.validateMeta(nt.Kind, nt.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(nt.Tenant, nt.Namespace)
	if err != nil {
		return err
	}

	// check if network already exists
	nw, err := na.FindNetwork(nt.ObjectMeta)
	if err != nil {
		log.Errorf("Network %+v not found", nt.ObjectMeta)
		return errors.New("network not found")
	}

	// delete the network in datapath
	err = na.Datapath.DeleteNetwork(nw, ns)
	if err != nil {
		log.Errorf("Error deleting network {%+v}. Err: %v", nt, err)
	}

	// delete from db
	key := objectKey(nt.ObjectMeta, nt.TypeMeta)
	na.Lock()
	delete(na.NetworkDB, key)
	na.Unlock()
	err = na.Store.Delete(nt)

	return err
}
