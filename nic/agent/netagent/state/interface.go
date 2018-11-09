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

	ns, err := na.FindNamespace(intf.Tenant, intf.Namespace)
	if err != nil {
		return err
	}

	intfID, err := na.Store.GetNextID(types.InterfaceID)

	if err != nil {
		log.Errorf("Could not allocate interface id. {%+v}", err)
		return err
	}
	intf.Status.InterfaceID = intfID + uplinkOffset

	// Perform interface associations, currently only ENIC interfaces supported as
	switch intf.Spec.Type {
	case "ENIC":
		if len(intf.Spec.LifName) != 0 {
			lif, ok = na.findIntfByName(intf.Spec.LifName)
		} else {
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
		}

		if !ok {
			log.Errorf("could not find user specified lif: {%v}", lif)
			return errors.New("lif not found")
		}
	}

	// create it in datapath
	err = na.Datapath.CreateInterface(intf, lif, nil, ns)
	if err != nil {
		log.Errorf("Error creating interface in datapath. Interface {%+v}. Err: %v", intf, err)
		return err
	}

	// save it in db
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

// UpdateInterface updates an interface
func (na *Nagent) UpdateInterface(intf *netproto.Interface) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(intf.Tenant, intf.Namespace)
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

	err = na.Datapath.UpdateInterface(intf, ns)
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
	ns, err := na.FindNamespace(intf.Tenant, intf.Namespace)
	if err != nil {
		return err
	}

	existingIntf, err := na.FindInterface(intf.ObjectMeta)
	if err != nil {
		log.Errorf("Interface %+v not found", intf.ObjectMeta)
		return errors.New("interface not found")
	}

	// delete it in the datapath
	err = na.Datapath.DeleteInterface(existingIntf, ns)
	if err != nil {
		log.Errorf("Error deleting interface {%+v}. Err: %v", intf, err)
	}

	// delete from db
	key := na.Solver.ObjectKey(intf.ObjectMeta, intf.TypeMeta)
	na.Lock()
	delete(na.EnicDB, key)
	na.Unlock()
	err = na.Store.Delete(intf)

	return err
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
		key := na.Solver.ObjectKey(lif.ObjectMeta, lif.TypeMeta)
		na.Lock()
		na.HwIfDB[key] = lif
		na.Unlock()
	}

	// Populate Agent state
	for _, port := range ports {
		// Create Ports and Uplinks
		err = na.createPortAndUplink(port)
		if err != nil {
			log.Errorf("could not create {%v} ports. %v", port, err)
			return err
		}
		key := na.Solver.ObjectKey(port.ObjectMeta, port.TypeMeta)
		na.Lock()
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

func (na *Nagent) createPortAndUplink(p *netproto.Port) error {
	// TODO Use first class port create methods here
	// TODO support breakout

	var uplinkType string

	err := na.CreatePort(p)
	if err != nil {
		return err
	}
	id, err := na.Store.GetNextID(types.InterfaceID)
	if err != nil {
		log.Errorf("Could not allocate IDs for uplinks. %v", err)
		return fmt.Errorf("could not allocate IDs for uplinks. %v", err)
	}
	id += uint64(uplinkOffset)

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
			Name:      fmt.Sprintf("uplink%d", id),
		},
		Spec: netproto.InterfaceSpec{
			Type:        uplinkType,
			AdminStatus: "UP",
		},
		Status: netproto.InterfaceStatus{
			InterfaceID: id,
		},
	}
	ns, err := na.FindNamespace(uplink.Tenant, uplink.Namespace)
	if err != nil {
		return err
	}
	key := na.Solver.ObjectKey(uplink.ObjectMeta, uplink.TypeMeta)
	na.HwIfDB[key] = uplink
	err = na.Datapath.CreateInterface(uplink, nil, p, ns)
	if err != nil {
		log.Errorf("Failed to create uplinks during init stage. %v", err)
		return err
	}

	return nil
}
