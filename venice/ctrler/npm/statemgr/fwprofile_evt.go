// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

// FirewallProfileState is a wrapper for fwProfile object
type FirewallProfileState struct {
	smObjectTracker
	FirewallProfile *ctkit.FirewallProfile `json:"-"` // fwProfile object
	stateMgr        *Statemgr              // pointer to state manager
	markedForDelete bool
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

//GetFirewallProfileWatchOptions gets options
func (sm *Statemgr) GetFirewallProfileWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{}
	return &opts
}

// NewFirewallProfileState creates new fwProfile state object
func NewFirewallProfileState(fwProfile *ctkit.FirewallProfile, stateMgr *Statemgr) (*FirewallProfileState, error) {
	fps := FirewallProfileState{
		FirewallProfile: fwProfile,
		stateMgr:        stateMgr,
	}
	fwProfile.HandlerCtx = &fps
	fps.smObjectTracker.init(&fps)

	return &fps, nil
}

// convertFirewallProfile converts fw profile state to security profile
func convertFirewallProfile(fps *FirewallProfileState) *netproto.SecurityProfile {
	// build sg message
	creationTime, _ := types.TimestampProto(time.Now())
	fwp := netproto.SecurityProfile{
		TypeMeta:   api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: agentObjectMeta(fps.FirewallProfile.ObjectMeta),
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
				UDPDrop:            fps.FirewallProfile.Spec.UDPDropTimeout,
				ICMP:               fps.FirewallProfile.Spec.IcmpTimeout,
				ICMPDrop:           fps.FirewallProfile.Spec.ICMPDropTimeout,
			},
			RateLimits: &netproto.RateLimits{
				TcpHalfOpenSessionLimit: fps.FirewallProfile.Spec.TcpHalfOpenSessionLimit,
				UdpActiveSessionLimit:   fps.FirewallProfile.Spec.UdpActiveSessionLimit,
				IcmpActiveSessionLimit:  fps.FirewallProfile.Spec.IcmpActiveSessionLimit,
				OtherActiveSessionLimit: fps.FirewallProfile.Spec.OtherActiveSessionLimit,
			},
			DetectApp: fps.FirewallProfile.Spec.DetectApp,
		},
	}
	fwp.CreationTime = api.Timestamp{Timestamp: *creationTime}

	return &fwp
}

func (sm *Statemgr) propgatationStatusDifferent(
	current *security.PropagationStatus,
	other *security.PropagationStatus) bool {

	sliceEqual := func(X, Y []string) bool {
		m := make(map[string]int)

		for _, y := range Y {
			m[y]++
		}

		for _, x := range X {
			if m[x] > 0 {
				m[x]--
				if m[x] == 0 {
					delete(m, x)
				}
				continue
			}
			//not present or execess
			return false
		}

		return len(m) == 0
	}

	if other.GenerationID != current.GenerationID || other.Updated != current.Updated || other.Pending != current.Pending || other.Status != current.Status ||
		other.MinVersion != current.MinVersion || !sliceEqual(current.PendingNaples, other.PendingNaples) {
		return true
	}
	return false
}

//TrackedDSCs tracked DSCs
func (fps *FirewallProfileState) TrackedDSCs() []string {

	dscs, _ := fps.stateMgr.ListDistributedServiceCards()

	trackedDSCs := []string{}
	for _, dsc := range dscs {
		if fps.stateMgr.isDscEnforcednMode(&dsc.DistributedServiceCard.DistributedServiceCard) {
			trackedDSCs = append(trackedDSCs, dsc.DistributedServiceCard.DistributedServiceCard.Name)
		}
	}

	return trackedDSCs
}

func (fps *FirewallProfileState) isMarkedForDelete() bool {
	return fps.markedForDelete
}

// processDSCUpdate sgpolicy update handles for DSC
func (fps *FirewallProfileState) processDSCUpdate(dsc *cluster.DistributedServiceCard) error {

	fps.FirewallProfile.Lock()
	defer fps.FirewallProfile.Unlock()

	if fps.stateMgr.isDscEnforcednMode(dsc) {
		fps.smObjectTracker.startDSCTracking(dsc.Name)
	}

	return nil
}

// processDSCUpdate sgpolicy update handles for DSC
func (fps *FirewallProfileState) processDSCDelete(dsc *cluster.DistributedServiceCard) error {

	fps.FirewallProfile.Lock()
	defer fps.FirewallProfile.Unlock()

	fps.smObjectTracker.stopDSCTracking(dsc.Name)

	return nil
}

