// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// FirewallProfileState is a wrapper for fwProfile object
type FirewallProfileState struct {
	FirewallProfile *ctkit.FirewallProfile `json:"-"` // fwProfile object
	stateMgr        *Statemgr              // pointer to state manager
}

// FirewallProfileStateFromObj conerts from memdb object to fwProfile state
func FirewallProfileStateFromObj(obj runtime.Object) (*FirewallProfileState, error) {
	switch obj.(type) {
	case *ctkit.FirewallProfile:
		nsobj := obj.(*ctkit.FirewallProfile)
		switch nsobj.HandlerCtx.(type) {
		case *FirewallProfileState:
			fps := nsobj.HandlerCtx.(*FirewallProfileState)
			return fps, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewFirewallProfileState creates new fwProfile state object
func NewFirewallProfileState(fwProfile *ctkit.FirewallProfile, stateMgr *Statemgr) (*FirewallProfileState, error) {
	fps := FirewallProfileState{
		FirewallProfile: fwProfile,
		stateMgr:        stateMgr,
	}
	fwProfile.HandlerCtx = &fps

	return &fps, nil
}

// convertFirewallProfile converts fw profile state to security profile
func convertFirewallProfile(fps *FirewallProfileState) *netproto.SecurityProfile {
	// build sg message
	fwp := netproto.SecurityProfile{
		TypeMeta:   api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: fps.FirewallProfile.ObjectMeta,
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        fps.FirewallProfile.Spec.SessionIdleTimeout,
				TCP:                fps.FirewallProfile.Spec.TcpTimeout,
				TCPDrop:            fps.FirewallProfile.Spec.TCPDropTimeout,
				TCPConnectionSetup: fps.FirewallProfile.Spec.TCPConnectionSetupTimeout,
				TCPClose:           fps.FirewallProfile.Spec.TCPCloseTimeout,
				TCPHalfClose:       fps.FirewallProfile.Spec.TCPHalfClosedTimeout,
				Drop:               fps.FirewallProfile.Spec.DropTimeout,
				UDP:                fps.FirewallProfile.Spec.UdpTimeout,
				UDPDrop:            fps.FirewallProfile.FirewallProfile.Spec.UDPDropTimeout,
				ICMP:               fps.FirewallProfile.Spec.IcmpTimeout,
				ICMPDrop:           fps.FirewallProfile.Spec.ICMPDropTimeout,
			},
		},
	}

	return &fwp
}

// OnFirewallProfileCreate handles fwProfile creation
func (sm *Statemgr) OnFirewallProfileCreate(fwProfile *ctkit.FirewallProfile) error {
	log.Infof("Creating fwProfile: %+v", fwProfile)

	// create new fwProfile object
	fps, err := NewFirewallProfileState(fwProfile, sm)
	if err != nil {
		log.Errorf("Error creating fwProfile %+v. Err: %v", fwProfile, err)
		return err
	}

	// store it in local DB
	sm.mbus.AddObject(convertFirewallProfile(fps))

	return nil
}

// OnFirewallProfileUpdate handles update event
func (sm *Statemgr) OnFirewallProfileUpdate(fwProfile *ctkit.FirewallProfile) error {
	fps, err := sm.FindFirewallProfile(fwProfile.Tenant, fwProfile.Name)
	if err != nil {
		log.Errorf("Could not find the firewall profile %+v. Err: %v", fwProfile.ObjectMeta, err)
		return err
	}

	// update the object in mbus
	fps.FirewallProfile = fwProfile
	sm.mbus.UpdateObject(convertFirewallProfile(fps))
	log.Infof("Updated fwProfile: %+v", fwProfile)

	return nil
}

// OnFirewallProfileDelete handles fwProfile deletion
func (sm *Statemgr) OnFirewallProfileDelete(fwProfile *ctkit.FirewallProfile) error {
	// see if we have the fwProfile
	fps, err := sm.FindFirewallProfile(fwProfile.Tenant, fwProfile.Name)
	if err != nil {
		log.Errorf("Could not find the fwProfile %v. Err: %v", fwProfile, err)
		return err
	}

	log.Infof("Deleting fwProfile: %+v", fwProfile)

	// delete the object
	return sm.mbus.DeleteObject(convertFirewallProfile(fps))
}

// FindFirewallProfile finds a fwProfile
func (sm *Statemgr) FindFirewallProfile(tenant, name string) (*FirewallProfileState, error) {
	// find the object
	obj, err := sm.FindObject("FirewallProfile", tenant, "default", name)
	if err != nil {
		return nil, err
	}

	return FirewallProfileStateFromObj(obj)
}

// ListFirewallProfiles lists all apps
func (sm *Statemgr) ListFirewallProfiles() ([]*FirewallProfileState, error) {
	objs := sm.ListObjects("FirewallProfile")

	var fwps []*FirewallProfileState
	for _, obj := range objs {
		fwp, err := FirewallProfileStateFromObj(obj)
		if err != nil {
			return fwps, err
		}

		fwps = append(fwps, fwp)
	}

	return fwps, nil
}
