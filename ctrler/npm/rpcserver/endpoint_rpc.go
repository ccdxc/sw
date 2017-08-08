// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"errors"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/ctrler/npm/statemgr"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/memdb"
	"golang.org/x/net/context"
)

// EndpointRPCHandler is the endpoint RPC server
type EndpointRPCHandler struct {
	stateMgr *statemgr.Statemgr // reference to statemgr
}

// CreateEndpoint creates endpoint
func (ep *EndpointRPCHandler) CreateEndpoint(ctx context.Context, epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	log.Infof("Received CreateEndpoint: {%+v}", epinfo)

	// find the network
	nw, err := ep.stateMgr.FindNetwork(epinfo.ObjectMeta.Tenant, epinfo.Spec.NetworkName)
	if err != nil {
		log.Errorf("Could not find network %s|%s", epinfo.ObjectMeta.Tenant, epinfo.Spec.NetworkName)
		return nil, fmt.Errorf("Could not find the network")
	}

	// build the endpoint params
	epp := network.Endpoint{
		TypeMeta:   epinfo.TypeMeta,
		ObjectMeta: epinfo.ObjectMeta,
		Status: network.EndpointStatus{
			EndpointUUID:   epinfo.Spec.EndpointUUID,
			WorkloadUUID:   epinfo.Spec.WorkloadUUID,
			WorkloadName:   epinfo.Spec.WorkloadName,
			Network:        epinfo.Spec.NetworkName,
			HomingHostAddr: epinfo.Status.HomingHostAddr,
			HomingHostName: epinfo.Status.HomingHostName,
			NodeUUID:       epinfo.Status.NodeUUID,
		},
	}
	// create the endpoint
	eps, err := nw.CreateEndpoint(&epp)
	if err != nil {
		log.Errorf("Error creating endpoint {%+v}. Err: %v", epinfo, err)
		return nil, err
	}

	newEp := netproto.Endpoint{
		TypeMeta:   eps.TypeMeta,
		ObjectMeta: eps.ObjectMeta,
		Spec: netproto.EndpointSpec{
			EndpointUUID:   eps.Status.EndpointUUID,
			WorkloadUUID:   eps.Status.WorkloadUUID,
			WorkloadName:   eps.Status.WorkloadName,
			NetworkName:    eps.Status.Network,
			SecurityGroups: eps.Status.SecurityGroups,
		},
		Status: netproto.EndpointStatus{
			IPv4Address:    eps.Status.IPv4Address,
			IPv4Gateway:    eps.Status.IPv4Gateway,
			IPv6Address:    eps.Status.IPv6Address,
			IPv6Gateway:    eps.Status.IPv6Gateway,
			MacAddress:     eps.Status.MacAddress,
			HomingHostAddr: eps.Status.HomingHostAddr,
			HomingHostName: eps.Status.HomingHostName,
			UsegVlan:       eps.Status.MicroSegmentVlan,
			NodeUUID:       eps.Status.NodeUUID,
		},
	}

	return &newEp, nil
}

// GetEndpoint returns a specific endpoint
func (ep *EndpointRPCHandler) GetEndpoint(ctx context.Context, objmeta *api.ObjectMeta) (*netproto.Endpoint, error) {
	eps, err := ep.stateMgr.FindEndpoint(objmeta.Tenant, objmeta.Name)
	if err != nil {
		log.Errorf("Endpoint %+v not found. Err: %v", objmeta, err)
		return nil, err
	}

	// build endpoint
	endpoint := netproto.Endpoint{
		TypeMeta:   eps.Endpoint.TypeMeta,
		ObjectMeta: eps.Endpoint.ObjectMeta,
		Spec: netproto.EndpointSpec{
			EndpointUUID:   eps.Status.EndpointUUID,
			WorkloadUUID:   eps.Status.WorkloadUUID,
			WorkloadName:   eps.Status.WorkloadName,
			WorkloadLabels: eps.Status.WorkloadAttributes,
			NetworkName:    eps.Status.Network,
			SecurityGroups: eps.Status.SecurityGroups,
		},
		Status: netproto.EndpointStatus{
			IPv4Address: eps.Status.IPv4Address,
			IPv4Gateway: eps.Status.IPv4Gateway,
			IPv6Address: eps.Status.IPv6Address,
			IPv6Gateway: eps.Status.IPv6Gateway,
			MacAddress:  eps.Status.MacAddress,
			UsegVlan:    eps.Status.MicroSegmentVlan,
			NodeUUID:    eps.Status.NodeUUID,
		},
	}

	return &endpoint, nil
}