// Write writes the object to api server
func (fps *FirewallProfileState) Write() error {
	fps.FirewallProfile.Lock()
	defer fps.FirewallProfile.Unlock()

	propStatus := fps.getPropStatus()
	newProp := &security.PropagationStatus{
		GenerationID:  propStatus.generationID,
		MinVersion:    propStatus.minVersion,
		Pending:       propStatus.pending,
		PendingNaples: propStatus.pendingDSCs,
		Updated:       propStatus.updated,
		Status:        propStatus.status,
	}
	prop := &fps.FirewallProfile.Status.PropagationStatus

	//Do write only if changed
	if fps.stateMgr.propgatationStatusDifferent(prop, newProp) {
		fps.FirewallProfile.Status.PropagationStatus = *newProp
		return fps.FirewallProfile.Write()
	}

	return nil

}

// OnSecurityProfileCreateReq gets called when agent sends create request
func (sm *Statemgr) OnSecurityProfileCreateReq(nodeID string, objinfo *netproto.SecurityProfile) error {
	return nil
}

// OnSecurityProfileUpdateReq gets called when agent sends update request
func (sm *Statemgr) OnSecurityProfileUpdateReq(nodeID string, objinfo *netproto.SecurityProfile) error {
	return nil
}

// OnSecurityProfileDeleteReq gets called when agent sends delete request
func (sm *Statemgr) OnSecurityProfileDeleteReq(nodeID string, objinfo *netproto.SecurityProfile) error {
	return nil
}

// OnSecurityProfileOperUpdate gets called when policy updates arrive from agents
func (sm *Statemgr) OnSecurityProfileOperUpdate(nodeID string, objinfo *netproto.SecurityProfile) error {
	sm.UpdateFirewallProfileStatus(nodeID, objinfo.ObjectMeta.Tenant, objinfo.ObjectMeta.Name, objinfo.ObjectMeta.GenerationID)
	return nil
}

// OnSecurityProfileOperDelete gets called when policy delete arrives from agent
func (sm *Statemgr) OnSecurityProfileOperDelete(nodeID string, objinfo *netproto.SecurityProfile) error {
	return nil
}

//GetDBObject get db object
func (fps *FirewallProfileState) GetDBObject() memdb.Object {
	return convertFirewallProfile(fps)
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

	//fps.iniNo

	sm.PeriodicUpdaterPush(fps)

	// store it in local DB
	err = sm.AddObjectToMbus(fwProfile.MakeKey("security"), fps, references(fwProfile))
	if err != nil {
		log.Errorf("Error storing the sg policy in memdb. Err: %v", err)
		return err
	}

	return nil
}

// OnFirewallProfileUpdate handles update event
func (sm *Statemgr) OnFirewallProfileUpdate(fwProfile *ctkit.FirewallProfile, nfwp *security.FirewallProfile) error {
	// see if anything changed
	log.Infof("Received update %v\n", nfwp)
	_, ok := ref.ObjDiff(fwProfile.Spec, nfwp.Spec)
	if (nfwp.GenerationID == fwProfile.GenerationID) && !ok {
		log.Infof("No update received")
		fwProfile.ObjectMeta = nfwp.ObjectMeta
		return nil
	}

	fps, err := sm.FindFirewallProfile(fwProfile.Tenant, fwProfile.Name)
	if err != nil {
		log.Errorf("Could not find the firewall profile %+v. Err: %v", fwProfile.ObjectMeta, err)
		return err
	}

	// update the object in mbus
	fps.FirewallProfile.Spec = nfwp.Spec
	fps.FirewallProfile.ObjectMeta = nfwp.ObjectMeta
	fps.FirewallProfile.Status = security.FirewallProfileStatus{}

	log.Infof("Sending udpate received")
	err = sm.UpdateObjectToMbus(fwProfile.MakeKey("security"), fps, references(fwProfile))
	if err != nil {
		log.Errorf("Error storing the sg policy in memdb. Err: %v", err)
		return err
	}
	log.Infof("Updated fwProfile: %+v", fwProfile)

	sm.PeriodicUpdaterPush(fps)

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

	//Mark fo delete will make sure DSC updates are not received
	// If we try to unregister, we will be waiting for sm.lock to update DSC notif list
	// But, sm.lock could be held by DSC update thread, which in turn might be waiting to
	// update current object.
	fps.markedForDelete = true
	// delete the object
	return sm.DeleteObjectToMbus(fwProfile.MakeKey("security"),
		fps, references(fwProfile))
}

// OnFirewallProfileReconnect is called when ctkit reconnects to apiserver
func (sm *Statemgr) OnFirewallProfileReconnect() {
	return
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

// GetKey returns the key of FirewallProfile
func (fps *FirewallProfileState) GetKey() string {
	return fps.FirewallProfile.GetKey()
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

//UpdateFirewallProfileStatus Updated the status of firewallProfile
func (sm *Statemgr) UpdateFirewallProfileStatus(nodeuuid, tenant, name, generationID string) {
	fps, err := sm.FindFirewallProfile(tenant, name)
	if err != nil {
		log.Errorf("Error finding FirwallProfile %s in tenant : %s. Err: %v", name, tenant, err)
		return
	}

	fps.updateNodeVersion(nodeuuid, generationID)
}
