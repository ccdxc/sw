// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package nimbus is a auto generated package.
Input file: security.proto
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

// FindSecurityGroup finds an SecurityGroup by object meta
func (ms *MbusServer) FindSecurityGroup(objmeta *api.ObjectMeta) (*netproto.SecurityGroup, error) {
	// find the object
	obj, err := ms.memDB.FindObject("SecurityGroup", objmeta)
	if err != nil {
		return nil, err
	}

	return SecurityGroupFromObj(obj)
}

// ListSecurityGroups lists all SecurityGroups in the mbus
func (ms *MbusServer) ListSecurityGroups(ctx context.Context, filterFn func(memdb.Object) bool) ([]*netproto.SecurityGroup, error) {
	var objlist []*netproto.SecurityGroup

	// walk all objects
	objs := ms.memDB.ListObjects("SecurityGroup", filterFn)
	for _, oo := range objs {
		obj, err := SecurityGroupFromObj(oo)
		if err == nil {
			objlist = append(objlist, obj)
		}
	}

	return objlist, nil
}

// SecurityGroupStatusReactor is the reactor interface implemented by controllers
type SecurityGroupStatusReactor interface {
	OnSecurityGroupCreateReq(nodeID string, objinfo *netproto.SecurityGroup) error
	OnSecurityGroupUpdateReq(nodeID string, objinfo *netproto.SecurityGroup) error
	OnSecurityGroupDeleteReq(nodeID string, objinfo *netproto.SecurityGroup) error
	OnSecurityGroupOperUpdate(nodeID string, objinfo *netproto.SecurityGroup) error
	OnSecurityGroupOperDelete(nodeID string, objinfo *netproto.SecurityGroup) error
	GetWatchFilter(kind string, ometa *api.ObjectMeta) func(memdb.Object) bool
}

type SecurityGroupNodeStatus struct {
	nodeID        string
	watcher       *memdb.Watcher
	opSentStatus  map[api.EventType]*EventStatus
	opAckedStatus map[api.EventType]*EventStatus
}

// SecurityGroupTopic is the SecurityGroup topic on message bus
type SecurityGroupTopic struct {
	sync.Mutex
	grpcServer    *rpckit.RPCServer // gRPC server instance
	server        *MbusServer
	statusReactor SecurityGroupStatusReactor // status event reactor
	nodeStatus    map[string]*SecurityGroupNodeStatus
}

// AddSecurityGroupTopic returns a network RPC server
func AddSecurityGroupTopic(server *MbusServer, reactor SecurityGroupStatusReactor) (*SecurityGroupTopic, error) {
	// RPC handler instance
	handler := SecurityGroupTopic{
		grpcServer:    server.grpcServer,
		server:        server,
		statusReactor: reactor,
		nodeStatus:    make(map[string]*SecurityGroupNodeStatus),
	}

	// register the RPC handlers
	if server.grpcServer != nil {
		netproto.RegisterSecurityGroupApiServer(server.grpcServer.GrpcServer, &handler)
	}

	return &handler, nil
}

func (eh *SecurityGroupTopic) registerWatcher(nodeID string, watcher *memdb.Watcher) {
	eh.Lock()
	defer eh.Unlock()

	eh.nodeStatus[nodeID] = &SecurityGroupNodeStatus{nodeID: nodeID, watcher: watcher}
	eh.nodeStatus[nodeID].opSentStatus = make(map[api.EventType]*EventStatus)
	eh.nodeStatus[nodeID].opAckedStatus = make(map[api.EventType]*EventStatus)
}

func (eh *SecurityGroupTopic) unRegisterWatcher(nodeID string) {
	eh.Lock()
	defer eh.Unlock()

	delete(eh.nodeStatus, nodeID)
}

