// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	dnetproto "github.com/pensando/sw/nic/agent/protos/generated/delphi/netproto/delphi"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateInterface creates an interface
func (na *Nagent) CreateInterface(intf *netproto.Interface) error {
	err := na.validateMeta(intf.Kind, intf.ObjectMeta)
	if err != nil {
		return err
	}
	var ok bool
	var lif *netproto.Interface

	oldIf, err := na.FindInterface(intf.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldIf, intf) {
			log.Errorf("Interface %+v already exists", oldIf)
			return errors.New("interface already exists")
		}

		log.Infof("Received duplicate interface create {%+v}", intf)
		return nil
	}

	_, err = na.FindNamespace(intf.ObjectMeta)
	if err != nil {
		return err
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if intf.Status.InterfaceID == 0 {
		intfID, err := na.Store.GetNextID(types.InterfaceID)

		if err != nil {
			log.Errorf("Could not allocate interface id. {%+v}", err)
			return err
		}
		intf.Status.InterfaceID = intfID + types.UplinkOffset
	}

	// Perform interface associations, currently only ENIC interfaces supported as
	switch intf.Spec.Type {
	case "ENIC":

		lifCount, err := na.countIntfs("LIF")
		if err != nil {
			log.Errorf("could not enumerate lifs created")
			return err
		}
		// lifIndex finds an available lif. Uses % operator to ensure uniform distribution
		intfOffset := intf.Status.InterfaceID % lifCount
		// lif ids start from 1
		if intfOffset == 0 {
			intfOffset++
		}
		lif, ok = na.findIntfByName(fmt.Sprintf("lif%d", intfOffset))

		if !ok {
			log.Errorf("could not find user specified lif: {%v}", lif)
			return errors.New("lif not found")
		}
	}

	// create it in datapath
	err = na.Datapath.CreateInterface(intf, lif)
	if err != nil {
		log.Errorf("Error creating interface in datapath. Interface {%+v}. Err: %v", intf, err)
		return err
	}

	// save it in db
	na.saveInterface(intf)
	key := na.Solver.ObjectKey(intf.ObjectMeta, intf.TypeMeta)
	na.Lock()
	na.EnicDB[key] = intf
	na.Unlock()
	err = na.Store.Write(intf)

	return err
}

// FindInterface finds an interface in local db
func (na *Nagent) FindInterface(meta api.ObjectMeta) (*netproto.Interface, error) {
	typeMeta := api.TypeMeta{
		Kind: "Interface",
	}

	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	tn, ok := na.EnicDB[key]
	if !ok {
		return nil, fmt.Errorf("interface not found %v", meta.Name)
	}

	return tn, nil
}

// ListInterface returns the list of interfaces
func (na *Nagent) ListInterface() []*netproto.Interface {
	var intfList []*netproto.Interface
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, intf := range na.EnicDB {
		intfList = append(intfList, intf)
	}

	return intfList
}

// ListHwInterface returns the list of interfaces
func (na *Nagent) ListHwInterface() []*netproto.Interface {
	var intfList []*netproto.Interface
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, intf := range na.HwIfDB {
		intfList = append(intfList, intf)
	}

	return intfList
}

// UpdateInterface updates an interface
func (na *Nagent) UpdateInterface(intf *netproto.Interface) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(intf.ObjectMeta)
	if err != nil {
		return err
	}

	existingIntf, err := na.FindInterface(intf.ObjectMeta)
	if err != nil {
		log.Errorf("Interface %v not found", intf.ObjectMeta)
		return err
	}

	if proto.Equal(intf, existingIntf) {
		log.Infof("Nothing to update.")
		return nil
	}

	// save it in db
	na.saveInterface(intf)
	err = na.Datapath.UpdateInterface(intf)
	key := na.Solver.ObjectKey(intf.ObjectMeta, intf.TypeMeta)
	na.Lock()
	na.EnicDB[key] = intf
	na.Unlock()
	err = na.Store.Write(intf)
	return err
}

