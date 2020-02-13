// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package nimbus is a auto generated package.
Input file: route.proto
*/

package nimbus

import (
	"context"
	"errors"
	"io"
	"strconv"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	hdr "github.com/pensando/sw/venice/utils/histogram"
	"github.com/pensando/sw/venice/utils/log"
	memdb "github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// FindRouteTable finds an RouteTable by object meta
func (ms *MbusServer) FindRouteTable(objmeta *api.ObjectMeta) (*netproto.RouteTable, error) {
	// find the object
	obj, err := ms.memDB.FindObject("RouteTable", objmeta)
	if err != nil {
		return nil, err
	}

	return RouteTableFromObj(obj)
}

// ListRouteTables lists all RouteTables in the mbus
func (ms *MbusServer) ListRouteTables(ctx context.Context, filters []memdb.FilterFn) ([]*netproto.RouteTable, error) {
	var objlist []*netproto.RouteTable

	// walk all objects
	objs := ms.memDB.ListObjects("RouteTable", filters)
	for _, oo := range objs {
		obj, err := RouteTableFromObj(oo)
		if err == nil {
			objlist = append(objlist, obj)
		}
	}

	return objlist, nil
}

// RouteTableStatusReactor is the reactor interface implemented by controllers
type RouteTableStatusReactor interface {
	OnRouteTableCreateReq(nodeID string, objinfo *netproto.RouteTable) error
	OnRouteTableUpdateReq(nodeID string, objinfo *netproto.RouteTable) error
	OnRouteTableDeleteReq(nodeID string, objinfo *netproto.RouteTable) error
	OnRouteTableOperUpdate(nodeID string, objinfo *netproto.RouteTable) error
	OnRouteTableOperDelete(nodeID string, objinfo *netproto.RouteTable) error
	GetWatchFilter(kind string, watchOptions *api.ListWatchOptions) []memdb.FilterFn
}

type RouteTableNodeStatus struct {
	nodeID        string
	watcher       *memdb.Watcher
	opSentStatus  map[api.EventType]*EventStatus
	opAckedStatus map[api.EventType]*EventStatus
}

// RouteTableTopic is the RouteTable topic on message bus
type RouteTableTopic struct {
	sync.Mutex
	grpcServer    *rpckit.RPCServer // gRPC server instance
	server        *MbusServer
	statusReactor RouteTableStatusReactor // status event reactor
	nodeStatus    map[string]*RouteTableNodeStatus
}

// AddRouteTableTopic returns a network RPC server
func AddRouteTableTopic(server *MbusServer, reactor RouteTableStatusReactor) (*RouteTableTopic, error) {
	// RPC handler instance
	handler := RouteTableTopic{
		grpcServer:    server.grpcServer,
		server:        server,
		statusReactor: reactor,
		nodeStatus:    make(map[string]*RouteTableNodeStatus),
	}

	// register the RPC handlers
	if server.grpcServer != nil {
		netproto.RegisterRouteTableApiV1Server(server.grpcServer.GrpcServer, &handler)
	}

	return &handler, nil
}

func (eh *RouteTableTopic) registerWatcher(nodeID string, watcher *memdb.Watcher) {
	eh.Lock()
	defer eh.Unlock()

	eh.nodeStatus[nodeID] = &RouteTableNodeStatus{nodeID: nodeID, watcher: watcher}
	eh.nodeStatus[nodeID].opSentStatus = make(map[api.EventType]*EventStatus)
	eh.nodeStatus[nodeID].opAckedStatus = make(map[api.EventType]*EventStatus)
}

func (eh *RouteTableTopic) unRegisterWatcher(nodeID string) {
	eh.Lock()
	defer eh.Unlock()

	delete(eh.nodeStatus, nodeID)
}

//update recv object status
func (eh *RouteTableTopic) updateAckedObjStatus(nodeID string, event api.EventType, objMeta *api.ObjectMeta) {

	eh.Lock()
	defer eh.Unlock()
	var evStatus *EventStatus

	nodeStatus, ok := eh.nodeStatus[nodeID]
	if !ok {
		//Watcher already unregistered.
		return
	}

	evStatus, ok = nodeStatus.opAckedStatus[event]
	if !ok {
		nodeStatus.opAckedStatus[event] = &EventStatus{}
		evStatus = nodeStatus.opAckedStatus[event]
	}

	if LatencyMeasurementEnabled {
		rcvdTime, _ := objMeta.ModTime.Time()
		sendTime, _ := objMeta.CreationTime.Time()
		delta := rcvdTime.Sub(sendTime)

		hdr.Record(nodeID+"_"+"RouteTable", delta)
		hdr.Record("RouteTable", delta)
		hdr.Record(nodeID, delta)
	}

	new, _ := strconv.Atoi(objMeta.ResourceVersion)
	//for create/delete keep track of last one sent to, this may not be full proof
	//  Create could be processed asynchoronusly by client and can come out of order.
	//  For now should be ok as at least we make sure all messages are processed.
	//For update keep track of only last one as nimbus client periodically pulls
	if evStatus.LastObjectMeta != nil {
		current, _ := strconv.Atoi(evStatus.LastObjectMeta.ResourceVersion)
		if current > new {
			return
		}
	}
	evStatus.LastObjectMeta = objMeta
}

//update recv object status
func (eh *RouteTableTopic) updateSentObjStatus(nodeID string, event api.EventType, objMeta *api.ObjectMeta) {

	eh.Lock()
	defer eh.Unlock()
	var evStatus *EventStatus

	nodeStatus, ok := eh.nodeStatus[nodeID]
	if !ok {
		//Watcher already unregistered.
		return
	}

	evStatus, ok = nodeStatus.opSentStatus[event]
	if !ok {
		nodeStatus.opSentStatus[event] = &EventStatus{}
		evStatus = nodeStatus.opSentStatus[event]
	}

	new, _ := strconv.Atoi(objMeta.ResourceVersion)
	//for create/delete keep track of last one sent to, this may not be full proof
	//  Create could be processed asynchoronusly by client and can come out of order.
	//  For now should be ok as at least we make sure all messages are processed.
	//For update keep track of only last one as nimbus client periodically pulls
	if evStatus.LastObjectMeta != nil {
		current, _ := strconv.Atoi(evStatus.LastObjectMeta.ResourceVersion)
		if current > new {
			return
		}
	}
	evStatus.LastObjectMeta = objMeta
}

//update recv object status
func (eh *RouteTableTopic) WatcherInConfigSync(nodeID string, event api.EventType) bool {

	var ok bool
	var evStatus *EventStatus
	var evAckStatus *EventStatus

	eh.Lock()
	defer eh.Unlock()

	nodeStatus, ok := eh.nodeStatus[nodeID]
	if !ok {
		return true
	}

	evStatus, ok = nodeStatus.opSentStatus[event]
	if !ok {
		//nothing sent, so insync
		return true
	}

	//In-flight object still exists
	if len(nodeStatus.watcher.Channel) != 0 {
		log.Infof("watcher %v still has objects in in-flight %v(%v)", nodeID, "RouteTable", event)
		return false
	}

	evAckStatus, ok = nodeStatus.opAckedStatus[event]
	if !ok {
		//nothing received, failed.
		log.Infof("watcher %v still has not received anything %v(%v)", nodeID, "RouteTable", event)
		return false
	}

	if evAckStatus.LastObjectMeta.ResourceVersion < evStatus.LastObjectMeta.ResourceVersion {
		log.Infof("watcher %v resource version mismatch for %v(%v)  sent %v: recived %v",
			nodeID, "RouteTable", event, evStatus.LastObjectMeta.ResourceVersion,
			evAckStatus.LastObjectMeta.ResourceVersion)
		return false
	}

	return true
}

/*
//GetSentEventStatus
func (eh *RouteTableTopic) GetSentEventStatus(nodeID string, event api.EventType) *EventStatus {

    eh.Lock()
    defer eh.Unlock()
    var evStatus *EventStatus

    objStatus, ok := eh.opSentStatus[nodeID]
    if ok {
        evStatus, ok = objStatus.opStatus[event]
        if ok {
            return evStatus
        }
    }
    return nil
}


//GetAckedEventStatus
func (eh *RouteTableTopic) GetAckedEventStatus(nodeID string, event api.EventType) *EventStatus {

    eh.Lock()
    defer eh.Unlock()
    var evStatus *EventStatus

    objStatus, ok := eh.opAckedStatus[nodeID]
    if ok {
        evStatus, ok = objStatus.opStatus[event]
        if ok {
            return evStatus
        }
    }
    return nil
}

*/

// CreateRouteTable creates RouteTable
func (eh *RouteTableTopic) CreateRouteTable(ctx context.Context, objinfo *netproto.RouteTable) (*netproto.RouteTable, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received CreateRouteTable from node %v: {%+v}", nodeID, objinfo)

	// trigger callbacks. we allow creates to happen before it exists in memdb
	if eh.statusReactor != nil {
		eh.statusReactor.OnRouteTableCreateReq(nodeID, objinfo)
	}

	// increment stats
	eh.server.Stats("RouteTable", "AgentCreate").Inc()

	return objinfo, nil
}

// UpdateRouteTable updates RouteTable
func (eh *RouteTableTopic) UpdateRouteTable(ctx context.Context, objinfo *netproto.RouteTable) (*netproto.RouteTable, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received UpdateRouteTable from node %v: {%+v}", nodeID, objinfo)

	// incr stats
	eh.server.Stats("RouteTable", "AgentUpdate").Inc()

	// trigger callbacks
	if eh.statusReactor != nil {
		eh.statusReactor.OnRouteTableUpdateReq(nodeID, objinfo)
	}

	return objinfo, nil
}

// DeleteRouteTable deletes an RouteTable
func (eh *RouteTableTopic) DeleteRouteTable(ctx context.Context, objinfo *netproto.RouteTable) (*netproto.RouteTable, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received DeleteRouteTable from node %v: {%+v}", nodeID, objinfo)

	// incr stats
	eh.server.Stats("RouteTable", "AgentDelete").Inc()

	// trigger callbacks
	if eh.statusReactor != nil {
		eh.statusReactor.OnRouteTableDeleteReq(nodeID, objinfo)
	}

	return objinfo, nil
}

// RouteTableFromObj converts memdb object to RouteTable
func RouteTableFromObj(obj memdb.Object) (*netproto.RouteTable, error) {
	switch obj.(type) {
	case *netproto.RouteTable:
		eobj := obj.(*netproto.RouteTable)
		return eobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// GetRouteTable returns a specific RouteTable
func (eh *RouteTableTopic) GetRouteTable(ctx context.Context, objmeta *api.ObjectMeta) (*netproto.RouteTable, error) {
	// find the object
	obj, err := eh.server.memDB.FindObject("RouteTable", objmeta)
	if err != nil {
		return nil, err
	}

	return RouteTableFromObj(obj)
}

// ListRouteTables lists all RouteTables matching object selector
func (eh *RouteTableTopic) ListRouteTables(ctx context.Context, objsel *api.ListWatchOptions) (*netproto.RouteTableList, error) {
	var objlist netproto.RouteTableList
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	filters := []memdb.FilterFn{}

	filterFn := func(obj, prev memdb.Object) bool {
		return true
	}

	if eh.statusReactor != nil {
		filters = eh.statusReactor.GetWatchFilter("RouteTable", objsel)
	} else {
		filters = append(filters, filterFn)
	}

	// walk all objects
	objs := eh.server.memDB.ListObjects("RouteTable", filters)
	//creationTime, _ := types.TimestampProto(time.Now())
	for _, oo := range objs {
		obj, err := RouteTableFromObj(oo)
		if err == nil {
			//obj.CreationTime = api.Timestamp{Timestamp: *creationTime}
			objlist.RouteTables = append(objlist.RouteTables, obj)
			//record the last object sent to check config sync
			eh.updateSentObjStatus(nodeID, api.EventType_UpdateEvent, &obj.ObjectMeta)
		}
	}

	return &objlist, nil
}

// WatchRouteTables watches RouteTables and sends streaming resp
func (eh *RouteTableTopic) WatchRouteTables(watchOptions *api.ListWatchOptions, stream netproto.RouteTableApiV1_WatchRouteTablesServer) error {
	// watch for changes
	watcher := memdb.Watcher{}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	watcher.Filters = make(map[string][]memdb.FilterFn)
	defer close(watcher.Channel)

	if eh.statusReactor != nil {
		watcher.Filters["RouteTable"] = eh.statusReactor.GetWatchFilter("RouteTable", watchOptions)
	} else {
		filt := func(obj, prev memdb.Object) bool {
			return true
		}
		watcher.Filters["RouteTable"] = append(watcher.Filters["RouteTable"], filt)
	}

	ctx := stream.Context()
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	watcher.Name = nodeID
	eh.server.memDB.WatchObjects("RouteTable", &watcher)
	defer eh.server.memDB.StopWatchObjects("RouteTable", &watcher)

	// get a list of all RouteTables
	objlist, err := eh.ListRouteTables(context.Background(), watchOptions)
	if err != nil {
		log.Errorf("Error getting a list of objects. Err: %v", err)
		return err
	}

	eh.registerWatcher(nodeID, &watcher)
	defer eh.unRegisterWatcher(nodeID)

	// increment stats
	eh.server.Stats("RouteTable", "ActiveWatch").Inc()
	eh.server.Stats("RouteTable", "WatchConnect").Inc()
	defer eh.server.Stats("RouteTable", "ActiveWatch").Dec()
	defer eh.server.Stats("RouteTable", "WatchDisconnect").Inc()

	// walk all RouteTables and send it out
	watchEvts := netproto.RouteTableEventList{}
	for _, obj := range objlist.RouteTables {
		watchEvt := netproto.RouteTableEvent{
			EventType:  api.EventType_CreateEvent,
			RouteTable: *obj,
		}
		watchEvts.RouteTableEvents = append(watchEvts.RouteTableEvents, &watchEvt)
	}
	if len(watchEvts.RouteTableEvents) > 0 {
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending RouteTable to stream. Err: %v", err)
			return err
		}
	}
	timer := time.NewTimer(DefaultWatchHoldInterval)
	if !timer.Stop() {
		<-timer.C
	}

	running := false
	watchEvts = netproto.RouteTableEventList{}
	sendToStream := func() error {
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending RouteTable to stream. Err: %v", err)
			return err
		}
		watchEvts = netproto.RouteTableEventList{}
		return nil
	}

	// loop forever on watch channel
	for {
		select {
		// read from channel
		case evt, ok := <-watcher.Channel:
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
			obj, err := RouteTableFromObj(evt.Obj)
			if err != nil {
				return err
			}

			// convert to netproto format
			watchEvt := netproto.RouteTableEvent{
				EventType:  etype,
				RouteTable: *obj,
			}
			watchEvts.RouteTableEvents = append(watchEvts.RouteTableEvents, &watchEvt)
			if !running {
				running = true
				timer.Reset(DefaultWatchHoldInterval)
			}
			if len(watchEvts.RouteTableEvents) >= DefaultWatchBatchSize {
				if err = sendToStream(); err != nil {
					return err
				}
				if !timer.Stop() {
					<-timer.C
				}
				timer.Reset(DefaultWatchHoldInterval)
			}
			eh.updateSentObjStatus(nodeID, etype, &obj.ObjectMeta)
		case <-timer.C:
			running = false
			if err = sendToStream(); err != nil {
				return err
			}
		case <-ctx.Done():
			return ctx.Err()
		}
	}

	// done
}

// updateRouteTableOper triggers oper update callbacks
func (eh *RouteTableTopic) updateRouteTableOper(oper *netproto.RouteTableEvent, nodeID string) error {
	eh.updateAckedObjStatus(nodeID, oper.EventType, &oper.RouteTable.ObjectMeta)
	switch oper.EventType {
	case api.EventType_CreateEvent:
		fallthrough
	case api.EventType_UpdateEvent:
		// incr stats
		eh.server.Stats("RouteTable", "AgentUpdate").Inc()

		// trigger callbacks
		if eh.statusReactor != nil {
			return eh.statusReactor.OnRouteTableOperUpdate(nodeID, &oper.RouteTable)
		}
	case api.EventType_DeleteEvent:
		// incr stats
		eh.server.Stats("RouteTable", "AgentDelete").Inc()

		// trigger callbacks
		if eh.statusReactor != nil {
			eh.statusReactor.OnRouteTableOperDelete(nodeID, &oper.RouteTable)
		}
	}

	return nil
}

func (eh *RouteTableTopic) RouteTableOperUpdate(stream netproto.RouteTableApiV1_RouteTableOperUpdateServer) error {
	ctx := stream.Context()
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)

	for {
		oper, err := stream.Recv()
		if err == io.EOF {
			log.Errorf("%v RouteTableOperUpdate stream ended. closing..", nodeID)
			return stream.SendAndClose(&api.TypeMeta{})
		} else if err != nil {
			log.Errorf("Error receiving from %v RouteTableOperUpdate stream. Err: %v", nodeID, err)
			return err
		}

		err = eh.updateRouteTableOper(oper, nodeID)
		if err != nil {
			log.Errorf("Error updating RouteTable oper state. Err: %v", err)
		}
	}
}

