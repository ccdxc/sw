// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
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
		if !proto.Equal(&oldTun.Spec, &tun.Spec) {
			log.Errorf("Tunnel %+v already exists", oldTun)
			return errors.New("tunnel already exists")
		}

		log.Infof("Received duplicate tunnel create for {%+v}", tun)
		return nil
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(tun.ObjectMeta)
	if err != nil {
		return err
	}

	// find the corresponding vrf for the route
	vrf, err := na.ValidateVrf(tun.Tenant, tun.Namespace, tun.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", tun.Spec.VrfName)
		return err
	}
	// Allocate ID only on first object creates and use existing ones during config replay
	if tun.Status.TunnelID == 0 {
		// Tunnel IDs and Interface IDs must be unique in the datapath as tunnel is modeled as an interface in HAL.
		tunnelID, err := na.Store.GetNextID(types.InterfaceID, 0)
		if err != nil {
			log.Errorf("Could not allocate tunnel id. {%+v}", err)
			return err
		}
		tun.Status.TunnelID = tunnelID + types.UplinkOffset + types.TunnelOffset
	}

	// create it in datapath
	err = na.Datapath.CreateTunnel(tun, vrf)
	if err != nil {
		log.Errorf("Error creating tunnel in datapath. Nw {%+v}. Err: %v", tun, err)
		return err
	}

	// Add the current tunnel as a dependency to the namespace.
	err = na.Solver.Add(ns, tun)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, tun)
		return err
	}

	// Add the current tunnel as a dependency to the vrf.
	err = na.Solver.Add(vrf, tun)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", vrf, tun)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(tun.ObjectMeta, tun.TypeMeta)
	na.Lock()
	na.TunnelDB[key] = tun
	na.Unlock()
	dat, _ := tun.Marshal()
	err = na.Store.RawWrite(tun.GetKind(), tun.GetKey(), dat)

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
	key := na.Solver.ObjectKey(meta, typeMeta)
	tun, ok := na.TunnelDB[key]
	if !ok {
		return nil, fmt.Errorf("tunnel not found %v", meta.Name)
	}

	return tun, nil
}

// UpdateTunnel updates a tunnel. ToDo implement tunnel updates in datapath
func (na *Nagent) UpdateTunnel(tun *netproto.Tunnel) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(tun.ObjectMeta)
	if err != nil {
		return err
	}

	existingTunnel, err := na.FindTunnel(tun.ObjectMeta)
	if err != nil {
		log.Errorf("Tunnel %v not found", tun.ObjectMeta)
		return err
	}

	// find the corresponding vrf for the route
	vrf, err := na.ValidateVrf(existingTunnel.Tenant, existingTunnel.Namespace, existingTunnel.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingTunnel.Spec.VrfName)
		return err
	}

	if proto.Equal(tun, existingTunnel) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.Datapath.UpdateTunnel(existingTunnel, vrf)
	if err != nil {
		log.Errorf("Error updating the tunnel {%+v} in datapath. Err: %v", existingTunnel, err)
		return err
	}
	key := na.Solver.ObjectKey(tun.ObjectMeta, tun.TypeMeta)
	na.Lock()
	na.TunnelDB[key] = tun
	na.Unlock()
	err = na.Store.Write(tun)
	return err
}

// DeleteTunnel deletes a tunnel. ToDo implement tunnel deletes in datapath
func (na *Nagent) DeleteTunnel(tn, namespace, name string) error {
	tun := &netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(tun.Kind, tun.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(tun.ObjectMeta)
	if err != nil {
		return err
	}

	// check if existingTunnel already exists
	existingTunnel, err := na.FindTunnel(tun.ObjectMeta)
	if err != nil {
		log.Errorf("Tunnel %+v not found", tun.ObjectMeta)
		return errors.New("tunnel not found")
	}

	// find the corresponding vrf for the route
	vrf, err := na.ValidateVrf(existingTunnel.Tenant, existingTunnel.Namespace, existingTunnel.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingTunnel.Spec.VrfName)
		return err
	}

	// check if the current tunnel has any objects referring to it
	err = na.Solver.Solve(existingTunnel)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingTunnel.Name, err)
		return err
	}

	// delete the existingTunnel in datapath
	err = na.Datapath.DeleteTunnel(existingTunnel, vrf)
	if err != nil {
		log.Errorf("Error deleting tunnel {%+v}. Err: %v", tun, err)
		return err
	}

	err = na.Solver.Remove(vrf, existingTunnel)
	if err != nil {
		log.Errorf("Could not remove the reference to the vrf: %v. Err: %v", existingTunnel.Spec.VrfName, err)
		return err
	}

	// update parent references
	err = na.Solver.Remove(ns, existingTunnel)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(tun.ObjectMeta, tun.TypeMeta)
	na.Lock()
	delete(na.TunnelDB, key)
	na.Unlock()
	err = na.Store.RawDelete(tun.GetKind(), tun.GetKey())

	return err
}
