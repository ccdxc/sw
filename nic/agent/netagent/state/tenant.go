// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateTenant creates a tenant
func (na *NetAgent) CreateTenant(tn *netproto.Tenant) error {
	oldTn, err := na.FindTenant(tn.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldTn, tn) {
			log.Errorf("Tenant %+v already exists", oldTn)
			return errors.New("tenant already exists")
		}

		log.Infof("Received duplicate tenant create {%+v}", tn)
		return nil
	}

	tn.Status.TenantID, err = na.store.GetNextID(TenantID)

	if err != nil {
		log.Errorf("Could not allocate tenant id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateTenant(tn)
	if err != nil {
		log.Errorf("Error creating tenant in datapath. Tenant {%+v}. Err: %v", tn, err)
		return err
	}

	// save it in db
	key := objectKey(tn.ObjectMeta)
	na.Lock()
	na.tenantDB[key] = tn
	na.Unlock()
	err = na.store.Write(tn)

	return err
}

// FindTenant finds a tenant in local db
func (na *NetAgent) FindTenant(meta api.ObjectMeta) (*netproto.Tenant, error) {
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta)
	tn, ok := na.tenantDB[key]
	if !ok {
		return nil, errors.New("tenant not found")
	}

	return tn, nil
}

// ListTenant returns the list of tenants
func (na *NetAgent) ListTenant() []*netproto.Tenant {
	var tenantList []*netproto.Tenant
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, tn := range na.tenantDB {
		tenantList = append(tenantList, tn)
	}

	return tenantList
}

// UpdateTenant updates a tenant
func (na *NetAgent) UpdateTenant(tn *netproto.Tenant) error {
	existingTn, err := na.FindTenant(tn.ObjectMeta)
	if err != nil {
		log.Errorf("Tenant %v not found", tn.ObjectMeta)
		return err
	}

	if proto.Equal(tn, existingTn) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.datapath.UpdateTenant(tn)
	key := objectKey(tn.ObjectMeta)
	na.Lock()
	na.tenantDB[key] = tn
	na.Unlock()
	err = na.store.Write(tn)
	return err
}

// DeleteTenant deletes a tenant
func (na *NetAgent) DeleteTenant(tn *netproto.Tenant) error {
	if tn.Name == "default" {
		return errors.New("default tenants can not be deleted")
	}

	existingTenant, err := na.FindTenant(tn.ObjectMeta)
	if err != nil {
		log.Errorf("Tenant %+v not found", tn.ObjectMeta)
		return errors.New("tenant not found")
	}

	// delete it in the datapath
	err = na.datapath.DeleteTenant(existingTenant)
	if err != nil {
		log.Errorf("Error deleting tenant {%+v}. Err: %v", tn, err)
	}

	// delete from db
	key := objectKey(tn.ObjectMeta)
	na.Lock()
	delete(na.tenantDB, key)
	na.Unlock()
	err = na.store.Delete(tn)

	return err
}
