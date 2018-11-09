// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreatePort creates a port
func (na *Nagent) CreatePort(port *netproto.Port) error {
	err := na.validateMeta(port.Kind, port.ObjectMeta)
	if err != nil {
		return err
	}
	// check if port already exists
	oldPort, err := na.FindPort(port.ObjectMeta)
	if err == nil {
		// check if port contents are same
		if !proto.Equal(oldPort, port) {
			log.Errorf("Port %+v already exists", oldPort)
			return errors.New("port already exists")
		}

		log.Infof("Received duplicate port create for {%+v}", port)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(port.Tenant, port.Namespace)
	if err != nil {
		return err
	}

	// create it in datapath
	port, err = na.Datapath.CreatePort(port)
	if err != nil {
		log.Errorf("Error creating port in datapath. Nw {%+v}. Err: %v", port, err)
		return err
	}

	// Add the current port as a dependency to the namespace.
	err = na.Solver.Add(ns, port)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, port)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(port.ObjectMeta, port.TypeMeta)
	na.Lock()
	na.PortDB[key] = port
	na.Unlock()
	err = na.Store.Write(port)

	return err
}

// ListPort returns the list of ports
func (na *Nagent) ListPort() []*netproto.Port {
	var portList []*netproto.Port

	// lock the db
	na.Lock()
	defer na.Unlock()

	// walk all ports
	for _, rt := range na.PortDB {
		portList = append(portList, rt)
	}

	return portList
}

// FindPort dins a port in local db
func (na *Nagent) FindPort(meta api.ObjectMeta) (*netproto.Port, error) {
	typeMeta := api.TypeMeta{
		Kind: "Port",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	rt, ok := na.PortDB[key]
	if !ok {
		return nil, fmt.Errorf("port not found %v", meta.Name)
	}

	return rt, nil
}

// UpdatePort updates a port.
func (na *Nagent) UpdatePort(port *netproto.Port) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(port.Tenant, port.Namespace)
	if err != nil {
		return err
	}

	existingPort, err := na.FindPort(port.ObjectMeta)
	if err != nil {
		log.Errorf("Port %v not found", port.ObjectMeta)
		return err
	}

	if proto.Equal(port, existingPort) {
		log.Infof("Nothing to update.")
		return nil
	}

	// Set the port ID from the existing port
	port.Status.PortID = existingPort.Status.PortID

	port, err = na.Datapath.UpdatePort(port)
	if err != nil {
		log.Errorf("Error updating the port {%+v} in datapath. Err: %v", existingPort, err)
		return err
	}
	key := na.Solver.ObjectKey(port.ObjectMeta, port.TypeMeta)
	na.Lock()
	na.PortDB[key] = port
	na.Unlock()
	err = na.Store.Write(port)
	return err
}

// DeletePort deletes a port. ToDo implement port deletes in datapath
func (na *Nagent) DeletePort(tn, namespace, name string) error {
	port := &netproto.Port{
		TypeMeta: api.TypeMeta{Kind: "Port"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(port.Kind, port.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(port.Tenant, port.Namespace)
	if err != nil {
		return err
	}

	// check if port already exists
	existingPort, err := na.FindPort(port.ObjectMeta)
	if err != nil {
		log.Errorf("Port %+v not found", port.ObjectMeta)
		return errors.New("port not found")
	}

	// delete the port in datapath
	err = na.Datapath.DeletePort(existingPort)
	if err != nil {
		log.Errorf("Error deleting port {%+v}. Err: %v", existingPort, err)
		return err
	}

	// update parent references
	err = na.Solver.Remove(ns, existingPort)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(port.ObjectMeta, port.TypeMeta)
	na.Lock()
	delete(na.PortDB, key)
	na.Unlock()
	err = na.Store.Delete(port)

	return err
}
