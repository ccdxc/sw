// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"
	"time"

	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateTenant creates a tenant
func (na *NetAgent) CreateTenant(tn *netproto.Tenant) error {
	err := na.validateMeta(tn.Kind, tn.ObjectMeta)
	if err != nil {
		return err
	}
	oldTn, err := na.FindTenant(tn.ObjectMeta.Name)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldTn, tn) {
			log.Errorf("Tenant %+v already exists", oldTn)
			return errors.New("tenant already exists")
		}

		log.Infof("Received duplicate tenant create {%+v}", tn)
		return nil
	}

	tn.Status.TenantID, err = na.store.GetNextID(VrfID)

	if err != nil {
		log.Errorf("Could not allocate tenant id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateVrf(tn.Status.TenantID)
	if err != nil {
		log.Errorf("Error creating tenant in datapath. Tenant {%+v}. Err: %v", tn, err)
		return err
	}

	// save it in db
	key := objectKey(tn.ObjectMeta, tn.TypeMeta)
	na.Lock()
	na.tenantDB[key] = tn
	na.Unlock()
	err = na.store.Write(tn)
	if err != nil {
		log.Errorf("Could not persist tenant object to the store. %v", err)
		return err
	}

	c, _ := types.TimestampProto(time.Now())
	// Create a default namespace for every tenant
	defaultNS := &netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: tn.Name,
			Name:   "default",
			CreationTime: api.Timestamp{
				Timestamp: *c,
			},
			ModTime: api.Timestamp{
				Timestamp: *c,
			},
		},
	}
	return na.CreateNamespace(defaultNS)
}

// FindTenant finds a tenant in local db
func (na *NetAgent) FindTenant(tenant string) (*netproto.Tenant, error) {
	meta := api.ObjectMeta{
		Name: tenant,
	}
	typeMeta := api.TypeMeta{
		Kind: "Tenant",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	tn, ok := na.tenantDB[key]
	if !ok {
		return nil, fmt.Errorf("tenant not found %v", tn)
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
	existingTn, err := na.FindTenant(tn.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Tenant %v not found", tn.ObjectMeta)
		return err
	}

	if proto.Equal(tn, existingTn) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.datapath.UpdateVrf(tn.Status.TenantID)
	key := objectKey(tn.ObjectMeta, tn.TypeMeta)
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

	// delete the default namespace under the tenant first
	// Create a default namespace for every tenant
	defaultNS := &netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: tn.Name,
			Name:   "default",
		},
	}
	// ignore error if the default namespace under non-default tenant is already deleted
	na.DeleteNamespace(defaultNS)

	existingTenant, err := na.FindTenant(tn.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Tenant %+v not found", tn.ObjectMeta)
		return errors.New("tenant not found")
	}

	// delete it in the datapath
	err = na.datapath.DeleteVrf(existingTenant.Status.TenantID)
	if err != nil {
		log.Errorf("Error deleting tenant {%+v}. Err: %v", tn, err)
	}

	// delete from db
	key := objectKey(tn.ObjectMeta, tn.TypeMeta)
	na.Lock()
	delete(na.tenantDB, key)
	na.Unlock()
	err = na.store.Delete(tn)

	return err
}
