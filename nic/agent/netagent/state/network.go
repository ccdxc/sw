// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateNetwork creates a network
func (na *NetAgent) CreateNetwork(nt *netproto.Network) error {
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

	nt.Status.NetworkID, err = na.store.GetNextID(NetworkID)
	if err != nil {
		log.Errorf("Could not allocate network id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateNetwork(nt)
	if err != nil {
		log.Errorf("Error creating network in datapath. Nw {%+v}. Err: %v", nt, err)
		return err
	}

	// save it in db
	key := objectKey(nt.ObjectMeta)
	na.Lock()
	na.networkDB[key] = nt
	na.Unlock()
	err = na.store.Write(nt)

	return err
}

// ListNetwork returns the list of networks
func (na *NetAgent) ListNetwork() []*netproto.Network {
	var netlist []*netproto.Network

	// lock the db
	na.Lock()
	defer na.Unlock()

	// walk all networks
	for _, nw := range na.networkDB {
		netlist = append(netlist, nw)
	}

	return netlist
}

// FindNetwork dins a network in local db
func (na *NetAgent) FindNetwork(meta api.ObjectMeta) (*netproto.Network, error) {
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta)
	nt, ok := na.networkDB[key]
	if !ok {
		return nil, errors.New("Network not found")
	}

	return nt, nil
}

// UpdateNetwork updates a network
func (na *NetAgent) UpdateNetwork(nt *netproto.Network) error {
	oldNt, err := na.FindNetwork(nt.ObjectMeta)
	if err != nil {
		log.Errorf("Network %v not found", nt.ObjectMeta)
		return err
	}

	if proto.Equal(nt, oldNt) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.datapath.UpdateNetwork(nt)
	key := objectKey(nt.ObjectMeta)
	na.Lock()
	na.networkDB[key] = nt
	na.Unlock()
	err = na.store.Write(nt)
	return err
}

// DeleteNetwork deletes a network
func (na *NetAgent) DeleteNetwork(nt *netproto.Network) error {
	// check if network already exists
	nw, err := na.FindNetwork(nt.ObjectMeta)
	if err != nil {
		log.Errorf("Network %+v not found", nt.ObjectMeta)
		return errors.New("Network not found")
	}

	// delete the network in datapath
	err = na.datapath.DeleteNetwork(nw)
	if err != nil {
		log.Errorf("Error deleting network {%+v}. Err: %v", nt, err)
	}

	// delete from db
	key := objectKey(nt.ObjectMeta)
	na.Lock()
	delete(na.networkDB, key)
	na.Unlock()
	err = na.store.Delete(nt)

	return err
}
