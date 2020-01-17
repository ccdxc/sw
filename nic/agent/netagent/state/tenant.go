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
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateTenant creates a tenant
func (na *Nagent) CreateTenant(tn *netproto.Tenant) error {
	err := na.validateMeta(tn.Kind, tn.ObjectMeta)
	if err != nil {
		return err
	}
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

	// Allocate ID only on first object creates and use existing ones during config replay
	if tn.Status.TenantID == 0 {
		tenantID, err := na.Store.GetNextID(types.TenantID, 0)
		if err != nil {
			log.Errorf("Could not allocate tenant id. {%+v}", err)
			return err
		}
		tn.Status.TenantID = tenantID
	}

	// save it in db
	key := na.Solver.ObjectKey(tn.ObjectMeta, tn.TypeMeta)
	na.Lock()
	na.TenantDB[key] = tn
	na.Unlock()

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
		Status: netproto.NamespaceStatus{NamespaceID: 1},
	}
	return na.CreateNamespace(defaultNS)
}

// FindTenant finds a tenant in local db
func (na *Nagent) FindTenant(ometa api.ObjectMeta) (*netproto.Tenant, error) {
	meta := api.ObjectMeta{
		Name: ometa.Tenant,
	}
	if meta.Name == "" {
		meta.Name = ometa.Name
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
		return nil, fmt.Errorf("tenant not found %v", key)
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
	existingTn, err := na.FindTenant(tn.ObjectMeta)
	if err != nil {
		log.Errorf("Tenant %v not found", tn.ObjectMeta)
		return err
	}

	if proto.Equal(tn, existingTn) {
		log.Infof("Nothing to update.")
		return nil
	}

	key := na.Solver.ObjectKey(tn.ObjectMeta, tn.TypeMeta)
	na.Lock()
	na.TenantDB[key] = tn
	na.Unlock()
	dat, _ := tn.Marshal()
	err = na.Store.RawWrite(tn.GetKind(), tn.GetKey(), dat)
	return err
}

// DeleteTenant deletes a tenant
func (na *Nagent) DeleteTenant(unused1, unused2, name string) error {
	tn := &netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
	}
	if tn.Name == "default" {
		return errors.New("default tenants can not be deleted")
	}

	existingTenant, err := na.FindTenant(tn.ObjectMeta)
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

	err = na.DeleteNamespace(defaultNS.Tenant, defaultNS.Name, defaultNS.Name)
	if err != nil {
		log.Errorf("Failed to delete default namespace under %v tenant. Err: %v", existingTenant.Name, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(tn.ObjectMeta, tn.TypeMeta)
	na.Lock()
	delete(na.TenantDB, key)
	na.Unlock()
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
