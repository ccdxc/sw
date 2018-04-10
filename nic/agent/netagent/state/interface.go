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

// CreateInterface creates an interface
func (na *NetAgent) CreateInterface(intf *netproto.Interface) error {
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

	intf.Status.InterfaceID, err = na.store.GetNextID(InterfaceID)

	if err != nil {
		log.Errorf("Could not allocate interface id. {%+v}", err)
		return err
	}

	// find the corresponding tenant
	tnMeta := api.ObjectMeta{
		Name:   intf.Tenant,
		Tenant: intf.Tenant,
	}

	tn, err := na.FindTenant(tnMeta)
	if err != nil {
		log.Errorf("Could not find the tenant: {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateInterface(intf, tn)
	if err != nil {
		log.Errorf("Error creating interface in datapath. Interface {%+v}. Err: %v", intf, err)
		return err
	}

	// save it in db
	key := objectKey(intf.ObjectMeta)
	na.Lock()
	na.interfaceDB[key] = intf
	na.Unlock()
	err = na.store.Write(intf)

	return err
}

// FindInterface finds an interface in local db
func (na *NetAgent) FindInterface(meta api.ObjectMeta) (*netproto.Interface, error) {
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta)
	tn, ok := na.interfaceDB[key]
	if !ok {
		return nil, fmt.Errorf("interface not found %v", tn)
	}

	return tn, nil
}

// ListInterface returns the list of interfaces
func (na *NetAgent) ListInterface() []*netproto.Interface {
	var intfList []*netproto.Interface
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, intf := range na.interfaceDB {
		intfList = append(intfList, intf)
	}

	return intfList
}

// UpdateInterface updates an interface
func (na *NetAgent) UpdateInterface(intf *netproto.Interface) error {
	existingIntf, err := na.FindInterface(intf.ObjectMeta)
	if err != nil {
		log.Errorf("Interface %v not found", intf.ObjectMeta)
		return err
	}

	if proto.Equal(intf, existingIntf) {
		log.Infof("Nothing to update.")
		return nil
	}

	// find the corresponding tenant
	tnMeta := api.ObjectMeta{
		Name:   intf.Tenant,
		Tenant: intf.Tenant,
	}

	tn, err := na.FindTenant(tnMeta)
	if err != nil {
		log.Errorf("Could not find the tenant: {%+v}", err)
		return err
	}

	err = na.datapath.UpdateInterface(intf, tn)
	key := objectKey(intf.ObjectMeta)
	na.Lock()
	na.interfaceDB[key] = intf
	na.Unlock()
	err = na.store.Write(intf)
	return err
}

// DeleteInterface deletes an interface
func (na *NetAgent) DeleteInterface(intf *netproto.Interface) error {
	existingIntf, err := na.FindInterface(intf.ObjectMeta)
	if err != nil {
		log.Errorf("Interface %+v not found", intf.ObjectMeta)
		return errors.New("interface not found")
	}

	// find the corresponding tenant
	tnMeta := api.ObjectMeta{
		Name:   intf.Tenant,
		Tenant: intf.Tenant,
	}

	tn, err := na.FindTenant(tnMeta)
	if err != nil {
		log.Errorf("Could not find the tenant: {%+v}", err)
		return err
	}

	// delete it in the datapath
	err = na.datapath.DeleteInterface(existingIntf, tn)
	if err != nil {
		log.Errorf("Error deleting interface {%+v}. Err: %v", intf, err)
	}

	// delete from db
	key := objectKey(intf.ObjectMeta)
	na.Lock()
	delete(na.interfaceDB, key)
	na.Unlock()
	err = na.store.Delete(intf)

	return err
}

//GetHwInterfaces queries the datapath interface for uplinks and lifs created and populates the interface DB
func (na *NetAgent) GetHwInterfaces() error {
	// LIFs and uplinks can be created outside of Agent's context, by nic mgr. We
	// need the uplinks currently on the HAL to associate with remote EPs
	lifs, uplinks, err := na.datapath.ListInterfaces()
	if err != nil {
		return err
	}
	for i, lif := range lifs.Response {
		l := &netproto.Interface{
			TypeMeta: api.TypeMeta{
				Kind: "Interface",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant: "default",
				Name:   fmt.Sprintf("default-lif-%d", i),
			},
			Spec: netproto.InterfaceSpec{
				Type: "LIF",
			},
			Status: netproto.InterfaceStatus{
				InterfaceID: lif.Spec.KeyOrHandle.GetLifId(),
			},
		}
		key := objectKey(l.ObjectMeta)
		na.Lock()
		na.interfaceDB[key] = l
		na.Unlock()
	}

	for i, uplink := range uplinks.Response {
		l := &netproto.Interface{
			TypeMeta: api.TypeMeta{
				Kind: "Interface",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant: "default",
				Name:   fmt.Sprintf("default-lif-%d", i),
			},
			Spec: netproto.InterfaceSpec{
				Type: "UPLINK",
			},
			Status: netproto.InterfaceStatus{
				InterfaceID: uplink.Spec.KeyOrHandle.GetInterfaceId(),
			},
		}
		key := objectKey(l.ObjectMeta)
		na.Lock()
		na.interfaceDB[key] = l
		na.Unlock()
	}

	return nil
}
