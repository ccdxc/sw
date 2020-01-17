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

// CreateNamespace creates a namespace
func (na *Nagent) CreateNamespace(ns *netproto.Namespace) error {
	err := na.validateMeta(ns.Kind, ns.ObjectMeta)
	if err != nil {
		return err
	}
	oldNs, err := na.FindNamespace(ns.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldNs, ns) {
			log.Errorf("Namespace %+v already exists", oldNs)
			return errors.New("namespace already exists")
		}

		log.Infof("Received duplicate namespace create {%+v}", ns)
		return nil
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if ns.Status.NamespaceID == 0 {
		namespaceID, err := na.Store.GetNextID(types.NamespaceID, 0)
		if err != nil {
			log.Errorf("Could not allocate namespace id. {%+v}", err)
			return err
		}
		ns.Status.NamespaceID = namespaceID
	}

	tn, err := na.FindTenant(ns.ObjectMeta)
	if err != nil {
		log.Errorf("Could not find the tenant %v for namespace %v. Err: %v", ns.Tenant, ns.Name, err)
		return err
	}

	// Add a dependency on successful create
	err = na.Solver.Add(tn, ns)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", tn, ns)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(ns.ObjectMeta, ns.TypeMeta)
	na.Lock()
	na.NamespaceDB[key] = ns
	na.Unlock()
	return err
}

// FindNamespace finds a namespace in local db
func (na *Nagent) FindNamespace(ometa api.ObjectMeta) (*netproto.Namespace, error) {
	nsTypeMeta := api.TypeMeta{
		Kind: "Namespace",
	}
	meta := api.ObjectMeta{
		Tenant: ometa.Tenant,
		Name:   ometa.Namespace,
	}
	if meta.Name == "" {
		meta.Name = ometa.Name
	}
	// Find the corresponding tenant
	_, err := na.FindTenant(meta)
	if err != nil {
		log.Errorf("Could not find the tenant: {%+v}", ometa.Tenant)
		return nil, err
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, nsTypeMeta)
	ns, ok := na.NamespaceDB[key]
	if !ok {
		return nil, fmt.Errorf("namespace not found %v", ometa.Namespace)
	}

	return ns, nil
}

// ListNamespace returns the list of namespaces
func (na *Nagent) ListNamespace() []*netproto.Namespace {
	var namespaceList []*netproto.Namespace
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, tn := range na.NamespaceDB {
		namespaceList = append(namespaceList, tn)
	}

	return namespaceList
}

// UpdateNamespace updates a namespace
func (na *Nagent) UpdateNamespace(ns *netproto.Namespace) error {
	existingNs, err := na.FindNamespace(ns.ObjectMeta)
	if err != nil {
		log.Errorf("Namespace %v not found", ns.ObjectMeta)
		return err
	}

	if proto.Equal(ns, existingNs) {
		log.Infof("Nothing to update.")
		return nil
	}

	key := na.Solver.ObjectKey(ns.ObjectMeta, ns.TypeMeta)
	na.Lock()
	na.NamespaceDB[key] = ns
	na.Unlock()
	return na.Store.Write(ns)
}

// DeleteNamespace deletes a namespace
func (na *Nagent) DeleteNamespace(tn, unused, name string) error {
	ns := &netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: tn,
			Name:   name,
		},
	}
	if ns.Name == "default" && ns.Tenant == "default" {
		return errors.New("default namespaces under default tenant cannot be deleted")
	}

	existingNamespace, err := na.FindNamespace(ns.ObjectMeta)
	if err != nil {
		log.Errorf("Namespace %+v not found", ns.ObjectMeta)
		return errors.New("namespace not found")
	}

	// check if the current namespace has any objects referring to it
	err = na.Solver.Solve(existingNamespace)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingNamespace.Name, err)
		return err

	}

	// update the parent references.
	tenant, _ := na.FindTenant(existingNamespace.ObjectMeta)
	err = na.Solver.Remove(tenant, existingNamespace)
	if err != nil {
		log.Errorf("Could not remove the reference to the tenant: %v. Err: %v", existingNamespace.Tenant, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(ns.ObjectMeta, ns.TypeMeta)
	na.Lock()
	delete(na.NamespaceDB, key)
	na.Unlock()

	return err
}
