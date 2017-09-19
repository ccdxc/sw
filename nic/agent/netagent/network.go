// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"errors"

	"github.com/gogo/protobuf/proto"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateNetwork creates a network
func (ag *NetAgent) CreateNetwork(nt *netproto.Network) error {
	// check if network already exists
	oldNt, err := ag.FindNetwork(nt.ObjectMeta)
	if err == nil {
		// check if network contents are same
		if !proto.Equal(oldNt, nt) {
			log.Errorf("Network %+v already exists", oldNt)
			return errors.New("Network already exists")
		}

		log.Infof("Received duplicate network create for ep {%+v}", nt)
		return nil
	}

	// allocate network id
	nt.Status.NetworkID = ag.currentNetworkID
	ag.currentNetworkID++

	// create it in datapath
	err = ag.datapath.CreateNetwork(nt)
	if err != nil {
		log.Errorf("Error creating network in datapath. Nw {%+v}. Err: %v", nt, err)
		return err
	}

	// save it in db
	key := objectKey(nt.ObjectMeta)
	ag.Lock()
	ag.networkDB[key] = nt
	ag.Unlock()
	err = ag.store.Write(nt)

	return err
}

// FindNetwork dins a network in local db
func (ag *NetAgent) FindNetwork(meta api.ObjectMeta) (*netproto.Network, error) {
	// lock the db
	ag.Lock()
	defer ag.Unlock()

	// lookup the database
	key := objectKey(meta)
	nt, ok := ag.networkDB[key]
	if !ok {
		return nil, errors.New("Network not found")
	}

	return nt, nil
}

// UpdateNetwork updates a network
func (ag *NetAgent) UpdateNetwork(nt *netproto.Network) error {
	// FIXME:
	return nil
}

// DeleteNetwork deletes a network
func (ag *NetAgent) DeleteNetwork(nt *netproto.Network) error {
	// check if network already exists
	nw, err := ag.FindNetwork(nt.ObjectMeta)
	if err != nil {
		log.Errorf("Network %+v not found", nt.ObjectMeta)
		return errors.New("Network not found")
	}

	// delete the network in datapath
	err = ag.datapath.DeleteNetwork(nw)
	if err != nil {
		log.Errorf("Error deleting network {%+v}. Err: %v", nt, err)
	}

	// delete from db
	key := objectKey(nt.ObjectMeta)
	ag.Lock()
	delete(ag.networkDB, key)
	ag.Unlock()
	err = ag.store.Delete(nt)

	return err
}