// FindRoutingConfig finds an RoutingConfig by object meta
func (ms *MbusServer) FindRoutingConfig(objmeta *api.ObjectMeta) (*netproto.RoutingConfig, error) {
	// find the object
	obj, err := ms.memDB.FindObject("RoutingConfig", objmeta)
	if err != nil {
		return nil, err
	}

	return RoutingConfigFromObj(obj)
}

// ListRoutingConfigs lists all RoutingConfigs in the mbus
func (ms *MbusServer) ListRoutingConfigs(ctx context.Context, filters []memdb.FilterFn) ([]*netproto.RoutingConfig, error) {
	var objlist []*netproto.RoutingConfig

	// walk all objects
	objs := ms.memDB.ListObjects("RoutingConfig", filters)
	for _, oo := range objs {
		obj, err := RoutingConfigFromObj(oo)
		if err == nil {
			objlist = append(objlist, obj)
		}
	}

	return objlist, nil
}

// RoutingConfigStatusReactor is the reactor interface implemented by controllers
type RoutingConfigStatusReactor interface {
	OnRoutingConfigCreateReq(nodeID string, objinfo *netproto.RoutingConfig) error
	OnRoutingConfigUpdateReq(nodeID string, objinfo *netproto.RoutingConfig) error
	OnRoutingConfigDeleteReq(nodeID string, objinfo *netproto.RoutingConfig) error
	OnRoutingConfigOperUpdate(nodeID string, objinfo *netproto.RoutingConfig) error
	OnRoutingConfigOperDelete(nodeID string, objinfo *netproto.RoutingConfig) error
	GetWatchFilter(kind string, watchOptions *api.ListWatchOptions) []memdb.FilterFn
}

