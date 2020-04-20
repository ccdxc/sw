// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package nimbus is a auto generated package.
Input file: vrf.proto
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

// FindVrf finds an Vrf by object meta
func (ms *MbusServer) FindVrf(objmeta *api.ObjectMeta) (*netproto.Vrf, error) {
	// find the object
	obj, err := ms.memDB.FindObject("Vrf", objmeta)
	if err != nil {
		return nil, err
	}

	return VrfFromObj(obj)
}

// ListVrfs lists all Vrfs in the mbus
func (ms *MbusServer) ListVrfs(ctx context.Context, nodeID string, filters []memdb.FilterFn) ([]*netproto.Vrf, error) {
	var objlist []*netproto.Vrf

	// walk all objects
	objs := ms.memDB.ListObjectsForReceiver("Vrf", nodeID, filters)
	for _, oo := range objs {
		obj, err := VrfFromObj(oo)
		if err == nil {
			objlist = append(objlist, obj)
		}
	}

	return objlist, nil
}

// ListVrfsNoFilter lists all Vrfs in the mbus without applying a watch filter
func (ms *MbusServer) ListVrfsNoFilter(ctx context.Context) ([]*netproto.Vrf, error) {
	var objlist []*netproto.Vrf

	// walk all objects
	objs := ms.memDB.ListObjects("Vrf", nil)
	for _, oo := range objs {
		obj, err := VrfFromObj(oo)
		if err == nil {
			objlist = append(objlist, obj)
		}
	}

	return objlist, nil
}

// VrfStatusReactor is the reactor interface implemented by controllers
type VrfStatusReactor interface {
	OnVrfCreateReq(nodeID string, objinfo *netproto.Vrf) error
	OnVrfUpdateReq(nodeID string, objinfo *netproto.Vrf) error
	OnVrfDeleteReq(nodeID string, objinfo *netproto.Vrf) error
	OnVrfOperUpdate(nodeID string, objinfo *netproto.Vrf) error
	OnVrfOperDelete(nodeID string, objinfo *netproto.Vrf) error
	GetAgentWatchFilter(ctx context.Context, kind string, watchOptions *api.ListWatchOptions) []memdb.FilterFn
}

type VrfNodeStatus struct {
	nodeID        string
	watcher       *memdb.Watcher
	opSentStatus  map[api.EventType]*EventStatus
	opAckedStatus map[api.EventType]*EventStatus
}

// VrfTopic is the Vrf topic on message bus
type VrfTopic struct {
	sync.Mutex
	grpcServer    *rpckit.RPCServer // gRPC server instance
	server        *MbusServer
	statusReactor VrfStatusReactor // status event reactor
	nodeStatus    map[string]*VrfNodeStatus
}

// AddVrfTopic returns a network RPC server
func AddVrfTopic(server *MbusServer, reactor VrfStatusReactor) (*VrfTopic, error) {
	// RPC handler instance
	handler := VrfTopic{
		grpcServer:    server.grpcServer,
		server:        server,
		statusReactor: reactor,
		nodeStatus:    make(map[string]*VrfNodeStatus),
	}

	// register the RPC handlers
	if server.grpcServer != nil {
		netproto.RegisterVrfApiV1Server(server.grpcServer.GrpcServer, &handler)
	}

	return &handler, nil
}

func (eh *VrfTopic) registerWatcher(nodeID string, watcher *memdb.Watcher) {
	eh.Lock()
	defer eh.Unlock()

	eh.nodeStatus[nodeID] = &VrfNodeStatus{nodeID: nodeID, watcher: watcher}
	eh.nodeStatus[nodeID].opSentStatus = make(map[api.EventType]*EventStatus)
	eh.nodeStatus[nodeID].opAckedStatus = make(map[api.EventType]*EventStatus)
}

func (eh *VrfTopic) unRegisterWatcher(nodeID string) {
	eh.Lock()
	defer eh.Unlock()

	delete(eh.nodeStatus, nodeID)
}

