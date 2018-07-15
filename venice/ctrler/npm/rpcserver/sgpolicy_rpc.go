// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"errors"
	"fmt"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// SGPolicyRPCServer serves security rpc requests
type SGPolicyRPCServer struct {
	stateMgr *statemgr.Statemgr // reference to network manager
}

// GetSGPolicy gets sg policy
func (s *SGPolicyRPCServer) GetSGPolicy(ctx context.Context, ometa *api.ObjectMeta) (*netproto.SGPolicy, error) {
	// find the sgp
	sgp, err := s.stateMgr.FindSgpolicy(ometa.Tenant, ometa.Name)
	if err != nil {
		log.Errorf("Could not find the sg %s|%s", ometa.Tenant, ometa.Name)
		return nil, fmt.Errorf("could not find the sg policy %v", ometa.Name)
	}

	// build sg message
	sg := netproto.SGPolicy{
		TypeMeta:   sgp.TypeMeta,
		ObjectMeta: sgp.ObjectMeta,
		Spec: netproto.SGPolicySpec{
			AttachTenant: sgp.Spec.AttachTenant,
			AttachGroup:  sgp.Spec.AttachGroups,
			Rules:        []netproto.PolicyRule{}, // TODO: fill in fw rules and handle app stuff
		},
	}

	return &sg, nil
}

// ListSGPolicys returns a list of sg policies
func (s *SGPolicyRPCServer) ListSGPolicys(context.Context, *api.ObjectMeta) (*netproto.SGPolicyList, error) {
	var sgpList netproto.SGPolicyList
	// get all sgs
	sgPolicies, err := s.stateMgr.ListSgpolicies()
	if err != nil {
		return nil, err
	}

	// walk all sgs in the list
	for _, sgp := range sgPolicies {

		// build sg message
		sg := netproto.SGPolicy{
			TypeMeta:   sgp.TypeMeta,
			ObjectMeta: sgp.ObjectMeta,
			Spec: netproto.SGPolicySpec{
				AttachTenant: sgp.Spec.AttachTenant,
				AttachGroup:  sgp.Spec.AttachGroups,
				Rules:        []netproto.PolicyRule{}, // TODO: fill in fw rules and handle app stuff
			},
		}
		sgpList.Sgpolicies = append(sgpList.Sgpolicies, &sg)
	}

	return &sgpList, nil
}

// WatchSGPolicys watches for sg changes
func (s *SGPolicyRPCServer) WatchSGPolicys(sel *api.ObjectMeta, stream netproto.SGPolicyApi_WatchSGPolicysServer) error {
	// watch for changes
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)
	s.stateMgr.WatchObjects("SGPolicy", watchChan)
	defer s.stateMgr.StopWatchObjects("SGPolicy", watchChan)

	// get a list of all sg policies
	sgPolicies, err := s.ListSGPolicys(context.Background(), sel)
	if err != nil {
		log.Errorf("Error getting a list of sgs. Err: %v", err)
		return err
	}

	ctx := stream.Context()

	// send the objects out as a stream
	for _, sgp := range sgPolicies.Sgpolicies {
		watchEvt := netproto.SGPolicyEvent{
			EventType: api.EventType_CreateEvent,
			SGPolicy:  *sgp,
		}
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
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

			// convert to sg policy object
			policyState, err := statemgr.SgpolicyStateFromObj(evt.Obj)
			if err != nil {
				continue
			}

			// construct the netproto object
			watchEvt := netproto.SGPolicyEvent{
				EventType: etype,
				SGPolicy: netproto.SGPolicy{
					TypeMeta:   policyState.TypeMeta,
					ObjectMeta: policyState.ObjectMeta,
					Spec: netproto.SGPolicySpec{
						AttachTenant: policyState.Spec.AttachTenant,
						AttachGroup:  policyState.Spec.AttachGroups,
						Rules:        []netproto.PolicyRule{}, // TODO: fill in fw rules and handle app stuff
					},
				},
			}

			err = stream.Send(&watchEvt)
			if err != nil {
				log.Errorf("Error sending stream. Err: %v", err)
				return err
			}
		case <-ctx.Done():
			return ctx.Err()
		}
	}

	// done
}

// NewSGPolicyRPCServer returns a security RPC server
func NewSGPolicyRPCServer(stateMgr *statemgr.Statemgr) (*SGPolicyRPCServer, error) {
	return &SGPolicyRPCServer{stateMgr: stateMgr}, nil
}
