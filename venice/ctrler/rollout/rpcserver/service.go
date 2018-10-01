package rpcserver

import (
	"context"
	"errors"
	"fmt"
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/ctrler/rollout/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// ServiceRolloutRPCServer implements ServiceRolloutApi interface
type ServiceRolloutRPCServer struct {
	stateMgr *statemgr.Statemgr
}

// NewServiceRolloutRPCServer returns a RPC server for Node update services
func NewServiceRolloutRPCServer(stateMgr *statemgr.Statemgr) (*ServiceRolloutRPCServer, error) {
	return &ServiceRolloutRPCServer{stateMgr: stateMgr}, nil
}

// WatchServiceRollout returns watches for ServiceRollout
func (n *ServiceRolloutRPCServer) WatchServiceRollout(sel *api.ObjectMeta, stream protos.ServiceRolloutApi_WatchServiceRolloutServer) error {
	var sentSpec protos.ServiceRolloutSpec

	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)
	n.stateMgr.WatchObjects("ServiceRollout", watchChan)
	defer n.stateMgr.StopWatchObjects("ServiceRollout", watchChan)

	rs, err := n.stateMgr.ListServiceRollouts()
	if err != nil {
		log.Errorf("Error listing existing ServiceRollout %v", err)
		return fmt.Errorf("Error %v listing exisitng ServiceRollout", err)
	}
	for _, svcRolloutState := range rs {
		watchEvt := protos.ServiceRolloutEvent{
			EventType:      api.EventType_CreateEvent,
			ServiceRollout: *svcRolloutState.ServiceRollout,
		}
		if reflect.DeepEqual(sentSpec, watchEvt.ServiceRollout.Spec) {
			log.Debugf("No spec update. suppressing watch")
			continue
		}
		sentSpec = watchEvt.ServiceRollout.Spec
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending serviceRollout Watch evt to %v Err: %v", sel.Name, err)
			return err
		}
		log.Debugf("Sent veniceRollout %#v ", watchEvt)
	}

	ctx := stream.Context()
	// loop forever on watch channel
	for {
		select {
		// read from channel
		case evt, ok := <-watchChan:
			if !ok {
				log.Errorf("Error reading from channel. Closing watch")
				return errors.New("Error reading from channel")
			}

			// get event type from memdb event
			var etype api.EventType
			switch evt.EventType {
			case memdb.CreateEvent:
				etype = api.EventType_CreateEvent
			case memdb.UpdateEvent:
				etype = api.EventType_UpdateEvent
			case memdb.DeleteEvent:
				etype = api.EventType_DeleteEvent
			}

			snRolloutState, err := statemgr.ServiceRolloutStateFromObj(evt.Obj)
			if err != nil {
				log.Errorf("Error getting ServiceRollout from statemgr. Err: %v", err)
				return err
			}

			// construct the NodeUpdate object
			watchEvt := protos.ServiceRolloutEvent{
				EventType: etype,
				ServiceRollout: protos.ServiceRollout{
					TypeMeta:   snRolloutState.TypeMeta,
					ObjectMeta: snRolloutState.ObjectMeta,
					Spec: protos.ServiceRolloutSpec{
						Ops: append([]*protos.ServiceOpSpec{}, snRolloutState.Spec.Ops...),
					},
				},
			}
			if etype != api.EventType_DeleteEvent && reflect.DeepEqual(sentSpec, watchEvt.ServiceRollout.Spec) {
				// No spec update. suppressing watch
				continue
			}
			if etype == api.EventType_DeleteEvent {
				sentSpec = protos.ServiceRolloutSpec{}
			} else {
				sentSpec = watchEvt.ServiceRollout.Spec
			}

			err = stream.Send(&watchEvt)
			if err != nil {
				log.Errorf("Error sending ServiceRollout Watch evt to %v Err: %v", sel.Name, err)
				return err
			}
			log.Debugf("Sent serviceRollout to %s ", watchEvt.ServiceRollout.Name)

		case <-ctx.Done():
			return ctx.Err()
		}
	}
}

// UpdateServiceRolloutStatus updates the status of individual Service Rollout
func (n *ServiceRolloutRPCServer) UpdateServiceRolloutStatus(ctx context.Context, status *protos.ServiceRolloutStatusUpdate) (*api.Empty, error) {

	sros, err := n.stateMgr.GetServiceRolloutState(status.ObjectMeta.Tenant, status.ObjectMeta.Name)
	if err != nil {
		log.Debugf("non-existent ServiceRollout {%+v}. Err: %v", status.ObjectMeta.Name, err)
		return &api.Empty{}, nil
	}
	sros.UpdateServiceRolloutStatus(&status.Status)
	return &api.Empty{}, nil
}
