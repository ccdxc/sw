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

// CreateTunnel creates a tunnel
func (na *Nagent) CreateTunnel(tun *netproto.Tunnel) error {
	err := na.validateMeta(tun.Kind, tun.ObjectMeta)
	if err != nil {
		return err
	}
	// check if tunnel already exists
	oldTun, err := na.FindTunnel(tun.ObjectMeta)
	if err == nil {
		// check if tunnel contents are same
		if !proto.Equal(oldTun, tun) {
			log.Errorf("Tunnel %+v already exists", oldTun)
			return errors.New("tunnel already exists")
		}

		log.Infof("Received duplicate tunnel create for {%+v}", tun)
		return nil
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(tun.Tenant, tun.Namespace)
	if err != nil {
		return err
	}

	// Tunnel IDs and Interface IDs must be unique in the datapath as tunnel is modeled as an interface in HAL.
	tunnelID, err := na.Store.GetNextID(types.InterfaceID)
	if err != nil {
		log.Errorf("Could not allocate tunnel id. {%+v}", err)
		return err
	}
	tun.Status.TunnelID = tunnelID + maxNumUplinks

	// create it in datapath
	err = na.Datapath.CreateTunnel(tun, ns)
	if err != nil {
		log.Errorf("Error creating tunnel in datapath. Nw {%+v}. Err: %v", tun, err)
		return err
	}

	// save it in db
	key := objectKey(tun.ObjectMeta, tun.TypeMeta)
	na.Lock()
	na.TunnelDB[key] = tun
	na.Unlock()
	err = na.Store.Write(tun)

	return err
}

// ListTunnel returns the list of tunnels
func (na *Nagent) ListTunnel() []*netproto.Tunnel {
	var tunnelList []*netproto.Tunnel

	// lock the db
	na.Lock()
	defer na.Unlock()

	// walk all tunnels
	for _, tun := range na.TunnelDB {
		tunnelList = append(tunnelList, tun)
	}

	return tunnelList
}

// FindTunnel dins a tunnel in local db
func (na *Nagent) FindTunnel(meta api.ObjectMeta) (*netproto.Tunnel, error) {
	typeMeta := api.TypeMeta{
		Kind: "Tunnel",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	tun, ok := na.TunnelDB[key]
	if !ok {
		return nil, fmt.Errorf("tunnel not found %v", meta.Name)
	}

	return tun, nil
}

// UpdateTunnel updates a tunnel. ToDo implement tunnel updates in datapath
func (na *Nagent) UpdateTunnel(tun *netproto.Tunnel) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(tun.Tenant, tun.Namespace)
	if err != nil {
		return err
	}

	oldTun, err := na.FindTunnel(tun.ObjectMeta)
	if err != nil {
		log.Errorf("Tunnel %v not found", tun.ObjectMeta)
		return err
	}

	if proto.Equal(tun, oldTun) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.Datapath.UpdateTunnel(tun, ns)
	key := objectKey(tun.ObjectMeta, tun.TypeMeta)
	na.Lock()
	na.TunnelDB[key] = tun
	na.Unlock()
	err = na.Store.Write(tun)
	return err
}

// DeleteTunnel deletes a tunnel. ToDo implement tunnel deletes in datapath
func (na *Nagent) DeleteTunnel(tun *netproto.Tunnel) error {
	err := na.validateMeta(tun.Kind, tun.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(tun.Tenant, tun.Namespace)
	if err != nil {
		return err
	}

	// check if tunnel already exists
	tunnel, err := na.FindTunnel(tun.ObjectMeta)
	if err != nil {
		log.Errorf("Tunnel %+v not found", tun.ObjectMeta)
		return errors.New("tunnel not found")
	}

	// delete the tunnel in datapath
	err = na.Datapath.DeleteTunnel(tunnel, ns)
	if err != nil {
		log.Errorf("Error deleting tunnel {%+v}. Err: %v", tun, err)
		return err
	}

	// delete from db
	key := objectKey(tun.ObjectMeta, tun.TypeMeta)
	na.Lock()
	delete(na.TunnelDB, key)
	na.Unlock()
	err = na.Store.Delete(tun)

	return err
}