// ListEndpoints lists all endpoints matching object selector
func (ep *EndpointRPCHandler) ListEndpoints(ctx context.Context, objsel *api.ObjectMeta) (*netproto.EndpointList, error) {
	var eplist netproto.EndpointList

	// get all networks
	networks, err := ep.stateMgr.ListNetworks()
	if err != nil {
		return nil, err
	}

	// walk all networks

	for _, nt := range networks {
		// walk all endpoints
		epl := nt.ListEndpoints()
		for _, eps := range epl {

			// convert to netproto format
			endpoint := netproto.Endpoint{
				TypeMeta:   eps.Endpoint.TypeMeta,
				ObjectMeta: eps.Endpoint.ObjectMeta,
				Spec: netproto.EndpointSpec{
					EndpointUUID:   eps.Status.EndpointUUID,
					WorkloadUUID:   eps.Status.WorkloadUUID,
					WorkloadName:   eps.Status.WorkloadName,
					WorkloadLabels: eps.Status.WorkloadAttributes,
					NetworkName:    eps.Status.Network,
					SecurityGroups: eps.Status.SecurityGroups,
				},
				Status: netproto.EndpointStatus{
					IPv4Address: eps.Status.IPv4Address,
					IPv4Gateway: eps.Status.IPv4Gateway,
					IPv6Address: eps.Status.IPv6Address,
					IPv6Gateway: eps.Status.IPv6Gateway,
					MacAddress:  eps.Status.MacAddress,
					UsegVlan:    eps.Status.MicroSegmentVlan,
					NodeUUID:    eps.Status.NodeUUID,
				},
			}

			eplist.Endpoints = append(eplist.Endpoints, &endpoint)
		}
	}

	return &eplist, nil
}

