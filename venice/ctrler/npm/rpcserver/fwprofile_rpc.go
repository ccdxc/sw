// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"errors"
	"fmt"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// FirewallProfileRPCServer serves firewall profile rpc requests
type FirewallProfileRPCServer struct {
	stateMgr *statemgr.Statemgr // reference to network manager
}

// convertFirewallProfile converts fw profile state to security profile
func convertFirewallProfile(fps *statemgr.FirewallProfileState) *netproto.SecurityProfile {
	// build sg message
	fwp := netproto.SecurityProfile{
		TypeMeta:   api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: fps.ObjectMeta,
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        fps.Spec.SessionIdleTimeout,
				TCP:                fps.Spec.TcpTimeout,
				TCPDrop:            fps.Spec.TCPDropTimeout,
				TCPConnectionSetup: fps.Spec.TCPConnectionSetupTimeout,
				TCPClose:           fps.Spec.TCPCloseTimeout,
				TCPHalfClose:       fps.Spec.TCPHalfClosedTimeout,
				Drop:               fps.Spec.DropTimeout,
				UDP:                fps.Spec.UdpTimeout,
				UDPDrop:            fps.Spec.UDPDropTimeout,
				ICMP:               fps.Spec.IcmpTimeout,
				ICMPDrop:           fps.Spec.ICMPDropTimeout,
			},
		},
	}

	return &fwp
}

// GetSecurityProfile gets firewall profile
func (s *FirewallProfileRPCServer) GetSecurityProfile(ctx context.Context, ometa *api.ObjectMeta) (*netproto.SecurityProfile, error) {
	// find the fw profile
	fps, err := s.stateMgr.FindFirewallProfile(ometa.Tenant, ometa.Name)
	if err != nil {
		log.Errorf("Could not find the sg %s|%s", ometa.Tenant, ometa.Name)
		return nil, fmt.Errorf("could not find the sg policy %v", ometa.Name)
	}

	return convertFirewallProfile(fps), nil
}

// ListSecurityProfiles returns a list of fw profiles
func (s *FirewallProfileRPCServer) ListSecurityProfiles(context.Context, *api.ObjectMeta) (*netproto.SecurityProfileList, error) {
	var spList netproto.SecurityProfileList
	// get all fw profiles
	fwps, err := s.stateMgr.ListFirewallProfiles()
	if err != nil {
		return nil, err
	}

	// walk all fw profiles in the list
	for _, fps := range fwps {
		spList.Profiles = append(spList.Profiles, convertFirewallProfile(fps))
	}

	return &spList, nil
}

// WatchSecurityProfiles watches for fw profile changes
func (s *FirewallProfileRPCServer) WatchSecurityProfiles(sel *api.ObjectMeta, stream netproto.SecurityProfileApi_WatchSecurityProfilesServer) error {
	// watch for changes
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)
	s.stateMgr.WatchObjects("FirewallProfile", watchChan)
	defer s.stateMgr.StopWatchObjects("FirewallProfile", watchChan)

	// get a list of all fw profiles
	sps, err := s.ListSecurityProfiles(context.Background(), sel)
	if err != nil {
		log.Errorf("Error getting a list of fw profile. Err: %v", err)
		return err
	}

	ctx := stream.Context()

	// send the objects out as a stream
	for _, sp := range sps.Profiles {
		watchEvt := netproto.SecurityProfileEvent{
			EventType:       api.EventType_CreateEvent,
			SecurityProfile: *sp,
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

			// convert to sg policy object
			fps, err := statemgr.FirewallProfileStateFromObj(evt.Obj)
			if err != nil {
				continue
			}

			// construct the netproto object
			watchEvt := netproto.SecurityProfileEvent{
				EventType:       etype,
				SecurityProfile: *convertFirewallProfile(fps),
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

}

// NewFirewallProfileRPCServer returns a security RPC server
func NewFirewallProfileRPCServer(stateMgr *statemgr.Statemgr) (*FirewallProfileRPCServer, error) {
	return &FirewallProfileRPCServer{stateMgr: stateMgr}, nil
}
