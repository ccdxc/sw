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
	ns, err := na.FindNamespace(rt.Tenant, rt.Namespace)
	if err != nil {
		return err
	}

	rt.Status.RouteID, err = na.Store.GetNextID(types.RouteID)
	if err != nil {
		log.Errorf("Could not allocate route id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateRoute(rt, ns)
	if err != nil {
		log.Errorf("Error creating route in datapath. Nw {%+v}. Err: %v", rt, err)
		return err
	}

	// save it in db
	key := objectKey(rt.ObjectMeta, rt.TypeMeta)
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
	key := objectKey(meta, typeMeta)
	rt, ok := na.RouteDB[key]
	if !ok {
		return nil, fmt.Errorf("route not found %v", meta.Name)
	}

	return rt, nil
}

// UpdateRoute updates a route. ToDo implement route updates in datapath
func (na *Nagent) UpdateRoute(rt *netproto.Route) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(rt.Tenant, rt.Namespace)
	if err != nil {
		return err
	}

	oldRt, err := na.FindRoute(rt.ObjectMeta)
	if err != nil {
		log.Errorf("Route %v not found", rt.ObjectMeta)
		return err
	}

	if proto.Equal(rt, oldRt) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.Datapath.UpdateRoute(rt, ns)
	key := objectKey(rt.ObjectMeta, rt.TypeMeta)
	na.Lock()
	na.RouteDB[key] = rt
	na.Unlock()
	err = na.Store.Write(rt)
	return err
}

// DeleteRoute deletes a route. ToDo implement route deletes in datapath
func (na *Nagent) DeleteRoute(rt *netproto.Route) error {
	err := na.validateMeta(rt.Kind, rt.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(rt.Tenant, rt.Namespace)
	if err != nil {
		return err
	}

	// check if route already exists
	route, err := na.FindRoute(rt.ObjectMeta)
	if err != nil {
		log.Errorf("Route %+v not found", rt.ObjectMeta)
		return errors.New("route not found")
	}

	// delete the route in datapath
	err = na.Datapath.DeleteRoute(route, ns)
	if err != nil {
		log.Errorf("Error deleting route {%+v}. Err: %v", rt, err)
	}

	// delete from db
	key := objectKey(rt.ObjectMeta, rt.TypeMeta)
	na.Lock()
	delete(na.RouteDB, key)
	na.Unlock()
	err = na.Store.Delete(rt)

	return err
}