// WatchEndpoints watches endpoints and sends streaming resp
func (ep *EndpointRPCHandler) WatchEndpoints(ometa *api.ObjectMeta, stream netproto.EndpointApi_WatchEndpointsServer) error {
	// watch for changes
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	ep.stateMgr.WatchObjects("Endpoint", watchChan)

	// get a list of all endpoints
	endpoints, err := ep.ListEndpoints(context.Background(), ometa)
	if err != nil {
		log.Errorf("Error getting a list of endpoints. Err: %v", err)
		return err
	}

	// walk all endpoints and send it out
	for _, endpoint := range endpoints.Endpoints {
		watchEvt := netproto.EndpointEvent{
			EventType: api.EventType_CreateEvent,
			Endpoint:  *endpoint,
		}
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending endpoint to stream. Err: %v", err)
			return err
		}
	}

	// loop forever on watch channel
	for {
		select {
		// read from channel
		case evt, ok := <-watchChan:
			if !ok {
				log.Errorf("Error reading from channel. Closing watch")
				close(watchChan)
				return errors.New("Error reading from channel")
			}

			// convert the events
			var etype api.EventType
			switch evt.EventType {
			case memdb.CreateEvent:
				etype = api.EventType_CreateEvent
			case memdb.UpdateEvent:
				etype = api.EventType_UpdateEvent
			case memdb.DeleteEvent:
				etype = api.EventType_DeleteEvent
			}

			// get the object
			endpoint, err := statemgr.EndpointStateFromObj(evt.Obj)
			if err != nil {
				return err
			}

			// convert to netproto format
			watchEvt := netproto.EndpointEvent{
				EventType: etype,
				Endpoint: netproto.Endpoint{
					TypeMeta:   endpoint.TypeMeta,
					ObjectMeta: endpoint.ObjectMeta,
					Spec: netproto.EndpointSpec{
						EndpointUUID:   endpoint.Status.EndpointUUID,
						WorkloadUUID:   endpoint.Status.WorkloadUUID,
						WorkloadName:   endpoint.Status.WorkloadName,
						WorkloadLabels: endpoint.Status.WorkloadAttributes,
						NetworkName:    endpoint.Status.Network,
						SecurityGroups: endpoint.Status.SecurityGroups,
					},
					Status: netproto.EndpointStatus{
						IPv4Address:    endpoint.Status.IPv4Address,
						IPv4Gateway:    endpoint.Status.IPv4Gateway,
						IPv6Address:    endpoint.Status.IPv6Address,
						IPv6Gateway:    endpoint.Status.IPv6Gateway,
						MacAddress:     endpoint.Status.MacAddress,
						HomingHostAddr: endpoint.Status.HomingHostAddr,
						HomingHostName: endpoint.Status.HomingHostName,
						UsegVlan:       endpoint.Status.MicroSegmentVlan,
						NodeUUID:       endpoint.Status.NodeUUID,
					},
				},
			}

			// streaming send
			err = stream.Send(&watchEvt)
			if err != nil {
				log.Errorf("Error sending endpoint to stream. Err: %v", err)
				return err
			}
		}
	}

	// done
}

// DeleteEndpoint deletes an endpoint
func (ep *EndpointRPCHandler) DeleteEndpoint(ctx context.Context, epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	log.Infof("Received DeleteEndpoint: {%+v}", epinfo)

	// find the network
	nw, err := ep.stateMgr.FindNetwork(epinfo.ObjectMeta.Tenant, epinfo.Spec.NetworkName)
	if err != nil {
		log.Errorf("Could not find network %s|%s", epinfo.ObjectMeta.Tenant, epinfo.Spec.NetworkName)
		return nil, fmt.Errorf("Could not find the network")
	}

	// delete the endpoint
	eps, err := nw.DeleteEndpoint(&epinfo.ObjectMeta)
	if err != nil {
		log.Errorf("Error deleting endpoint {%+v}. Err: %v", epinfo, err)
		return nil, err
	}

	// build endpoint message
	delEp := netproto.Endpoint{
		TypeMeta:   eps.TypeMeta,
		ObjectMeta: eps.ObjectMeta,
		Spec: netproto.EndpointSpec{
			EndpointUUID:   eps.Status.EndpointUUID,
			WorkloadUUID:   eps.Status.WorkloadUUID,
			WorkloadName:   eps.Status.WorkloadName,
			NetworkName:    eps.Status.Network,
			SecurityGroups: eps.Status.SecurityGroups,
		},
		Status: netproto.EndpointStatus{
			IPv4Address:    eps.Status.IPv4Address,
			IPv4Gateway:    eps.Status.IPv4Gateway,
			IPv6Address:    eps.Status.IPv6Address,
			IPv6Gateway:    eps.Status.IPv6Gateway,
			MacAddress:     eps.Status.MacAddress,
			HomingHostAddr: eps.Status.HomingHostAddr,
			HomingHostName: eps.Status.HomingHostName,
			UsegVlan:       eps.Status.MicroSegmentVlan,
			NodeUUID:       eps.Status.NodeUUID,
		},
	}

	return &delEp, nil
}

// NewEndpointRPCServer returns a network RPC server
func NewEndpointRPCServer(stateMgr *statemgr.Statemgr) (*EndpointRPCHandler, error) {
	return &EndpointRPCHandler{stateMgr: stateMgr}, nil
}
