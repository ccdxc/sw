// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"
	"time"

	"github.com/gogo/protobuf/proto"
	gogoproto "github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateTenant creates a tenant
func (na *Nagent) CreateTenant(tn *netproto.Tenant) error {
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

	tn.Status.TenantID, err = na.Store.GetNextID(types.VrfID)

	if err != nil {
		log.Errorf("Could not allocate tenant id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateVrf(tn.Status.TenantID, "")
	if err != nil {
		log.Errorf("Error creating tenant in datapath. Tenant {%+v}. Err: %v", tn, err)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(tn.ObjectMeta, tn.TypeMeta)
	na.Lock()
	na.TenantDB[key] = tn
	na.Unlock()
	err = na.Store.Write(tn)
	if err != nil {
		log.Errorf("Could not persist tenant object to the store. %v", err)
		return err
	}

	c, _ := gogoproto.TimestampProto(time.Now())
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
func (na *Nagent) FindTenant(tenant string) (*netproto.Tenant, error) {
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
	key := na.Solver.ObjectKey(meta, typeMeta)
	tn, ok := na.TenantDB[key]
	if !ok {
		return nil, fmt.Errorf("tenant not found %v", tn)
	}

	return tn, nil
}

// ListTenant returns the list of tenants
func (na *Nagent) ListTenant() []*netproto.Tenant {
	var tenantList []*netproto.Tenant
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, tn := range na.TenantDB {
		tenantList = append(tenantList, tn)
	}

	return tenantList
}

// UpdateTenant updates a tenant
func (na *Nagent) UpdateTenant(tn *netproto.Tenant) error {
	existingTn, err := na.FindTenant(tn.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Tenant %v not found", tn.ObjectMeta)
		return err
	}

	if proto.Equal(tn, existingTn) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.Datapath.UpdateVrf(tn.Status.TenantID)
	key := na.Solver.ObjectKey(tn.ObjectMeta, tn.TypeMeta)
	na.Lock()
	na.TenantDB[key] = tn
	na.Unlock()
	err = na.Store.Write(tn)
	return err
}

// DeleteTenant deletes a tenant
func (na *Nagent) DeleteTenant(tn *netproto.Tenant) error {
	if tn.Name == "default" {
		return errors.New("default tenants can not be deleted")
	}

	existingTenant, err := na.FindTenant(tn.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Tenant %+v not found", tn.ObjectMeta)
		return errors.New("tenant not found")
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

	// check if the current tenant has any objects referring to it
	err = na.Solver.Solve(existingTenant)

	// err is non nil even for default namespaces under the tenant. In such case, we
	// ignore the err as the default ns will be automatically deleted on a tenant delete.
	delErr := checkForDefaultNS(err, existingTenant.Name)

	if delErr != nil {
		log.Errorf("Found active references to %v. Err: %v", existingTenant.Name, err)
		return err
	}

	err = na.DeleteNamespace(defaultNS)
	if err != nil {
		log.Errorf("Failed to delete default namespace under %v tenant. Err: %v", existingTenant.Name, err)
		return err
	}

	// clear for deletion, call datapath delete
	err = na.Datapath.DeleteVrf(existingTenant.Status.TenantID)
	if err != nil {
		log.Errorf("Error deleting tenant {%+v}. Err: %v", tn, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(tn.ObjectMeta, tn.TypeMeta)
	na.Lock()
	delete(na.TenantDB, key)
	na.Unlock()
	err = na.Store.Delete(tn)

	return err
}

func checkForDefaultNS(err error, tenantName string) error {
	delErr, ok := err.(*types.ErrCannotDelete)
	defaultNS := fmt.Sprintf("/api/namespaces/%v/default", tenantName)
	// check if the err is of type ErrCannotDelete, has just the default ns dependency
	if ok && len(delErr.References) == 1 && delErr.References[0] == defaultNS {
		// benign error, can be ignored
		return nil
	}
	// return the unmodified error
	return err
}
