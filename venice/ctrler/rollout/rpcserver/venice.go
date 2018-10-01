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

// VeniceRolloutRPCServer implements VeniceRolloutApi interface
type VeniceRolloutRPCServer struct {
	stateMgr *statemgr.Statemgr
}

// NewVeniceRolloutRPCServer returns a RPC server for Node update services
func NewVeniceRolloutRPCServer(stateMgr *statemgr.Statemgr) (*VeniceRolloutRPCServer, error) {
	return &VeniceRolloutRPCServer{stateMgr: stateMgr}, nil
}

// WatchVeniceRollout returns watches for VeniceRollout
func (n *VeniceRolloutRPCServer) WatchVeniceRollout(sel *api.ObjectMeta, stream protos.VeniceRolloutApi_WatchVeniceRolloutServer) error {
	var sentSpec protos.VeniceRolloutSpec

	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)
	n.stateMgr.WatchObjects("VeniceRollout", watchChan)
	defer n.stateMgr.StopWatchObjects("VeniceRollout", watchChan)

	log.Debugf("Watcher for VeniceRollout for %v", sel.Name)
	rs, err := n.stateMgr.ListVeniceRollouts()
	if err != nil {
		log.Errorf("Error listing existing VeniceRollout %v", err)
		return fmt.Errorf("Error %v listing exisitng VeniceRollout", err)
	}
	for _, vRolloutState := range rs {
		if vRolloutState.ObjectMeta.Name != sel.Name {
			log.Debugf("Ignoring update of other VeniceRollout %v Interested in only %v ", vRolloutState.ObjectMeta.Name, sel.Name)
			continue
		}
		watchEvt := protos.VeniceRolloutEvent{
			EventType:     api.EventType_CreateEvent,
			VeniceRollout: *vRolloutState.VeniceRollout,
		}
		if reflect.DeepEqual(sentSpec, watchEvt.VeniceRollout.Spec) {
			log.Debugf("No spec update. suppressing watch")
			continue
		}
		sentSpec = watchEvt.VeniceRollout.Spec
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending veniceRollout Watch evt to %v Err: %v", sel.Name, err)
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
				return errors.New("Error reading from watch channel")
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

			vRolloutState, err := statemgr.VeniceRolloutStateFromObj(evt.Obj)
			if err != nil {
				log.Errorf("Error getting veniceRollout from statemgr. Err: %v", err)
				return err
			}
			if vRolloutState.ObjectMeta.Name != sel.Name {
				continue
			}

			// construct the NodeUpdate object
			watchEvt := protos.VeniceRolloutEvent{
				EventType: etype,
				VeniceRollout: protos.VeniceRollout{
					TypeMeta:   vRolloutState.TypeMeta,
					ObjectMeta: vRolloutState.ObjectMeta,
					Spec: protos.VeniceRolloutSpec{
						Ops: append([]*protos.VeniceOpSpec{}, vRolloutState.Spec.Ops...),
					},
				},
			}
			if etype != api.EventType_DeleteEvent && reflect.DeepEqual(sentSpec, watchEvt.VeniceRollout.Spec) {
				// No spec update. suppressing watch
				continue
			}
			if etype == api.EventType_DeleteEvent {
				sentSpec = protos.VeniceRolloutSpec{}
			} else {
				sentSpec = watchEvt.VeniceRollout.Spec
			}

			err = stream.Send(&watchEvt)
			if err != nil {
				log.Errorf("Error sending veniceRollout Watch evt to %v Err: %v", sel.Name, err)
				return err
			}
			log.Debugf("Sent veniceRollout to %s ", watchEvt.VeniceRollout.Name)

		case <-ctx.Done():
			return ctx.Err()
		}
	}
}

// UpdateVeniceRolloutStatus updates the status of individual Venice Rollout
func (n *VeniceRolloutRPCServer) UpdateVeniceRolloutStatus(ctx context.Context, status *protos.VeniceRolloutStatusUpdate) (*api.Empty, error) {
	vros, err := n.stateMgr.GetVeniceRolloutState(status.ObjectMeta.Tenant, status.ObjectMeta.Name)
	if err != nil {
		return &api.Empty{}, fmt.Errorf("non-existent VeniceRollout {%+v}. Err: %v", status.ObjectMeta.Name, err)
	}
	err = vros.UpdateVeniceRolloutStatus(&status.Status)
	return &api.Empty{}, err
}
