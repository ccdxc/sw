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

	intfID, err := na.store.GetNextID(InterfaceID)

	if err != nil {
		log.Errorf("Could not allocate interface id. {%+v}", err)
		return err
	}
	intf.Status.InterfaceID = intfID + maxNumUplinks

	// find the corresponding tenant
	tnMeta := api.ObjectMeta{
		Name:      intf.Tenant,
		Namespace: intf.Namespace,
		Tenant:    intf.Tenant,
	}

	tn, err := na.FindTenant(tnMeta)
	if err != nil {
		log.Errorf("Could not find the tenant: {%+v}", err)
		return err
	}

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
			lif, ok = na.findIntfByName(fmt.Sprintf("default-lif-%d", intf.Status.InterfaceID%lifCount))
		}

		if !ok {
			log.Errorf("could not find user specified lif: {%v}", lif)
			return errors.New("lif not found")
		}
	}

	// create it in datapath
	err = na.datapath.CreateInterface(intf, lif, tn)
	if err != nil {
		log.Errorf("Error creating interface in datapath. Interface {%+v}. Err: %v", intf, err)
		return err
	}

	// save it in db
	key := objectKey(intf.ObjectMeta)
	na.Lock()
	na.enicDB[key] = intf
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
	tn, ok := na.enicDB[key]
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

	for _, intf := range na.enicDB {
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
		Name:      intf.Tenant,
		Namespace: intf.Namespace,
		Tenant:    intf.Tenant,
	}

	tn, err := na.FindTenant(tnMeta)
	if err != nil {
		log.Errorf("Could not find the tenant: {%+v}", err)
		return err
	}

	err = na.datapath.UpdateInterface(intf, tn)
	key := objectKey(intf.ObjectMeta)
	na.Lock()
	na.enicDB[key] = intf
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
		Name:      intf.Tenant,
		Namespace: intf.Namespace,
		Tenant:    intf.Tenant,
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
	delete(na.enicDB, key)
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
				Tenant:    "default",
				Namespace: "default",
				Name:      fmt.Sprintf("default-lif-%d", i),
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
		na.hwIfDB[key] = l
		na.Unlock()
	}

	for i, uplink := range uplinks.Response {
		u := &netproto.Interface{
			TypeMeta: api.TypeMeta{
				Kind: "Interface",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      fmt.Sprintf("default-uplink-%d", i),
			},
			Spec: netproto.InterfaceSpec{
				Type: "UPLINK",
			},
			Status: netproto.InterfaceStatus{
				InterfaceID: uplink.Spec.KeyOrHandle.GetInterfaceId(),
			},
		}
		key := objectKey(u.ObjectMeta)
		na.Lock()
		na.hwIfDB[key] = u
		na.Unlock()
	}

	return nil
}

// findIntfByName looks up either uplinks or lifs from the hw interfaces db
func (na *NetAgent) findIntfByName(intfName string) (intf *netproto.Interface, ok bool) {
	lifMeta := api.ObjectMeta{
		Name:      intfName,
		Tenant:    "default",
		Namespace: "default",
	}
	key := objectKey(lifMeta)
	na.Lock()
	intf, ok = na.hwIfDB[key]
	na.Unlock()
	return
}

func (na *NetAgent) countIntfs(intfName string) (intfCount uint64, err error) {
	na.Lock()
	defer na.Unlock()
	for _, i := range na.hwIfDB {
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
