// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

// DSCProfileState is a wrapper for dscProfile object
type DSCProfileState struct {
	DSCProfile   *ctkit.DSCProfile   `json:"-"` // dscProfile object
	stateMgr     *Statemgr           // pointer to state manager
	NodeVersions map[string]string   // Map fpr node -> version
	DscList      map[string]struct{} // set for list of dsc
}

// DSCProfileStateFromObj converts from memdb object to dscProfile state
func DSCProfileStateFromObj(obj runtime.Object) (*DSCProfileState, error) {
	switch obj.(type) {
	case *ctkit.DSCProfile:
		nsobj := obj.(*ctkit.DSCProfile)
		switch nsobj.HandlerCtx.(type) {
		case *DSCProfileState:
			fps := nsobj.HandlerCtx.(*DSCProfileState)
			return fps, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

//GetDSCProfileWatchOptions gets options
func (sm *Statemgr) GetDSCProfileWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{}
	return &opts
}

// NewDSCProfileState creates new dscProfile state object
func NewDSCProfileState(dscProfile *ctkit.DSCProfile, stateMgr *Statemgr) (*DSCProfileState, error) {
	fps := DSCProfileState{
		DSCProfile:   dscProfile,
		stateMgr:     stateMgr,
		NodeVersions: make(map[string]string),
		DscList:      make(map[string]struct{}),
	}
	dscProfile.HandlerCtx = &fps

	return &fps, nil
}

// convertDSCProfile converts dsc profile state to security profile
func convertDSCProfile(fps *DSCProfileState) *netproto.Profile {
	// build sg message
	creationTime, _ := types.TimestampProto(time.Now())
	fwp := netproto.Profile{
		TypeMeta:   api.TypeMeta{Kind: "Profile"},
		ObjectMeta: agentObjectMeta(fps.DSCProfile.ObjectMeta),
		Spec: netproto.ProfileSpec{
			PolicyMode: fps.DSCProfile.Spec.FlowPolicyMode,
			FwdMode:    fps.DSCProfile.Spec.FwdMode,
		},
	}
	fwp.CreationTime = api.Timestamp{Timestamp: *creationTime}

	return &fwp
}

// OnDSCProfileCreateReq gets called when agent sends create request
func (sm *Statemgr) OnDSCProfileCreateReq(nodeID string, objinfo *netproto.Profile) error {
	return nil
}

// OnDSCProfileUpdateReq gets called when agent sends update request
func (sm *Statemgr) OnDSCProfileUpdateReq(nodeID string, objinfo *netproto.Profile) error {
	return nil
}

// OnDSCProfileDeleteReq gets called when agent sends delete request
func (sm *Statemgr) OnDSCProfileDeleteReq(nodeID string, objinfo *netproto.Profile) error {
	return nil
}

// OnDSCProfileOperUpdate gets called when policy updates arrive from agents
func (sm *Statemgr) OnDSCProfileOperUpdate(nodeID string, objinfo *netproto.Profile) error {
	return nil
}

// OnDSCProfileOperDelete gets called when policy delete arrives from agent
func (sm *Statemgr) OnDSCProfileOperDelete(nodeID string, objinfo *netproto.Profile) error {
	return nil
}

// OnDSCProfileCreate handles dscProfile creation
func (sm *Statemgr) OnDSCProfileCreate(dscProfile *ctkit.DSCProfile) error {
	log.Infof("Creating dscProfile: %+v", dscProfile)

	// create new dscProfile object
	fps, err := NewDSCProfileState(dscProfile, sm)
	if err != nil {
		log.Errorf("Error creating dscProfile %+v. Err: %v", dscProfile, err)
		return err
	}

	// store it in local DB
	sm.mbus.AddObjectWithReferences(dscProfile.MakeKey("profile"), convertDSCProfile(fps), references(dscProfile))

	return nil
}

// OnDSCProfileUpdate handles update event
func (sm *Statemgr) OnDSCProfileUpdate(dscProfile *ctkit.DSCProfile, nfwp *cluster.DSCProfile) error {
	// see if anything changed
	log.Infof("Received update %v\n", nfwp)
	_, ok := ref.ObjDiff(dscProfile.Spec, nfwp.Spec)
	if (nfwp.GenerationID == dscProfile.GenerationID) && !ok {
		log.Infof("No update received")
		dscProfile.ObjectMeta = nfwp.ObjectMeta
		return nil
	}

	fps, err := sm.FindDSCProfile(dscProfile.Tenant, dscProfile.Name)
	if err != nil {
		log.Errorf("Could not find the dsc profile %+v. Err: %v", dscProfile.ObjectMeta, err)
		return err
	}

	// update the object in mbus
	fps.DSCProfile.Spec = nfwp.Spec
	fps.DSCProfile.ObjectMeta = nfwp.ObjectMeta
	fps.DSCProfile.Status = cluster.DSCProfileStatus{}

	log.Infof("Sending update received")
	// TODO Lakshmi : Might have to send the list of DSCs based on the api provided by Sudhi
	sm.mbus.UpdateObjectWithReferences(dscProfile.MakeKey("cluster"), convertDSCProfile(fps), references(dscProfile))
	log.Infof("Updated dscProfile: %+v", dscProfile)

	return nil
}

// OnDSCProfileDelete handles dscProfile deletion
func (sm *Statemgr) OnDSCProfileDelete(dscProfile *ctkit.DSCProfile) error {
	// see if we have the dscProfile
	fps, err := sm.FindDSCProfile("", dscProfile.Name)
	if err != nil {
		log.Errorf("Could not find the dscProfile %v. Err: %v", dscProfile, err)
		return err
	}

	log.Infof("Deleting dscProfile: %+v", dscProfile)

	// delete the object
	return sm.mbus.DeleteObjectWithReferences(dscProfile.MakeKey("profile"),
		convertDSCProfile(fps), references(dscProfile))
}

// FindDSCProfile finds a dscProfile
func (sm *Statemgr) FindDSCProfile(tenant, name string) (*DSCProfileState, error) {
	// find the object
	obj, err := sm.FindObject("DSCProfile", "", "", name)
	if err != nil {
		log.Infof("Unable to find the profile")
		return nil, err
	}
	log.Infof("Found the profile")
	return DSCProfileStateFromObj(obj)
}

// GetKey returns the key of DSCProfile
func (fps *DSCProfileState) GetKey() string {
	return fps.DSCProfile.GetKey()
}

// ListDSCProfiles lists all apps
func (sm *Statemgr) ListDSCProfiles() ([]*DSCProfileState, error) {
	objs := sm.ListObjects("DSCProfile")

	var fwps []*DSCProfileState
	for _, obj := range objs {
		fwp, err := DSCProfileStateFromObj(obj)
		if err != nil {
			return fwps, err
		}

		fwps = append(fwps, fwp)
	}

	return fwps, nil
}