type RoutingConfigNodeStatus struct {
	nodeID        string
	watcher       *memdb.Watcher
	opSentStatus  map[api.EventType]*EventStatus
	opAckedStatus map[api.EventType]*EventStatus
}

// RoutingConfigTopic is the RoutingConfig topic on message bus
type RoutingConfigTopic struct {
	sync.Mutex
	grpcServer    *rpckit.RPCServer // gRPC server instance
	server        *MbusServer
	statusReactor RoutingConfigStatusReactor // status event reactor
	nodeStatus    map[string]*RoutingConfigNodeStatus
}

// AddRoutingConfigTopic returns a network RPC server
func AddRoutingConfigTopic(server *MbusServer, reactor RoutingConfigStatusReactor) (*RoutingConfigTopic, error) {
	// RPC handler instance
	handler := RoutingConfigTopic{
		grpcServer:    server.grpcServer,
		server:        server,
		statusReactor: reactor,
		nodeStatus:    make(map[string]*RoutingConfigNodeStatus),
	}

	// register the RPC handlers
	if server.grpcServer != nil {
		netproto.RegisterRoutingConfigApiV1Server(server.grpcServer.GrpcServer, &handler)
	}

	return &handler, nil
}

func (eh *RoutingConfigTopic) registerWatcher(nodeID string, watcher *memdb.Watcher) {
	eh.Lock()
	defer eh.Unlock()

	eh.nodeStatus[nodeID] = &RoutingConfigNodeStatus{nodeID: nodeID, watcher: watcher}
	eh.nodeStatus[nodeID].opSentStatus = make(map[api.EventType]*EventStatus)
	eh.nodeStatus[nodeID].opAckedStatus = make(map[api.EventType]*EventStatus)
}

