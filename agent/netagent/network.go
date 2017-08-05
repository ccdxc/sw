// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"errors"

	"github.com/Sirupsen/logrus"
	"github.com/gogo/protobuf/proto"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
)

// CreateNetwork creates a network
func (ag *NetAgent) CreateNetwork(nt *netproto.Network) error {
	// check if network already exists
	key := objectKey(nt.ObjectMeta)
	oldNt, ok := ag.networkDB[key]
	if ok {
		// check if network contents are same
		if !proto.Equal(oldNt, nt) {
			logrus.Errorf("Network %+v already exists", oldNt)
			return errors.New("Network already exists")
		}

		logrus.Infof("Received duplicate network create for ep {%+v}", nt)
		return nil
	}

	// allocate network id
	nt.Status.NetworkID = ag.currentNetworkID
	ag.currentNetworkID++

	// create it in datapath
	err := ag.datapath.CreateNetwork(nt)
	if err != nil {
		logrus.Errorf("Error creating network in datapath. Nw {%+v}. Err: %v", nt, err)
		return err
	}

	// save it in db
	ag.networkDB[key] = nt

	return nil
}

// FindNetwork dins a network in local db
func (ag *NetAgent) FindNetwork(meta api.ObjectMeta) (*netproto.Network, error) {
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
	key := objectKey(nt.ObjectMeta)
	nw, ok := ag.networkDB[key]
	if !ok {
		logrus.Errorf("Network %+v not found", nt.ObjectMeta)
		return errors.New("Network not found")
	}

	// delete the network in datapath
	err := ag.datapath.DeleteNetwork(nw)
	if err != nil {
		logrus.Errorf("Error deleting network {%+v}. Err: %v", nt, err)
	}

	// delete from db
	delete(ag.networkDB, key)

	return nil
}
