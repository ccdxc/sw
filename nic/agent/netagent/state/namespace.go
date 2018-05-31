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

// CreateNamespace creates a namespace
func (na *Nagent) CreateNamespace(ns *netproto.Namespace) error {
	err := na.validateMeta(ns.Kind, ns.ObjectMeta)
	if err != nil {
		return err
	}
	oldNs, err := na.FindNamespace(ns.Tenant, ns.Name)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldNs, ns) {
			log.Errorf("Namespace %+v already exists", oldNs)
			return errors.New("namespace already exists")
		}

		log.Infof("Received duplicate namespace create {%+v}", ns)
		return nil
	}

	ns.Status.NamespaceID, err = na.Store.GetNextID(types.VrfID)

	if err != nil {
		log.Errorf("Could not allocate namespace id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateVrf(ns.Status.NamespaceID, "")
	if err != nil {
		log.Errorf("Error creating namespace in datapath. Namespace {%+v}. Err: %v", ns, err)
		return err
	}

	// save it in db
	key := objectKey(ns.ObjectMeta, ns.TypeMeta)
	na.Lock()
	na.NamespaceDB[key] = ns
	na.Unlock()
	err = na.Store.Write(ns)

	return err
}

// FindNamespace finds a namespace in local db
func (na *Nagent) FindNamespace(tenant, namespace string) (*netproto.Namespace, error) {
	nsTypeMeta := api.TypeMeta{
		Kind: "Namespace",
	}
	meta := api.ObjectMeta{
		Tenant: tenant,
		Name:   namespace,
	}
	// Find the corresponding tenant
	_, err := na.FindTenant(meta.Tenant)
	if err != nil {
		log.Errorf("Could not find the tenant: {%+v}", tenant)
		return nil, err
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, nsTypeMeta)
	ns, ok := na.NamespaceDB[key]
	if !ok {
		return nil, fmt.Errorf("namespace not found %v", namespace)
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
	existingNs, err := na.FindNamespace(ns.Tenant, ns.Name)
	if err != nil {
		log.Errorf("Namespace %v not found", ns.ObjectMeta)
		return err
	}

	if proto.Equal(ns, existingNs) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.Datapath.UpdateVrf(ns.Status.NamespaceID)
	key := objectKey(ns.ObjectMeta, ns.TypeMeta)
	na.Lock()
	na.NamespaceDB[key] = ns
	na.Unlock()
	err = na.Store.Write(ns)
	return err
}

// DeleteNamespace deletes a namespace
func (na *Nagent) DeleteNamespace(ns *netproto.Namespace) error {
	if ns.Name == "default" && ns.Tenant == "default" {
		return errors.New("default namespaces under default tenant cannot be deleted")
	}

	existingNamespace, err := na.FindNamespace(ns.Tenant, ns.Name)
	if err != nil {
		log.Errorf("Namespace %+v not found", ns.ObjectMeta)
		return errors.New("namespace not found")
	}

	// delete it in the datapath
	err = na.Datapath.DeleteVrf(existingNamespace.Status.NamespaceID)
	if err != nil {
		log.Errorf("Error deleting namespace {%+v}. Err: %v", ns, err)
	}

	// delete from db
	key := objectKey(ns.ObjectMeta, ns.TypeMeta)
	na.Lock()
	delete(na.NamespaceDB, key)
	na.Unlock()
	err = na.Store.Delete(ns)

	return err
}
