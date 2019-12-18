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

// CreateRoute creates a route
func (na *Nagent) CreateRoute(rt *netproto.Route) error {
	err := na.validateMeta(rt.Kind, rt.ObjectMeta)
	if err != nil {
		return err
	}
	// check if route already exists
	oldRt, err := na.FindRoute(rt.ObjectMeta)
	if err == nil {
		// check if route contents are same
		if !proto.Equal(oldRt, rt) {
			log.Errorf("Route %+v already exists", oldRt)
			return errors.New("route already exists")
		}

		log.Infof("Received duplicate route create for {%+v}", rt)
		return nil
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(rt.ObjectMeta)
	if err != nil {
		return err
	}

	// find the corresponding vrf for the route
	vrf, err := na.ValidateVrf(rt.Tenant, rt.Namespace, rt.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", rt.Spec.VrfName)
		return err
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if rt.Status.RouteID == 0 {
		rt.Status.RouteID, err = na.Store.GetNextID(types.RouteID)
	}

	if err != nil {
		log.Errorf("Could not allocate route id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateRoute(rt, vrf)
	if err != nil {
		log.Errorf("Error creating route in datapath. Nw {%+v}. Err: %v", rt, err)
		return err
	}

	// Add the current route as a dependency to the vrf.
	err = na.Solver.Add(vrf, rt)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", vrf, rt)
		return err
	}

	// Add the current route as a dependency to the namespace.
	err = na.Solver.Add(ns, rt)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, rt)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(rt.ObjectMeta, rt.TypeMeta)
	na.Lock()
	na.RouteDB[key] = rt
	na.Unlock()
	err = na.Store.Write(rt)

	return err
}

// ListRoute returns the list of routes
func (na *Nagent) ListRoute() []*netproto.Route {
	var routeList []*netproto.Route

	// lock the db
	na.Lock()
	defer na.Unlock()

	// walk all routes
	for _, rt := range na.RouteDB {
		routeList = append(routeList, rt)
	}

	return routeList
}

// FindRoute dins a route in local db
func (na *Nagent) FindRoute(meta api.ObjectMeta) (*netproto.Route, error) {
	typeMeta := api.TypeMeta{
		Kind: "Route",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	rt, ok := na.RouteDB[key]
	if !ok {
		return nil, fmt.Errorf("route not found %v", meta.Name)
	}

	return rt, nil
}

// UpdateRoute updates a route. ToDo implement route updates in datapath
func (na *Nagent) UpdateRoute(rt *netproto.Route) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(rt.ObjectMeta)
	if err != nil {
		return err
	}

	existingRoute, err := na.FindRoute(rt.ObjectMeta)
	if err != nil {
		log.Errorf("Route %v not found", rt.ObjectMeta)
		return err
	}

	if proto.Equal(rt, existingRoute) {
		log.Infof("Nothing to update.")
		return nil
	}
	// find the corresponding vrf for the route
	vrf, err := na.ValidateVrf(existingRoute.Tenant, existingRoute.Namespace, existingRoute.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingRoute.Spec.VrfName)
		return err
	}

	err = na.Datapath.UpdateRoute(existingRoute, vrf)
	if err != nil {
		log.Errorf("Error updating the route {%+v} in datapath. Err: %v", existingRoute, err)
		return err
	}
	key := na.Solver.ObjectKey(rt.ObjectMeta, rt.TypeMeta)
	na.Lock()
	na.RouteDB[key] = rt
	na.Unlock()
	err = na.Store.Write(rt)
	return err
}

// DeleteRoute deletes a route. ToDo implement route deletes in datapath
func (na *Nagent) DeleteRoute(tn, namespace, name string) error {
	rt := &netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(rt.Kind, rt.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(rt.ObjectMeta)
	if err != nil {
		return err
	}

	// check if route already exists
	existingRoute, err := na.FindRoute(rt.ObjectMeta)
	if err != nil {
		log.Errorf("Route %+v not found", rt.ObjectMeta)
		return errors.New("route not found")
	}

	// find the corresponding vrf for the route
	vrf, err := na.ValidateVrf(existingRoute.Tenant, existingRoute.Namespace, existingRoute.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingRoute.Spec.VrfName)
		return err
	}

	// delete the route in datapath
	err = na.Datapath.DeleteRoute(existingRoute, vrf)
	if err != nil {
		log.Errorf("Error deleting route {%+v}. Err: %v", existingRoute, err)
		return err
	}

	err = na.Solver.Remove(vrf, existingRoute)
	if err != nil {
		log.Errorf("Could not remove the reference to the vrf: %v. Err: %v", existingRoute.Spec.VrfName, err)
		return err
	}

	// update parent references
	err = na.Solver.Remove(ns, existingRoute)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(rt.ObjectMeta, rt.TypeMeta)
	na.Lock()
	delete(na.RouteDB, key)
	na.Unlock()
	err = na.Store.Delete(rt)

	return err
}