//update recv object status
func (eh *SecurityGroupTopic) updateAckedObjStatus(nodeID string, event api.EventType, objMeta *api.ObjectMeta) {

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

		hdr.Record(nodeID+"_"+"SecurityGroup", delta)
		hdr.Record("SecurityGroup", delta)
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
func (eh *SecurityGroupTopic) updateSentObjStatus(nodeID string, event api.EventType, objMeta *api.ObjectMeta) {

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
func (eh *SecurityGroupTopic) WatcherInConfigSync(nodeID string, event api.EventType) bool {

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
		log.Infof("watcher %v still has objects in in-flight %v(%v)", nodeID, "SecurityGroup", event)
		return false
	}

	evAckStatus, ok = nodeStatus.opAckedStatus[event]
	if !ok {
		//nothing received, failed.
		log.Infof("watcher %v still has not received anything %v(%v)", nodeID, "SecurityGroup", event)
		return false
	}

	if evAckStatus.LastObjectMeta.ResourceVersion < evStatus.LastObjectMeta.ResourceVersion {
		log.Infof("watcher %v resource version mismatch for %v(%v)  sent %v: recived %v",
			nodeID, "SecurityGroup", event, evStatus.LastObjectMeta.ResourceVersion,
			evAckStatus.LastObjectMeta.ResourceVersion)
		return false
	}

	return true
}

/*
//GetSentEventStatus
func (eh *SecurityGroupTopic) GetSentEventStatus(nodeID string, event api.EventType) *EventStatus {

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
func (eh *SecurityGroupTopic) GetAckedEventStatus(nodeID string, event api.EventType) *EventStatus {

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

// CreateSecurityGroup creates SecurityGroup
func (eh *SecurityGroupTopic) CreateSecurityGroup(ctx context.Context, objinfo *netproto.SecurityGroup) (*netproto.SecurityGroup, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received CreateSecurityGroup from node %v: {%+v}", nodeID, objinfo)

	// trigger callbacks. we allow creates to happen before it exists in memdb
	if eh.statusReactor != nil {
		eh.statusReactor.OnSecurityGroupCreateReq(nodeID, objinfo)
	}

	// increment stats
	eh.server.Stats("SecurityGroup", "AgentCreate").Inc()

	return objinfo, nil
}

// UpdateSecurityGroup updates SecurityGroup
func (eh *SecurityGroupTopic) UpdateSecurityGroup(ctx context.Context, objinfo *netproto.SecurityGroup) (*netproto.SecurityGroup, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received UpdateSecurityGroup from node %v: {%+v}", nodeID, objinfo)

	// incr stats
	eh.server.Stats("SecurityGroup", "AgentUpdate").Inc()

	// trigger callbacks
	if eh.statusReactor != nil {
		eh.statusReactor.OnSecurityGroupUpdateReq(nodeID, objinfo)
	}

	return objinfo, nil
}

// DeleteSecurityGroup deletes an SecurityGroup
func (eh *SecurityGroupTopic) DeleteSecurityGroup(ctx context.Context, objinfo *netproto.SecurityGroup) (*netproto.SecurityGroup, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received DeleteSecurityGroup from node %v: {%+v}", nodeID, objinfo)

	// incr stats
	eh.server.Stats("SecurityGroup", "AgentDelete").Inc()

	// trigger callbacks
	if eh.statusReactor != nil {
		eh.statusReactor.OnSecurityGroupDeleteReq(nodeID, objinfo)
	}

	return objinfo, nil
}

// SecurityGroupFromObj converts memdb object to SecurityGroup
func SecurityGroupFromObj(obj memdb.Object) (*netproto.SecurityGroup, error) {
	switch obj.(type) {
	case *netproto.SecurityGroup:
		eobj := obj.(*netproto.SecurityGroup)
		return eobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// GetSecurityGroup returns a specific SecurityGroup
func (eh *SecurityGroupTopic) GetSecurityGroup(ctx context.Context, objmeta *api.ObjectMeta) (*netproto.SecurityGroup, error) {
	// find the object
	obj, err := eh.server.memDB.FindObject("SecurityGroup", objmeta)
	if err != nil {
		return nil, err
	}

	return SecurityGroupFromObj(obj)
}

// ListSecurityGroups lists all SecurityGroups matching object selector
func (eh *SecurityGroupTopic) ListSecurityGroups(ctx context.Context, objsel *api.ObjectMeta) (*netproto.SecurityGroupList, error) {
	var objlist netproto.SecurityGroupList
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)

	filterFn := func(memdb.Object) bool {
		return true
	}

	if eh.statusReactor != nil {
		filterFn = eh.statusReactor.GetWatchFilter("SecurityGroup", objsel)
	}

	// walk all objects
	objs := eh.server.memDB.ListObjects("SecurityGroup", filterFn)
	//creationTime, _ := types.TimestampProto(time.Now())
	for _, oo := range objs {
		obj, err := SecurityGroupFromObj(oo)
		if err == nil {
			//obj.CreationTime = api.Timestamp{Timestamp: *creationTime}
			objlist.SecurityGroups = append(objlist.SecurityGroups, obj)
			//record the last object sent to check config sync
			eh.updateSentObjStatus(nodeID, api.EventType_UpdateEvent, &obj.ObjectMeta)
		}
	}

	return &objlist, nil
}

// WatchSecurityGroups watches SecurityGroups and sends streaming resp
func (eh *SecurityGroupTopic) WatchSecurityGroups(ometa *api.ObjectMeta, stream netproto.SecurityGroupApi_WatchSecurityGroupsServer) error {
	// watch for changes
	watcher := memdb.Watcher{}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	defer close(watcher.Channel)

	if eh.statusReactor != nil {
		watcher.Filter = eh.statusReactor.GetWatchFilter("SecurityGroup", ometa)
	} else {
		watcher.Filter = func(memdb.Object) bool {
			return true
		}
	}

	ctx := stream.Context()
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	watcher.Name = nodeID
	eh.server.memDB.WatchObjects("SecurityGroup", &watcher)
	defer eh.server.memDB.StopWatchObjects("SecurityGroup", &watcher)

	// get a list of all SecurityGroups
	objlist, err := eh.ListSecurityGroups(context.Background(), ometa)
	if err != nil {
		log.Errorf("Error getting a list of objects. Err: %v", err)
		return err
	}

	eh.registerWatcher(nodeID, &watcher)
	defer eh.unRegisterWatcher(nodeID)

	// increment stats
	eh.server.Stats("SecurityGroup", "ActiveWatch").Inc()
	eh.server.Stats("SecurityGroup", "WatchConnect").Inc()
	defer eh.server.Stats("SecurityGroup", "ActiveWatch").Dec()
	defer eh.server.Stats("SecurityGroup", "WatchDisconnect").Inc()

	// walk all SecurityGroups and send it out
	watchEvts := netproto.SecurityGroupEventList{}
	for _, obj := range objlist.SecurityGroups {
		watchEvt := netproto.SecurityGroupEvent{
			EventType:     api.EventType_CreateEvent,
			SecurityGroup: *obj,
		}
		watchEvts.SecurityGroupEvents = append(watchEvts.SecurityGroupEvents, &watchEvt)
	}
	if len(watchEvts.SecurityGroupEvents) > 0 {
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending SecurityGroup to stream. Err: %v", err)
			return err
		}
	}
	timer := time.NewTimer(DefaultWatchHoldInterval)
	if !timer.Stop() {
		<-timer.C
	}

	running := false
	watchEvts = netproto.SecurityGroupEventList{}
	sendToStream := func() error {
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending SecurityGroup to stream. Err: %v", err)
			return err
		}
		watchEvts = netproto.SecurityGroupEventList{}
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
			obj, err := SecurityGroupFromObj(evt.Obj)
			if err != nil {
				return err
			}

			// convert to netproto format
			watchEvt := netproto.SecurityGroupEvent{
				EventType:     etype,
				SecurityGroup: *obj,
			}
			watchEvts.SecurityGroupEvents = append(watchEvts.SecurityGroupEvents, &watchEvt)
			if !running {
				running = true
				timer.Reset(DefaultWatchHoldInterval)
			}
			if len(watchEvts.SecurityGroupEvents) >= DefaultWatchBatchSize {
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

// updateSecurityGroupOper triggers oper update callbacks
func (eh *SecurityGroupTopic) updateSecurityGroupOper(oper *netproto.SecurityGroupEvent, nodeID string) error {
	eh.updateAckedObjStatus(nodeID, oper.EventType, &oper.SecurityGroup.ObjectMeta)
	switch oper.EventType {
	case api.EventType_CreateEvent:
		fallthrough
	case api.EventType_UpdateEvent:
		// incr stats
		eh.server.Stats("SecurityGroup", "AgentUpdate").Inc()

		// trigger callbacks
		if eh.statusReactor != nil {
			return eh.statusReactor.OnSecurityGroupOperUpdate(nodeID, &oper.SecurityGroup)
		}
	case api.EventType_DeleteEvent:
		// incr stats
		eh.server.Stats("SecurityGroup", "AgentDelete").Inc()

		// trigger callbacks
		if eh.statusReactor != nil {
			eh.statusReactor.OnSecurityGroupOperDelete(nodeID, &oper.SecurityGroup)
		}
	}

	return nil
}

func (eh *SecurityGroupTopic) SecurityGroupOperUpdate(stream netproto.SecurityGroupApi_SecurityGroupOperUpdateServer) error {
	ctx := stream.Context()
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)

	for {
		oper, err := stream.Recv()
		if err == io.EOF {
			log.Errorf("%v SecurityGroupOperUpdate stream ended. closing..", nodeID)
			return stream.SendAndClose(&api.TypeMeta{})
		} else if err != nil {
			log.Errorf("Error receiving from %v SecurityGroupOperUpdate stream. Err: %v", nodeID, err)
			return err
		}

		err = eh.updateSecurityGroupOper(oper, nodeID)
		if err != nil {
			log.Errorf("Error updating SecurityGroup oper state. Err: %v", err)
		}
	}
}