func (eh *RoutingConfigTopic) unRegisterWatcher(nodeID string) {
	eh.Lock()
	defer eh.Unlock()

	delete(eh.nodeStatus, nodeID)
}

//update recv object status
func (eh *RoutingConfigTopic) updateAckedObjStatus(nodeID string, event api.EventType, objMeta *api.ObjectMeta) {

	eh.Lock()
	defer eh.Unlock()
	var evStatus *EventStatus

	nodeStatus, ok := eh.nodeStatus[nodeID]
	if !ok {
		//Watcher already unregistered.
		return
	}

	evStatus, ok = nodeStatus.opAckedStatus[event]
	if !ok {
		nodeStatus.opAckedStatus[event] = &EventStatus{}
		evStatus = nodeStatus.opAckedStatus[event]
	}

	if LatencyMeasurementEnabled {
		rcvdTime, _ := objMeta.ModTime.Time()
		sendTime, _ := objMeta.CreationTime.Time()
		delta := rcvdTime.Sub(sendTime)

		hdr.Record(nodeID+"_"+"RoutingConfig", delta)
		hdr.Record("RoutingConfig", delta)
		hdr.Record(nodeID, delta)
	}

	new, _ := strconv.Atoi(objMeta.ResourceVersion)
	//for create/delete keep track of last one sent to, this may not be full proof
	//  Create could be processed asynchoronusly by client and can come out of order.
	//  For now should be ok as at least we make sure all messages are processed.
	//For update keep track of only last one as nimbus client periodically pulls
	if evStatus.LastObjectMeta != nil {
		current, _ := strconv.Atoi(evStatus.LastObjectMeta.ResourceVersion)
		if current > new {
			return
		}
	}
	evStatus.LastObjectMeta = objMeta
}

