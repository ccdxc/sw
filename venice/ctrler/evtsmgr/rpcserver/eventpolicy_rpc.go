package rpcserver

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	emgrpc "github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

var pkgName = "rpcserver"
var rpcLogger = log.WithContext("pkg", "rpcserver")

// convert memdb event type to api event type
var apiEventTypeMap = map[memdb.EventType]api.EventType{
	memdb.CreateEvent: api.EventType_CreateEvent,
	memdb.UpdateEvent: api.EventType_UpdateEvent,
	memdb.DeleteEvent: api.EventType_DeleteEvent,
}

// EventPolicyRPCHandler is the event policy RPC handler
type EventPolicyRPCHandler struct {
	policyDb *memdb.Memdb
	logger   log.Logger
}

// NewEventPolicyRPCHandler returns a event policy RPC server
func NewEventPolicyRPCHandler(memDb *memdb.Memdb, logger log.Logger) (*EventPolicyRPCHandler, error) {
	return &EventPolicyRPCHandler{policyDb: memDb, logger: logger}, nil
}

// WatchEventPolicy watches the event policy objects on mem DB
func (e *EventPolicyRPCHandler) WatchEventPolicy(in *api.ObjectMeta, stream emgrpc.EventPolicyAPI_WatchEventPolicyServer) error {
	e.logger.Infof("watch event policy from [%v]", in.Name)

	// watch for changes
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)
	e.policyDb.WatchObjects("EventPolicy", watchChan)
	defer e.policyDb.StopWatchObjects("EventPolicy", watchChan)

	// send existing event policies to the watcher
	for _, obj := range e.policyDb.ListObjects("EventPolicy") {
		if policy, ok := obj.(*monitoring.EventPolicy); ok {
			// convert monitoring.EventPolicy to evtsmgrprotos.EventPolicy
			eventPolicy := &emgrpc.EventPolicy{
				TypeMeta:   policy.TypeMeta,
				ObjectMeta: policy.ObjectMeta,
				Spec:       policy.Spec,
				Status:     policy.Status,
			}
			if err := stream.Send(&emgrpc.EventPolicyEvent{EventType: api.EventType_CreateEvent,
				Policy: eventPolicy}); err != nil {
				e.logger.Errorf("failed to send event policy to %s, error %s", in.GetName(), err)
				return err
			}
		} else {
			e.logger.Errorf("invalid event policy object from list %+v", obj)
			return fmt.Errorf("invalid event policy object from list")
		}
	}

	ctx := stream.Context()

	// loop forever on watch channel
	for {
		select {
		case event, ok := <-watchChan:
			if !ok {
				e.logger.Errorf("[%s] error received from", in.GetName())
				return fmt.Errorf("invalid event from watch channel")
			}

			policy, ok := event.Obj.(*monitoring.EventPolicy)
			if !ok {
				e.logger.Errorf("event policy watch error received from [%s]", in.GetName())
				return fmt.Errorf("watch error")
			}

			// convert monitoring.EventPolicy to evtsmgrprotos.EventPolicy
			eventPolicy := &emgrpc.EventPolicy{
				TypeMeta:   policy.TypeMeta,
				ObjectMeta: policy.ObjectMeta,
				Spec:       policy.Spec,
				Status:     policy.Status,
			}
			if err := stream.Send(&emgrpc.EventPolicyEvent{EventType: apiEventTypeMap[event.EventType],
				Policy: eventPolicy}); err != nil {
				e.logger.Errorf("failed to send event policy to %s, error %s", in.GetName(), err)
				return err
			}
		case <-ctx.Done():
			e.logger.Errorf("context canceled from %v, error:%s", in.GetName(), ctx.Err())
			return fmt.Errorf("context cancelled")
		}
	}
}
