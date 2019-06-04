// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package nimbus is a auto generated package.
Input file: endpoint.proto
*/

package nimbus

import (
	"context"
	"errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// FindEndpoint finds an Endpoint by object meta
func (ms *MbusServer) FindEndpoint(objmeta *api.ObjectMeta) (*netproto.Endpoint, error) {
	// find the object
	obj, err := ms.memDB.FindObject("Endpoint", objmeta)
	if err != nil {
		return nil, err
	}

	return EndpointFromObj(obj)
}

// ListEndpoints lists all Endpoints in the mbus
func (ms *MbusServer) ListEndpoints(ctx context.Context) ([]*netproto.Endpoint, error) {
	var objlist []*netproto.Endpoint

	// walk all objects
	objs := ms.memDB.ListObjects("Endpoint")
	for _, oo := range objs {
		obj, err := EndpointFromObj(oo)
		if err == nil {
			objlist = append(objlist, obj)
		}
	}

	return objlist, nil
}

// EndpointStatusReactor is the reactor interface implemented by controllers
type EndpointStatusReactor interface {
	OnEndpointAgentStatusSet(nodeID string, objinfo *netproto.Endpoint) error
	OnEndpointAgentStatusDelete(nodeID string, objinfo *netproto.Endpoint) error
}

// EndpointTopic is the Endpoint topic on message bus
type EndpointTopic struct {
	grpcServer    *rpckit.RPCServer // gRPC server instance
	server        *MbusServer
	statusReactor EndpointStatusReactor // status event reactor
}

// AddEndpointTopic returns a network RPC server
func AddEndpointTopic(server *MbusServer, reactor EndpointStatusReactor) (*EndpointTopic, error) {
	// RPC handler instance
	handler := EndpointTopic{
		grpcServer:    server.grpcServer,
		server:        server,
		statusReactor: reactor,
	}

	// register the RPC handlers
	if server.grpcServer != nil {
		netproto.RegisterEndpointApiServer(server.grpcServer.GrpcServer, &handler)
	}

	return &handler, nil
}

// CreateEndpoint creates Endpoint
func (eh *EndpointTopic) CreateEndpoint(ctx context.Context, objinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received CreateEndpoint from node %v: {%+v}", nodeID, objinfo)

	// trigger callbacks. we allow creates to happen before it exists in memdb
	if eh.statusReactor != nil {
		eh.statusReactor.OnEndpointAgentStatusSet(nodeID, objinfo)
	}

	// increment stats
	eh.server.Stats("Endpoint", "AgentCreate").Inc()

	// add object to node state
	err := eh.server.AddNodeState(nodeID, objinfo)
	if err != nil {
		log.Errorf("Error adding node state to memdb. Err: %v. node %v, Obj: {%+v}", err, nodeID, objinfo)
	}

	return objinfo, nil
}

// UpdateEndpoint updates Endpoint
func (eh *EndpointTopic) UpdateEndpoint(ctx context.Context, objinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received UpdateEndpoint from node %v: {%+v}", nodeID, objinfo)

	// add object to node state
	err := eh.server.AddNodeState(nodeID, objinfo)
	if err != nil {
		log.Errorf("Error adding node state to memdb. Err: %v. node %v, Obj: {%+v}", err, nodeID, objinfo)
		return nil, err
	}

	// incr stats
	eh.server.Stats("Endpoint", "AgentUpdate").Inc()

	// trigger callbacks
	if eh.statusReactor != nil {
		eh.statusReactor.OnEndpointAgentStatusSet(nodeID, objinfo)
	}

	return objinfo, nil
}

// DeleteEndpoint deletes an Endpoint
func (eh *EndpointTopic) DeleteEndpoint(ctx context.Context, objinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received DeleteEndpoint from node %v: {%+v}", nodeID, objinfo)

	// incr stats
	eh.server.Stats("Endpoint", "AgentDelete").Inc()

	// delete node state from the memdb
	err := eh.server.DelNodeState(nodeID, objinfo)
	if err != nil {
		log.Errorf("Error adding node state to memdb. Err: %v. node %v, Obj: {%+v}", err, nodeID, objinfo)
	}

	// trigger callbacks
	if eh.statusReactor != nil {
		eh.statusReactor.OnEndpointAgentStatusDelete(nodeID, objinfo)
	}

	return objinfo, nil
}

// EndpointFromObj converts memdb object to Endpoint
func EndpointFromObj(obj memdb.Object) (*netproto.Endpoint, error) {
	switch obj.(type) {
	case *netproto.Endpoint:
		eobj := obj.(*netproto.Endpoint)
		return eobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// GetEndpoint returns a specific Endpoint
func (eh *EndpointTopic) GetEndpoint(ctx context.Context, objmeta *api.ObjectMeta) (*netproto.Endpoint, error) {
	// find the object
	obj, err := eh.server.memDB.FindObject("Endpoint", objmeta)
	if err != nil {
		return nil, err
	}

	return EndpointFromObj(obj)
}

// ListEndpoints lists all Endpoints matching object selector
func (eh *EndpointTopic) ListEndpoints(ctx context.Context, objsel *api.ObjectMeta) (*netproto.EndpointList, error) {
	var objlist netproto.EndpointList

	// walk all objects
	objs := eh.server.memDB.ListObjects("Endpoint")
	for _, oo := range objs {
		obj, err := EndpointFromObj(oo)
		if err == nil {
			objlist.Endpoints = append(objlist.Endpoints, obj)
		}
	}

	return &objlist, nil
}

// WatchEndpoints watches Endpoints and sends streaming resp
func (eh *EndpointTopic) WatchEndpoints(ometa *api.ObjectMeta, stream netproto.EndpointApi_WatchEndpointsServer) error {
	// watch for changes
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)
	eh.server.memDB.WatchObjects("Endpoint", watchChan)
	defer eh.server.memDB.StopWatchObjects("Endpoint", watchChan)

	// get a list of all Endpoints
	objlist, err := eh.ListEndpoints(context.Background(), ometa)
	if err != nil {
		log.Errorf("Error getting a list of objects. Err: %v", err)
		return err
	}

	// increment stats
	eh.server.Stats("Endpoint", "ActiveWatch").Inc()
	eh.server.Stats("Endpoint", "WatchConnect").Inc()
	defer eh.server.Stats("Endpoint", "ActiveWatch").Dec()
	defer eh.server.Stats("Endpoint", "WatchDisconnect").Inc()

	ctx := stream.Context()

	// walk all Endpoints and send it out
	for _, obj := range objlist.Endpoints {
		watchEvt := netproto.EndpointEvent{
			EventType: api.EventType_CreateEvent,
			Endpoint:  *obj,
		}
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending Endpoint to stream. Err: %v", err)
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
			obj, err := EndpointFromObj(evt.Obj)
			if err != nil {
				return err
			}

			// convert to netproto format
			watchEvt := netproto.EndpointEvent{
				EventType: etype,
				Endpoint:  *obj,
			}

			// streaming send
			err = stream.Send(&watchEvt)
			if err != nil {
				log.Errorf("Error sending Endpoint to stream. Err: %v", err)
				return err
			}
		case <-ctx.Done():
			return ctx.Err()
		}
	}

	// done
}