//update recv object status
func (eh *RoutingConfigTopic) updateSentObjStatus(nodeID string, event api.EventType, objMeta *api.ObjectMeta) {

	eh.Lock()
	defer eh.Unlock()
	var evStatus *EventStatus

	nodeStatus, ok := eh.nodeStatus[nodeID]
	if !ok {
		//Watcher already unregistered.
		return
	}

	evStatus, ok = nodeStatus.opSentStatus[event]
	if !ok {
		nodeStatus.opSentStatus[event] = &EventStatus{}
		evStatus = nodeStatus.opSentStatus[event]
	}

	new, _ := strconv.Atoi(objMeta.ResourceVersion)
	//for create/delete keep track of last one sent to, this may not be full proof
	//  Create could be processed asynchoronusly by client and can come out of order.
	//  For now should be ok as at least we make sure all messages are processed.
	//For update keep track of only last one as nimbus client periodically pulls
	if evStatus.LastObjectMeta != nil {
		current, _ := strconv.Atoi(evStatus.LastObjectMeta.ResourceVersion)
		if current > new {
			return
		}
	}
	evStatus.LastObjectMeta = objMeta
}

//update recv object status
func (eh *RoutingConfigTopic) WatcherInConfigSync(nodeID string, event api.EventType) bool {

	var ok bool
	var evStatus *EventStatus
	var evAckStatus *EventStatus

	eh.Lock()
	defer eh.Unlock()

	nodeStatus, ok := eh.nodeStatus[nodeID]
	if !ok {
		return true
	}

	evStatus, ok = nodeStatus.opSentStatus[event]
	if !ok {
		//nothing sent, so insync
		return true
	}

	//In-flight object still exists
	if len(nodeStatus.watcher.Channel) != 0 {
		log.Infof("watcher %v still has objects in in-flight %v(%v)", nodeID, "RoutingConfig", event)
		return false
	}

	evAckStatus, ok = nodeStatus.opAckedStatus[event]
	if !ok {
		//nothing received, failed.
		log.Infof("watcher %v still has not received anything %v(%v)", nodeID, "RoutingConfig", event)
		return false
	}

	if evAckStatus.LastObjectMeta.ResourceVersion < evStatus.LastObjectMeta.ResourceVersion {
		log.Infof("watcher %v resource version mismatch for %v(%v)  sent %v: recived %v",
			nodeID, "RoutingConfig", event, evStatus.LastObjectMeta.ResourceVersion,
			evAckStatus.LastObjectMeta.ResourceVersion)
		return false
	}

	return true
}

