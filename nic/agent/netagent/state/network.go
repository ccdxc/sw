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

// ErrNetworkNotFound is returned when network is not found
var ErrNetworkNotFound = errors.New("network not found")

// CreateNetwork creates a network
func (na *Nagent) CreateNetwork(nt *netproto.Network) error {
	log.Infof("Network Create: %v", nt)
	err := na.validateMeta(nt.Kind, nt.ObjectMeta)
	if err != nil {
		return err
	}
	// check if network already exists
	oldNt, err := na.FindNetwork(nt.ObjectMeta)
	if err == nil {
		// check if network contents are same
		if !proto.Equal(&oldNt.Spec, &nt.Spec) {
			log.Errorf("Network %+v already exists", oldNt)
			return errors.New("network already exists")
		}

		log.Infof("Received duplicate network create for ep {%+v}", nt.ObjectMeta)
		return nil
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(nt.ObjectMeta)
	if err != nil {
		return err
	}

	// find the corresponding vrf for the network
	vrf, err := na.ValidateVrf(nt.Tenant, nt.Namespace, nt.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", nt.Spec.VrfName)
		return err
	}

	// reject duplicate network prefixes
	if err = na.validateDuplicateNetworks(nt.Spec.VrfName, nt.Spec.VlanID); err != nil {
		log.Errorf("Invalid network parameters for network %v. Err: %v", nt.Name, err)
		return err
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if nt.Status.NetworkID == 0 {
		networkID, err := na.Store.GetNextID(types.NetworkID, 0)
		if err != nil {
			log.Errorf("Could not allocate network id. {%+v}", err)
			return err
		}
		nt.Status.NetworkID = networkID + types.NetworkOffset
	}

	uplinks := na.getUplinks()

	// create it in datapath
	err = na.Datapath.CreateNetwork(nt, uplinks, vrf)
	if err != nil {
		log.Errorf("Error creating network in datapath. Nw {%+v}. Err: %v", nt, err)
		return err
	}

	// Add the current network as a dependency to the namespace.
	err = na.Solver.Add(ns, nt)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, nt)
		return err
	}

	// Add the current network as a dependency to the vrf.
	err = na.Solver.Add(vrf, nt)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", vrf, nt)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(nt.ObjectMeta, nt.TypeMeta)
	na.Lock()
	na.NetworkDB[key] = nt
	na.Unlock()
	dat, _ := nt.Marshal()
	err = na.Store.RawWrite(nt.GetKind(), nt.GetKey(), dat)

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
	key := na.Solver.ObjectKey(meta, typeMeta)
	nt, ok := na.NetworkDB[key]
	if !ok {
		return nil, fmt.Errorf("network not found %v", meta.Name)
	}

	return nt, nil
}

// UpdateNetwork updates a network. ToDo implement network updates in datapath
func (na *Nagent) UpdateNetwork(nt *netproto.Network) error {
	// find the corresponding namespace
	log.Infof("Update Network: %v", nt.ObjectMeta)
	_, err := na.FindNamespace(nt.ObjectMeta)
	if err != nil {
		return err
	}

	existingNetwork, err := na.FindNetwork(nt.ObjectMeta)
	if err != nil {
		log.Errorf("Network %v not found", nt.ObjectMeta)
		return err
	}

	if proto.Equal(&nt.Spec, &existingNetwork.Spec) {
		return nil
	}

	// Use the ID that was previously allocated.
	nt.Status.NetworkID = existingNetwork.Status.NetworkID

	// find the corresponding vrf for the network
	vrf, err := na.ValidateVrf(existingNetwork.Tenant, existingNetwork.Namespace, existingNetwork.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingNetwork.Spec.VrfName)
		return err
	}

	uplinks := na.getUplinks()

	err = na.Datapath.UpdateNetwork(nt, uplinks, vrf)
	key := na.Solver.ObjectKey(nt.ObjectMeta, nt.TypeMeta)
	na.Lock()
	na.NetworkDB[key] = nt
	na.Unlock()
	err = na.Store.Write(nt)
	return err
}

// DeleteNetwork deletes a network. ToDo implement network deletes in datapath
func (na *Nagent) DeleteNetwork(tn, namespace, name string) error {
	if tn == "default" && namespace == "default" && name == "_internal_untagged_nw" {
		return errors.New("default untagged network under default tenant and namespace cannot be deleted")
	}
	nt := &netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(nt.Kind, nt.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(nt.ObjectMeta)
	if err != nil {
		return err
	}

	// check if network already exists
	nw, err := na.FindNetwork(nt.ObjectMeta)
	if err != nil {
		log.Errorf("Network %+v not found", nt.ObjectMeta)
		return ErrNetworkNotFound
	}

	// find the corresponding vrf for the network
	vrf, err := na.ValidateVrf(nw.Tenant, nw.Namespace, nw.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", nw.Spec.VrfName)
		return err
	}

	// get all the uplinks
	uplinks := na.getUplinks()

	// check if the current network has any objects referring to it
	err = na.Solver.Solve(nw)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", nw.Name, err)
		return err
	}
	// clear for deletion. delete the network in datapath
	err = na.Datapath.DeleteNetwork(nw, uplinks, vrf)
	if err != nil {
		log.Errorf("Error deleting network {%+v}. Err: %v", nt, err)
		return err
	}

	err = na.Solver.Remove(vrf, nw)
	if err != nil {
		log.Errorf("Could not remove the reference to the vrf: %v. Err: %v", nw.Spec.VrfName, err)
		return err
	}

	// update parent references
	err = na.Solver.Remove(ns, nw)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(nt.ObjectMeta, nt.TypeMeta)
	na.Lock()
	delete(na.NetworkDB, key)
	na.Unlock()
	err = na.Store.RawDelete(nt.GetKind(), nt.GetKey())

	return err
}

func (na *Nagent) validateDuplicateNetworks(vrfName string, vlanID uint32) (err error) {
	for _, net := range na.ListNetwork() {
		if vrfName == net.Spec.VrfName && vlanID == net.Spec.VlanID {
			err = fmt.Errorf("found an existing network %v with vlan-id %v", net.Name, vlanID)
			return
		}
	}
	return
}
