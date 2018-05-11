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

// CreateNamespace creates a namespace
func (na *NetAgent) CreateNamespace(ns *netproto.Namespace) error {

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

	ns.Status.NamespaceID, err = na.store.GetNextID(VrfID)

	if err != nil {
		log.Errorf("Could not allocate namespace id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateVrf(ns.Status.NamespaceID)
	if err != nil {
		log.Errorf("Error creating namespace in datapath. Namespace {%+v}. Err: %v", ns, err)
		return err
	}

	// save it in db
	key := objectKey(ns.ObjectMeta, ns.TypeMeta)
	na.Lock()
	na.namespaceDB[key] = ns
	na.Unlock()
	err = na.store.Write(ns)

	return err
}

// FindNamespace finds a namespace in local db
func (na *NetAgent) FindNamespace(tenant, namespace string) (*netproto.Namespace, error) {
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
	ns, ok := na.namespaceDB[key]
	if !ok {
		return nil, fmt.Errorf("namespace not found %v", namespace)
	}

	return ns, nil
}

// ListNamespace returns the list of namespaces
func (na *NetAgent) ListNamespace() []*netproto.Namespace {
	var namespaceList []*netproto.Namespace
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, tn := range na.namespaceDB {
		namespaceList = append(namespaceList, tn)
	}

	return namespaceList
}

// UpdateNamespace updates a namespace
func (na *NetAgent) UpdateNamespace(ns *netproto.Namespace) error {
	existingNs, err := na.FindNamespace(ns.Tenant, ns.Name)
	if err != nil {
		log.Errorf("Namespace %v not found", ns.ObjectMeta)
		return err
	}

	if proto.Equal(ns, existingNs) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.datapath.UpdateVrf(ns.Status.NamespaceID)
	key := objectKey(ns.ObjectMeta, ns.TypeMeta)
	na.Lock()
	na.namespaceDB[key] = ns
	na.Unlock()
	err = na.store.Write(ns)
	return err
}

// DeleteNamespace deletes a namespace
func (na *NetAgent) DeleteNamespace(ns *netproto.Namespace) error {
	if ns.Name == "default" {
		return errors.New("default namespaces can not be deleted")
	}

	existingNamespace, err := na.FindNamespace(ns.Tenant, ns.Name)
	if err != nil {
		log.Errorf("Namespace %+v not found", ns.ObjectMeta)
		return errors.New("namespace not found")
	}

	// delete it in the datapath
	err = na.datapath.DeleteVrf(existingNamespace.Status.NamespaceID)
	if err != nil {
		log.Errorf("Error deleting namespace {%+v}. Err: %v", ns, err)
	}

	// delete from db
	key := objectKey(ns.ObjectMeta, ns.TypeMeta)
	na.Lock()
	delete(na.namespaceDB, key)
	na.Unlock()
	err = na.store.Delete(ns)

	return err
}