/*
//GetSentEventStatus
func (eh *RoutingConfigTopic) GetSentEventStatus(nodeID string, event api.EventType) *EventStatus {

    eh.Lock()
    defer eh.Unlock()
    var evStatus *EventStatus

    objStatus, ok := eh.opSentStatus[nodeID]
    if ok {
        evStatus, ok = objStatus.opStatus[event]
        if ok {
            return evStatus
        }
    }
    return nil
}


//GetAckedEventStatus
func (eh *RoutingConfigTopic) GetAckedEventStatus(nodeID string, event api.EventType) *EventStatus {

    eh.Lock()
    defer eh.Unlock()
    var evStatus *EventStatus

    objStatus, ok := eh.opAckedStatus[nodeID]
    if ok {
        evStatus, ok = objStatus.opStatus[event]
        if ok {
            return evStatus
        }
    }
    return nil
}

*/

// CreateRoutingConfig creates RoutingConfig
func (eh *RoutingConfigTopic) CreateRoutingConfig(ctx context.Context, objinfo *netproto.RoutingConfig) (*netproto.RoutingConfig, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received CreateRoutingConfig from node %v: {%+v}", nodeID, objinfo)

	// trigger callbacks. we allow creates to happen before it exists in memdb
	if eh.statusReactor != nil {
		eh.statusReactor.OnRoutingConfigCreateReq(nodeID, objinfo)
	}

	// increment stats
	eh.server.Stats("RoutingConfig", "AgentCreate").Inc()

	return objinfo, nil
}