// DeleteInterface deletes an interface
func (na *Nagent) DeleteInterface(tn, namespace, name string) error {
	intf := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(intf.Kind, intf.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	_, err = na.FindNamespace(intf.ObjectMeta)
	if err != nil {
		return err
	}

	existingIntf, err := na.FindInterface(intf.ObjectMeta)
	if err != nil {
		log.Errorf("Interface %+v not found", intf.ObjectMeta)
		return errors.New("interface not found")
	}

	// delete it in the datapath
	err = na.Datapath.DeleteInterface(existingIntf)
	if err != nil {
		log.Errorf("Error deleting interface {%+v}. Err: %v", intf, err)
	}

	// delete from db
	na.discardInterface(intf)
	key := na.Solver.ObjectKey(intf.ObjectMeta, intf.TypeMeta)
	na.Lock()
	delete(na.EnicDB, key)
	na.Unlock()
	err = na.Store.Delete(intf)

	return err
}

// saveInterface saves interface to state stores
func (na *Nagent) saveInterface(intf *netproto.Interface) error {
	key := na.Solver.ObjectKey(intf.ObjectMeta, intf.TypeMeta)

	// write to delphi
	if na.DelphiClient != nil {
		dintf := dnetproto.Interface{
			Key:       key,
			Interface: intf,
		}

		err := na.DelphiClient.SetObject(&dintf)
		if err != nil {
			log.Errorf("Error writing Interface %s to delphi. Err: %v", key, err)
			return err
		}
	}

	return na.Store.Write(intf)
}

// discardInterface deletes interface from state stores
func (na *Nagent) discardInterface(intf *netproto.Interface) error {
	key := na.Solver.ObjectKey(intf.ObjectMeta, intf.TypeMeta)

	// delete it from delphi
	if na.DelphiClient != nil {
		dintf := dnetproto.Interface{
			Key:       key,
			Interface: intf,
		}

		err := na.DelphiClient.DeleteObject(&dintf)
		if err != nil {
			log.Errorf("Error deleting Interface %s from delphi. Err: %v", key, err)
			return err
		}
	}

	return na.Store.Delete(intf)
}

//GetHwInterfaces queries the datapath interface for uplinks and lifs created and populates the interface DB
func (na *Nagent) GetHwInterfaces() error {
	// LIFs and uplinks can be created outside of Agent's context, by nic mgr. We
	// need the uplinks currently on the HAL to associate with remote EPs
	lifs, ports, err := na.Datapath.ListInterfaces()
	if err != nil {
		return err
	}

	for _, lif := range lifs {
		na.Lock()
		key := na.Solver.ObjectKey(lif.ObjectMeta, lif.TypeMeta)
		na.HwIfDB[key] = lif
		na.Unlock()
		na.saveInterface(lif)
	}

	// Populate Agent state
	if err := na.createPortsAndUplinks(ports); err != nil {
		log.Errorf("could not create {%v} ports. %v", ports, err)
		return err
	}

	for _, port := range ports {
		na.Lock()
		key := na.Solver.ObjectKey(port.ObjectMeta, port.TypeMeta)
		na.PortDB[key] = port
		na.Unlock()
	}

	return nil
}

// findIntfByName looks up either uplinks or lifs from the hw interfaces db
func (na *Nagent) findIntfByName(intfName string) (intf *netproto.Interface, ok bool) {
	lifMeta := api.ObjectMeta{
		Name:      intfName,
		Tenant:    "default",
		Namespace: "default",
	}
	typeMeta := api.TypeMeta{
		Kind: "Interface",
	}
	key := na.Solver.ObjectKey(lifMeta, typeMeta)
	na.Lock()
	intf, ok = na.HwIfDB[key]
	na.Unlock()
	return intf, ok
}

func (na *Nagent) countIntfs(intfName string) (intfCount uint64, err error) {
	na.Lock()
	defer na.Unlock()
	for _, i := range na.HwIfDB {
		if i.Spec.Type == intfName {
			intfCount++
		}
	}
	// prevent divide by 0 in calculating %
	if intfCount == 0 {
		err = errors.New("lif count was 0")
	}
	return
}

func (na *Nagent) getUplinks() (uplinks []*netproto.Interface) {
	na.Lock()
	defer na.Unlock()
	for _, intf := range na.HwIfDB {
		if intf.Spec.Type == "UPLINK_ETH" {
			// get the corresponding port.
			uplinks = append(uplinks, intf)
		}
	}
	return
}

func (na *Nagent) getLifs() (lifs []*netproto.Interface) {
	na.Lock()
	defer na.Unlock()
	for _, intf := range na.HwIfDB {
		if intf.Spec.Type == "LIF" {
			lifs = append(lifs, intf)
		}
	}
	return
}

func (na *Nagent) createPortsAndUplinks(ports []*netproto.Port) error {
	// TODO Use first class port create methods here
	// TODO support breakout

	var uplinks []*netproto.Interface

	for id, p := range ports {
		var uplinkType string
		if p.Spec.Type == "TYPE_MANAGEMENT" {
			uplinkType = "UPLINK_MGMT"
		} else {
			uplinkType = "UPLINK_ETH"
		}
		uplink := &netproto.Interface{
			TypeMeta: api.TypeMeta{
				Kind: "Interface",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      fmt.Sprintf("uplink%d", uint64(id)+types.UplinkOffset+1),
			},
			Spec: netproto.InterfaceSpec{
				Type:        uplinkType,
				AdminStatus: "UP",
			},
			Status: netproto.InterfaceStatus{
				InterfaceID: uint64(id) + types.UplinkOffset + 1, // This will keep uplink IDs consistent across config replays
			},
		}

		uplink.Status.IFUplinkStatus = &netproto.InterfaceUplinkStatus{PortID: uint32(p.Status.PortID)}

		key := na.Solver.ObjectKey(uplink.ObjectMeta, uplink.TypeMeta)

		// save the interface into state stores
		err := na.saveInterface(uplink)
		if err != nil {
			log.Errorf("Error storing uplink %+v to state store. Err: %v", uplink.ObjectMeta, err)
		}

		na.Lock()
		na.HwIfDB[key] = uplink
		na.Unlock()
		uplinks = append(uplinks, uplink)
	}

	if err := na.Datapath.CreateInterface(uplinks...); err != nil {
		log.Errorf("Failed to create Uplinks in Datapath. Err: %v", err)
		return fmt.Errorf("failed to create uplinks in Datapath. Err: %v", err)
	}

	if err := na.Datapath.CreatePort(ports...); err != nil {
		log.Errorf("Failed to create Ports in Datapath. Err: %v", err)
		return fmt.Errorf("failed to create Ports in Datapath. Err: %v", err)
	}

	return nil
}
