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

// SmartNICRolloutRPCServer implements SmartNICRolloutApi interface
type SmartNICRolloutRPCServer struct {
	stateMgr *statemgr.Statemgr
}

// NewSmartNICRolloutRPCServer returns a RPC server for Node update services
func NewSmartNICRolloutRPCServer(stateMgr *statemgr.Statemgr) (*SmartNICRolloutRPCServer, error) {
	return &SmartNICRolloutRPCServer{stateMgr: stateMgr}, nil
}

// WatchSmartNICRollout returns watches for SmartNICRollout
func (n *SmartNICRolloutRPCServer) WatchSmartNICRollout(sel *api.ObjectMeta, stream protos.SmartNICRolloutApi_WatchSmartNICRolloutServer) error {
	var sentSpec protos.SmartNICRolloutSpec

	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)
	n.stateMgr.WatchObjects("SmartNICRollout", watchChan)
	defer n.stateMgr.StopWatchObjects("SmartNICRollout", watchChan)

	log.Debugf("Got WatchSmartNICRollout from %v", sel.Name)
	rs, err := n.stateMgr.ListSmartNICRollouts()
	if err != nil {
		log.Errorf("Error listing existing SmartNICRollout %v", err)
		return fmt.Errorf("Error %v listing exisitng SmartNICRollout", err)
	}
	for _, snRolloutState := range rs {
		if snRolloutState.ObjectMeta.Name != sel.Name {
			log.Debugf("Ignoring %s Looking for %s", snRolloutState.ObjectMeta.Name, sel.Name)
			continue
		}

		watchEvt := protos.SmartNICRolloutEvent{
			EventType:       api.EventType_CreateEvent,
			SmartNICRollout: *snRolloutState.SmartNICRollout,
		}
		if reflect.DeepEqual(sentSpec, watchEvt.SmartNICRollout.Spec) {
			log.Debugf("No spec update. suppressing watch")
			continue
		}
		sentSpec = watchEvt.SmartNICRollout.Spec
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending smartNICRollout Watch evt to %v Err: %v", sel.Name, err)
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

			snRolloutState, err := statemgr.SmartNICRolloutStateFromObj(evt.Obj)
			if err != nil {
				log.Errorf("Error getting smartNICRollout from statemgr. Err: %v", err)
				return err
			}
			if snRolloutState.ObjectMeta.Name != sel.Name {
				continue
			}

			// construct the NodeUpdate object
			watchEvt := protos.SmartNICRolloutEvent{
				EventType: etype,
				SmartNICRollout: protos.SmartNICRollout{
					TypeMeta:   snRolloutState.TypeMeta,
					ObjectMeta: snRolloutState.ObjectMeta,
					Spec: protos.SmartNICRolloutSpec{
						Ops: append([]*protos.SmartNICOpSpec{}, snRolloutState.Spec.Ops...),
					},
				},
			}

			if etype != api.EventType_DeleteEvent && reflect.DeepEqual(sentSpec, watchEvt.SmartNICRollout.Spec) {
				// No spec update. suppressing watch
				continue
			}
			if etype == api.EventType_DeleteEvent {
				sentSpec = protos.SmartNICRolloutSpec{}
			} else {
				sentSpec = watchEvt.SmartNICRollout.Spec
			}

			err = stream.Send(&watchEvt)
			if err != nil {
				log.Errorf("Error sending smartNICRollout Watch evt to %v Err: %v", sel.Name, err)
				return err
			}
			log.Debugf("Sent smartNICRollout to %s ", watchEvt.SmartNICRollout.Name)

		case <-ctx.Done():
			return ctx.Err()
		}
	}
}

// UpdateSmartNICRolloutStatus updates the status of individual SmartNIC Rollout
func (n *SmartNICRolloutRPCServer) UpdateSmartNICRolloutStatus(ctx context.Context, status *protos.SmartNICRolloutStatusUpdate) (*api.Empty, error) {
	sros, err := n.stateMgr.GetSmartNICRolloutState(
		status.ObjectMeta.Tenant,
		status.ObjectMeta.Name)
	if err != nil {
		log.Debugf("non-existent SmartNICRollout {%+v}. Err: %v", status.ObjectMeta.Name, err)
		return &api.Empty{}, fmt.Errorf("non-existent SmartNICRollout {%+v}. Err: %v", status.ObjectMeta.Name, err)
	}

	sros.UpdateSmartNICRolloutStatus(&status.Status)
	return &api.Empty{}, nil
}