// UpdateRoutingConfig updates RoutingConfig
func (eh *RoutingConfigTopic) UpdateRoutingConfig(ctx context.Context, objinfo *netproto.RoutingConfig) (*netproto.RoutingConfig, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received UpdateRoutingConfig from node %v: {%+v}", nodeID, objinfo)

	// incr stats
	eh.server.Stats("RoutingConfig", "AgentUpdate").Inc()

	// trigger callbacks
	if eh.statusReactor != nil {
		eh.statusReactor.OnRoutingConfigUpdateReq(nodeID, objinfo)
	}

	return objinfo, nil
}

// DeleteRoutingConfig deletes an RoutingConfig
func (eh *RoutingConfigTopic) DeleteRoutingConfig(ctx context.Context, objinfo *netproto.RoutingConfig) (*netproto.RoutingConfig, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received DeleteRoutingConfig from node %v: {%+v}", nodeID, objinfo)

	// incr stats
	eh.server.Stats("RoutingConfig", "AgentDelete").Inc()

	// trigger callbacks
	if eh.statusReactor != nil {
		eh.statusReactor.OnRoutingConfigDeleteReq(nodeID, objinfo)
	}

	return objinfo, nil
}

// RoutingConfigFromObj converts memdb object to RoutingConfig
func RoutingConfigFromObj(obj memdb.Object) (*netproto.RoutingConfig, error) {
	switch obj.(type) {
	case *netproto.RoutingConfig:
		eobj := obj.(*netproto.RoutingConfig)
		return eobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// GetRoutingConfig returns a specific RoutingConfig
func (eh *RoutingConfigTopic) GetRoutingConfig(ctx context.Context, objmeta *api.ObjectMeta) (*netproto.RoutingConfig, error) {
	// find the object
	obj, err := eh.server.memDB.FindObject("RoutingConfig", objmeta)
	if err != nil {
		return nil, err
	}

	return RoutingConfigFromObj(obj)
}

// ListRoutingConfigs lists all RoutingConfigs matching object selector
func (eh *RoutingConfigTopic) ListRoutingConfigs(ctx context.Context, objsel *api.ListWatchOptions) (*netproto.RoutingConfigList, error) {
	var objlist netproto.RoutingConfigList
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	filters := []memdb.FilterFn{}

	filterFn := func(obj, prev memdb.Object) bool {
		return true
	}

	if eh.statusReactor != nil {
		filters = eh.statusReactor.GetWatchFilter("RoutingConfig", objsel)
	} else {
		filters = append(filters, filterFn)
	}

	// walk all objects
	objs := eh.server.memDB.ListObjects("RoutingConfig", filters)
	//creationTime, _ := types.TimestampProto(time.Now())
	for _, oo := range objs {
		obj, err := RoutingConfigFromObj(oo)
		if err == nil {
			//obj.CreationTime = api.Timestamp{Timestamp: *creationTime}
			objlist.RoutingConfigs = append(objlist.RoutingConfigs, obj)
			//record the last object sent to check config sync
			eh.updateSentObjStatus(nodeID, api.EventType_UpdateEvent, &obj.ObjectMeta)
		}
	}

	return &objlist, nil
}

// WatchRoutingConfigs watches RoutingConfigs and sends streaming resp
func (eh *RoutingConfigTopic) WatchRoutingConfigs(watchOptions *api.ListWatchOptions, stream netproto.RoutingConfigApiV1_WatchRoutingConfigsServer) error {
	// watch for changes
	watcher := memdb.Watcher{}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	watcher.Filters = make(map[string][]memdb.FilterFn)
	defer close(watcher.Channel)

	if eh.statusReactor != nil {
		watcher.Filters["RoutingConfig"] = eh.statusReactor.GetWatchFilter("RoutingConfig", watchOptions)
	} else {
		filt := func(obj, prev memdb.Object) bool {
			return true
		}
		watcher.Filters["RoutingConfig"] = append(watcher.Filters["RoutingConfig"], filt)
	}

	ctx := stream.Context()
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	watcher.Name = nodeID
	eh.server.memDB.WatchObjects("RoutingConfig", &watcher)
	defer eh.server.memDB.StopWatchObjects("RoutingConfig", &watcher)

	// get a list of all RoutingConfigs
	objlist, err := eh.ListRoutingConfigs(context.Background(), watchOptions)
	if err != nil {
		log.Errorf("Error getting a list of objects. Err: %v", err)
		return err
	}

	eh.registerWatcher(nodeID, &watcher)
	defer eh.unRegisterWatcher(nodeID)

	// increment stats
	eh.server.Stats("RoutingConfig", "ActiveWatch").Inc()
	eh.server.Stats("RoutingConfig", "WatchConnect").Inc()
	defer eh.server.Stats("RoutingConfig", "ActiveWatch").Dec()
	defer eh.server.Stats("RoutingConfig", "WatchDisconnect").Inc()

	// walk all RoutingConfigs and send it out
	watchEvts := netproto.RoutingConfigEventList{}
	for _, obj := range objlist.RoutingConfigs {
		watchEvt := netproto.RoutingConfigEvent{
			EventType:     api.EventType_CreateEvent,
			RoutingConfig: *obj,
		}
		watchEvts.RoutingConfigEvents = append(watchEvts.RoutingConfigEvents, &watchEvt)
	}
	if len(watchEvts.RoutingConfigEvents) > 0 {
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending RoutingConfig to stream. Err: %v", err)
			return err
		}
	}
	timer := time.NewTimer(DefaultWatchHoldInterval)
	if !timer.Stop() {
		<-timer.C
	}

	running := false
	watchEvts = netproto.RoutingConfigEventList{}
	sendToStream := func() error {
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending RoutingConfig to stream. Err: %v", err)
			return err
		}
		watchEvts = netproto.RoutingConfigEventList{}
		return nil
	}

	// loop forever on watch channel
	for {
		select {
		// read from channel
		case evt, ok := <-watcher.Channel:
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
			obj, err := RoutingConfigFromObj(evt.Obj)
			if err != nil {
				return err
			}

			// convert to netproto format
			watchEvt := netproto.RoutingConfigEvent{
				EventType:     etype,
				RoutingConfig: *obj,
			}
			watchEvts.RoutingConfigEvents = append(watchEvts.RoutingConfigEvents, &watchEvt)
			if !running {
				running = true
				timer.Reset(DefaultWatchHoldInterval)
			}
			if len(watchEvts.RoutingConfigEvents) >= DefaultWatchBatchSize {
				if err = sendToStream(); err != nil {
					return err
				}
				if !timer.Stop() {
					<-timer.C
				}
				timer.Reset(DefaultWatchHoldInterval)
			}
			eh.updateSentObjStatus(nodeID, etype, &obj.ObjectMeta)
		case <-timer.C:
			running = false
			if err = sendToStream(); err != nil {
				return err
			}
		case <-ctx.Done():
			return ctx.Err()
		}
	}

	// done
}