//update recv object status
func (eh *VrfTopic) updateAckedObjStatus(nodeID string, event api.EventType, objMeta *api.ObjectMeta) {

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

		hdr.Record(nodeID+"_"+"Vrf", delta)
		hdr.Record("Vrf", delta)
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
func (eh *VrfTopic) updateSentObjStatus(nodeID string, event api.EventType, objMeta *api.ObjectMeta) {

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
func (eh *VrfTopic) WatcherInConfigSync(nodeID string, event api.EventType) bool {

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
		log.Infof("watcher %v still has objects in in-flight %v(%v)", nodeID, "Vrf", event)
		return false
	}

	evAckStatus, ok = nodeStatus.opAckedStatus[event]
	if !ok {
		//nothing received, failed.
		log.Infof("watcher %v still has not received anything %v(%v)", nodeID, "Vrf", event)
		return false
	}

	if evAckStatus.LastObjectMeta.ResourceVersion < evStatus.LastObjectMeta.ResourceVersion {
		log.Infof("watcher %v resource version mismatch for %v(%v)  sent %v: recived %v",
			nodeID, "Vrf", event, evStatus.LastObjectMeta.ResourceVersion,
			evAckStatus.LastObjectMeta.ResourceVersion)
		return false
	}

	return true
}

/*
//GetSentEventStatus
func (eh *VrfTopic) GetSentEventStatus(nodeID string, event api.EventType) *EventStatus {

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
func (eh *VrfTopic) GetAckedEventStatus(nodeID string, event api.EventType) *EventStatus {

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

// CreateVrf creates Vrf
func (eh *VrfTopic) CreateVrf(ctx context.Context, objinfo *netproto.Vrf) (*netproto.Vrf, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received CreateVrf from node %v: {%+v}", nodeID, objinfo)

	// trigger callbacks. we allow creates to happen before it exists in memdb
	if eh.statusReactor != nil {
		eh.statusReactor.OnVrfCreateReq(nodeID, objinfo)
	}

	// increment stats
	eh.server.Stats("Vrf", "AgentCreate").Inc()

	return objinfo, nil
}

// UpdateVrf updates Vrf
func (eh *VrfTopic) UpdateVrf(ctx context.Context, objinfo *netproto.Vrf) (*netproto.Vrf, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received UpdateVrf from node %v: {%+v}", nodeID, objinfo)

	// incr stats
	eh.server.Stats("Vrf", "AgentUpdate").Inc()

	// trigger callbacks
	if eh.statusReactor != nil {
		eh.statusReactor.OnVrfUpdateReq(nodeID, objinfo)
	}

	return objinfo, nil
}

// DeleteVrf deletes an Vrf
func (eh *VrfTopic) DeleteVrf(ctx context.Context, objinfo *netproto.Vrf) (*netproto.Vrf, error) {
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	log.Infof("Received DeleteVrf from node %v: {%+v}", nodeID, objinfo)

	// incr stats
	eh.server.Stats("Vrf", "AgentDelete").Inc()

	// trigger callbacks
	if eh.statusReactor != nil {
		eh.statusReactor.OnVrfDeleteReq(nodeID, objinfo)
	}

	return objinfo, nil
}

// VrfFromObj converts memdb object to Vrf
func VrfFromObj(obj memdb.Object) (*netproto.Vrf, error) {
	switch obj.(type) {
	case *netproto.Vrf:
		eobj := obj.(*netproto.Vrf)
		return eobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// GetVrf returns a specific Vrf
func (eh *VrfTopic) GetVrf(ctx context.Context, objmeta *api.ObjectMeta) (*netproto.Vrf, error) {
	// find the object
	obj, err := eh.server.memDB.FindObject("Vrf", objmeta)
	if err != nil {
		return nil, err
	}

	return VrfFromObj(obj)
}

// ListVrfs lists all Vrfs matching object selector
func (eh *VrfTopic) ListVrfs(ctx context.Context, objsel *api.ListWatchOptions) (*netproto.VrfList, error) {
	var objlist netproto.VrfList
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)
	filters := []memdb.FilterFn{}

	filterFn := func(obj, prev memdb.Object) bool {
		return true
	}

	if eh.statusReactor != nil {
		filters = eh.statusReactor.GetAgentWatchFilter(ctx, "netproto.Vrf", objsel)
	} else {
		filters = append(filters, filterFn)
	}

	// walk all objects
	objs := eh.server.memDB.ListObjectsForReceiver("Vrf", nodeID, filters)
	//creationTime, _ := types.TimestampProto(time.Now())
	for _, oo := range objs {
		obj, err := VrfFromObj(oo)
		if err == nil {
			//obj.CreationTime = api.Timestamp{Timestamp: *creationTime}
			objlist.Vrfs = append(objlist.Vrfs, obj)
			//record the last object sent to check config sync
			eh.updateSentObjStatus(nodeID, api.EventType_UpdateEvent, &obj.ObjectMeta)
		}
	}

	return &objlist, nil
}

// WatchVrfs watches Vrfs and sends streaming resp
func (eh *VrfTopic) WatchVrfs(watchOptions *api.ListWatchOptions, stream netproto.VrfApiV1_WatchVrfsServer) error {
	// watch for changes
	watcher := memdb.Watcher{}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	watcher.Filters = make(map[string][]memdb.FilterFn)
	defer close(watcher.Channel)

	ctx := stream.Context()
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)

	if eh.statusReactor != nil {
		watcher.Filters["Vrf"] = eh.statusReactor.GetAgentWatchFilter(ctx, "Vrf", watchOptions)
	} else {
		filt := func(obj, prev memdb.Object) bool {
			return true
		}
		watcher.Filters["Vrf"] = append(watcher.Filters["Vrf"], filt)
	}

	watcher.Name = nodeID
	err := eh.server.memDB.WatchObjects("Vrf", &watcher)
	if err != nil {
		log.Errorf("Error Starting watch for kind %v Err: %v", "Vrf", err)
		return err
	}
	defer eh.server.memDB.StopWatchObjects("Vrf", &watcher)

	// get a list of all Vrfs
	objlist, err := eh.ListVrfs(context.Background(), watchOptions)
	if err != nil {
		log.Errorf("Error getting a list of objects. Err: %v", err)
		return err
	}

	eh.registerWatcher(nodeID, &watcher)
	defer eh.unRegisterWatcher(nodeID)

	// increment stats
	eh.server.Stats("Vrf", "ActiveWatch").Inc()
	eh.server.Stats("Vrf", "WatchConnect").Inc()
	defer eh.server.Stats("Vrf", "ActiveWatch").Dec()
	defer eh.server.Stats("Vrf", "WatchDisconnect").Inc()

	// walk all Vrfs and send it out
	watchEvts := netproto.VrfEventList{}
	for _, obj := range objlist.Vrfs {
		watchEvt := netproto.VrfEvent{
			EventType: api.EventType_CreateEvent,

			Vrf: *obj,
		}
		watchEvts.VrfEvents = append(watchEvts.VrfEvents, &watchEvt)
	}
	if len(watchEvts.VrfEvents) > 0 {
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending Vrf to stream. Err: %v", err)
			return err
		}
	}
	timer := time.NewTimer(DefaultWatchHoldInterval)
	if !timer.Stop() {
		<-timer.C
	}

	running := false
	watchEvts = netproto.VrfEventList{}
	sendToStream := func() error {
		err = stream.Send(&watchEvts)
		if err != nil {
			log.Errorf("Error sending Vrf to stream. Err: %v", err)
			return err
		}
		watchEvts = netproto.VrfEventList{}
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
			obj, err := VrfFromObj(evt.Obj)
			if err != nil {
				return err
			}

			// convert to netproto format
			watchEvt := netproto.VrfEvent{
				EventType: etype,

				Vrf: *obj,
			}
			watchEvts.VrfEvents = append(watchEvts.VrfEvents, &watchEvt)
			if !running {
				running = true
				timer.Reset(DefaultWatchHoldInterval)
			}
			if len(watchEvts.VrfEvents) >= DefaultWatchBatchSize {
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

// updateVrfOper triggers oper update callbacks
func (eh *VrfTopic) updateVrfOper(oper *netproto.VrfEvent, nodeID string) error {
	eh.updateAckedObjStatus(nodeID, oper.EventType, &oper.Vrf.ObjectMeta)
	switch oper.EventType {
	case api.EventType_CreateEvent:
		fallthrough
	case api.EventType_UpdateEvent:
		// incr stats
		eh.server.Stats("Vrf", "AgentUpdate").Inc()

		// trigger callbacks
		if eh.statusReactor != nil {

			return eh.statusReactor.OnVrfOperUpdate(nodeID, &oper.Vrf)

		}
	case api.EventType_DeleteEvent:
		// incr stats
		eh.server.Stats("Vrf", "AgentDelete").Inc()

		// trigger callbacks
		if eh.statusReactor != nil {

			eh.statusReactor.OnVrfOperDelete(nodeID, &oper.Vrf)

		}
	}

	return nil
}

func (eh *VrfTopic) VrfOperUpdate(stream netproto.VrfApiV1_VrfOperUpdateServer) error {
	ctx := stream.Context()
	nodeID := netutils.GetNodeUUIDFromCtx(ctx)

	for {
		oper, err := stream.Recv()
		if err == io.EOF {
			log.Errorf("%v VrfOperUpdate stream ended. closing..", nodeID)
			return stream.SendAndClose(&api.TypeMeta{})
		} else if err != nil {
			log.Errorf("Error receiving from %v VrfOperUpdate stream. Err: %v", nodeID, err)
			return err
		}

		err = eh.updateVrfOper(oper, nodeID)
		if err != nil {
			log.Errorf("Error updating Vrf oper state. Err: %v", err)
		}
	}
}
