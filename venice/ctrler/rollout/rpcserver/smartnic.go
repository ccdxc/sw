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

// DSCRolloutRPCServer implements DSCRolloutApi interface
type DSCRolloutRPCServer struct {
	stateMgr *statemgr.Statemgr
}

// NewDSCRolloutRPCServer returns a RPC server for Node update services
func NewDSCRolloutRPCServer(stateMgr *statemgr.Statemgr) (*DSCRolloutRPCServer, error) {
	return &DSCRolloutRPCServer{stateMgr: stateMgr}, nil
}

// WatchDSCRollout returns watches for DSCRollout
func (n *DSCRolloutRPCServer) WatchDSCRollout(sel *api.ObjectMeta, stream protos.DSCRolloutApi_WatchDSCRolloutServer) error {
	var sentSpec protos.DSCRolloutSpec

	watcher := memdb.Watcher{}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	defer close(watcher.Channel)
	n.stateMgr.WatchObjects("DSCRollout", &watcher)
	defer n.stateMgr.StopWatchObjects("DSCRollout", &watcher)

	log.Debugf("Got WatchDSCRollout from %v", sel.Name)
	rs, err := n.stateMgr.ListDSCRollouts()
	if err != nil {
		log.Errorf("Error listing existing DSCRollout %v", err)
		return fmt.Errorf("error %v listing existing DSCRollout", err)
	}
	for _, snRolloutState := range rs {
		if snRolloutState.ObjectMeta.Name != sel.Name {
			log.Debugf("Ignoring %s Looking for %s", snRolloutState.ObjectMeta.Name, sel.Name)
			continue
		}

		watchEvt := protos.DSCRolloutEvent{
			EventType:  api.EventType_CreateEvent,
			DSCRollout: *snRolloutState.DSCRollout,
		}
		if reflect.DeepEqual(sentSpec, watchEvt.DSCRollout.Spec) {
			log.Debugf("No spec update. suppressing watch")
			continue
		}
		sentSpec = watchEvt.DSCRollout.Spec
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
		case evt, ok := <-watcher.Channel:
			if !ok {
				log.Errorf("Error reading from channel. Closing watch")
				return errors.New("error reading from channel")
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

			snRolloutState, err := statemgr.DSCRolloutStateFromObj(evt.Obj)
			if err != nil {
				log.Errorf("Error getting smartNICRollout from statemgr. Err: %v", err)
				return err
			}
			if snRolloutState.ObjectMeta.Name != sel.Name {
				continue
			}

			// construct the NodeUpdate object
			watchEvt := protos.DSCRolloutEvent{
				EventType: etype,
				DSCRollout: protos.DSCRollout{
					TypeMeta:   snRolloutState.TypeMeta,
					ObjectMeta: snRolloutState.ObjectMeta,
					Spec: protos.DSCRolloutSpec{
						Ops: append([]protos.DSCOpSpec{}, snRolloutState.Spec.Ops...),
					},
				},
			}

			if etype != api.EventType_DeleteEvent && reflect.DeepEqual(sentSpec, watchEvt.DSCRollout.Spec) {
				// No spec update. suppressing watch
				continue
			}

			sentSpec = protos.DSCRolloutSpec{}
			if etype != api.EventType_DeleteEvent {
				sentSpec.Ops = watchEvt.DSCRollout.Spec.Ops
			}

			err = stream.Send(&watchEvt)
			if err != nil {
				log.Errorf("Error sending smartNICRollout Watch evt to %v Err: %v", sel.Name, err)
				return err
			}
			log.Debugf("Sent smartNICRollout Spec:%v to %s ", watchEvt.DSCRollout.Spec, watchEvt.DSCRollout.Name)

		case <-ctx.Done():
			return ctx.Err()
		}
	}
}

// UpdateDSCRolloutStatus updates the status of individual SmartNIC Rollout
func (n *DSCRolloutRPCServer) UpdateDSCRolloutStatus(ctx context.Context, status *protos.DSCRolloutStatusUpdate) (*api.Empty, error) {
	sros, err := n.stateMgr.GetDSCRolloutState(
		status.ObjectMeta.Tenant,
		status.ObjectMeta.Name)
	if err != nil {
		log.Infof("non-existent DSCRollout {%+v}. Err: %v", status.ObjectMeta.Name, err)
		return &api.Empty{}, nil
	}

	sros.UpdateDSCRolloutStatus(&status.Status)
	return &api.Empty{}, nil
}