// updateRoutingConfigOper triggers oper update callbacks
func (eh *RoutingConfigTopic) updateRoutingConfigOper(oper *netproto.RoutingConfigEvent, nodeID string) error {
	eh.updateAckedObjStatus(nodeID, oper.EventType, &oper.RoutingConfig.ObjectMeta)
	switch oper.EventType {
	case api.EventType_CreateEvent:
		fallthrough
	case api.EventType_UpdateEvent:
		// incr stats
		eh.server.Stats("RoutingConfig", "AgentUpdate").Inc()

		// trigger callbacks
		if eh.statusReactor != nil {
			return eh.statusReactor.OnRoutingConfigOperUpdate(nodeID, &oper.RoutingConfig)
		}
	case api.EventType_DeleteEvent:
		// incr stats
		eh.server.Stats("RoutingConfig", "AgentDelete").Inc()

		// trigger callbacks
		if eh.statusReactor != nil {
			eh.statusReactor.OnRoutingConfigOperDelete(nodeID, &oper.RoutingConfig)
		}
	}

	return nil
}

func (eh *RoutingConfigTopic) RoutingConfigOperUpdate(stream netproto.RoutingConfigApiV1_RoutingConfigOperUpdateServer) error {
	ctx := stream.Context()
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)

	for {
		oper, err := stream.Recv()
		if err == io.EOF {
			log.Errorf("%v RoutingConfigOperUpdate stream ended. closing..", nodeID)
			return stream.SendAndClose(&api.TypeMeta{})
		} else if err != nil {
			log.Errorf("Error receiving from %v RoutingConfigOperUpdate stream. Err: %v", nodeID, err)
			return err
		}

		err = eh.updateRoutingConfigOper(oper, nodeID)
		if err != nil {
			log.Errorf("Error updating RoutingConfig oper state. Err: %v", err)
		}
	}
}
