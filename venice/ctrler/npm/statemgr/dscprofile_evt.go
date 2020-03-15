// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

type dscProfileVersion struct {
	profileName string
	agentGenID  string
}

// DSCProfileState is a wrapper for dscProfile object
type DSCProfileState struct {
	DSCProfile   *ctkit.DSCProfile            `json:"-"` // dscProfile object
	stateMgr     *Statemgr                    // pointer to state manager
	NodeVersions map[string]string            // Map  of node -> version
	DscList      map[string]dscProfileVersion // set for list of dsc
	PushObj      memdb.PushObjectHandle
}

// initNodeVersions initializes node versions for the policy
func (dps *DSCProfileState) initNodeVersions() error {
	dscs, _ := dps.stateMgr.ListDistributedServiceCards()

	// walk all smart nics
	for _, dsc := range dscs {
		if dps.stateMgr.isDscAdmitted(&dsc.DistributedServiceCard.DistributedServiceCard) {
			if _, ok := dps.NodeVersions[dsc.DistributedServiceCard.Name]; !ok {
				dps.NodeVersions[dsc.DistributedServiceCard.Name] = ""
			}
		}
	}

	return nil
}

// DSCProfileStateFromObj converts from memdb object to dscProfile state
func DSCProfileStateFromObj(obj runtime.Object) (*DSCProfileState, error) {
	switch obj.(type) {
	case *ctkit.DSCProfile:
		nsobj := obj.(*ctkit.DSCProfile)
		switch nsobj.HandlerCtx.(type) {
		case *DSCProfileState:
			dps := nsobj.HandlerCtx.(*DSCProfileState)
			return dps, nil
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
	dps := DSCProfileState{
		DSCProfile:   dscProfile,
		stateMgr:     stateMgr,
		NodeVersions: make(map[string]string),
		DscList:      make(map[string]dscProfileVersion),
	}
	dscProfile.HandlerCtx = &dps

	return &dps, nil
}

// convertDSCProfile converts dsc profile state to security profile
func convertDSCProfile(dps *DSCProfileState) *netproto.Profile {
	// build sg message
	creationTime, _ := types.TimestampProto(time.Now())
	fwp := netproto.Profile{
		TypeMeta:   api.TypeMeta{Kind: "Profile"},
		ObjectMeta: agentObjectMeta(dps.DSCProfile.ObjectMeta),
		Spec: netproto.ProfileSpec{
			PolicyMode: dps.DSCProfile.Spec.FlowPolicyMode,
			FwdMode:    dps.DSCProfile.Spec.FwdMode,
		},
	}
	fwp.CreationTime = api.Timestamp{Timestamp: *creationTime}
	fwp.ObjectMeta.Tenant = "default"
	fwp.ObjectMeta.Namespace = "default"
	log.Infof("UUID is %v", fwp.ObjectMeta.UUID)

	return &fwp
}

// OnDSCProfileCreate handles dscProfile creation
func (sm *Statemgr) OnDSCProfileCreate(dscProfile *ctkit.DSCProfile) error {
	log.Infof("Creating dscProfile: %+v", dscProfile)

	// create new dscProfile object
	dps, err := NewDSCProfileState(dscProfile, sm)
	if err != nil {
		log.Errorf("Error creating dscProfile %+v. Err: %v", dscProfile, err)
		return err
	}
	// in case of error, write status back
	defer func() {
		if err != nil {
			dps.DSCProfile.Status.PropagationStatus.Status = fmt.Sprintf("DSCProfile processing error")
		}
	}()

	// store it in local DB
	pushObj, err := sm.mbus.AddPushObject(dscProfile.MakeKey("cluster"), convertDSCProfile(dps), references(dscProfile), nil)
	dps.PushObj = pushObj
	dps.initNodeVersions()
	sm.PeriodicUpdaterPush(dps)

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

	dps, err := sm.FindDSCProfile(dscProfile.Tenant, dscProfile.Name)
	if err != nil {
		log.Errorf("Could not find the dsc profile %+v. Err: %v", dscProfile.ObjectMeta, err)
		return err
	}

	// update the object in mbus
	dps.DSCProfile.Spec = nfwp.Spec
	dps.DSCProfile.ObjectMeta = nfwp.ObjectMeta
	dps.DSCProfile.Status = cluster.DSCProfileStatus{}

	log.Infof("Sending update received")

	dps.PushObj.UpdateObjectWithReferences(dscProfile.MakeKey("cluster"), convertDSCProfile(dps), references(dscProfile))

	log.Infof("Updated dscProfile: %+v", dscProfile)

	dscs := dps.DscList
	for dsc := range dscs {
		dps.DscList[dsc] = dscProfileVersion{dscProfile.Name, nfwp.GenerationID}
	}
	return nil
}

// OnDSCProfileDelete handles dscProfile deletion
func (sm *Statemgr) OnDSCProfileDelete(dscProfile *ctkit.DSCProfile) error {
	// see if we have the dscProfile
	dps, err := sm.FindDSCProfile("", dscProfile.Name)
	if err != nil {
		log.Errorf("Could not find the dscProfile %v. Err: %v", dscProfile, err)
		return err
	}

	log.Infof("Deleting dscProfile: %+v %v", dscProfile, dps)

	return dps.PushObj.DeleteObjectWithReferences(dscProfile.MakeKey("cluster"), convertDSCProfile(dps), references(dscProfile))
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
func (dps *DSCProfileState) GetKey() string {
	return dps.DSCProfile.GetKey()
}

func (dps *DSCProfileState) propagatationStatusDifferent(
	current *cluster.PropagationStatus,
	other *cluster.PropagationStatus) bool {

	sliceEqual := func(X, Y []string) bool {
		m := make(map[string]int)

		for _, y := range Y {
			m[y]++
		}

		for _, x := range X {
			if m[x] > 0 {
				m[x]--
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

func (dps *DSCProfileState) updatePropagationStatus(genID string,
	current *cluster.PropagationStatus, nodeVersions map[string]string) *cluster.PropagationStatus {

	objs := dps.stateMgr.ListObjects("DistributedServiceCard")
	newProp := &cluster.PropagationStatus{GenerationID: genID}
	pendingNodes := []string{}
	for _, obj := range objs {
		snic, err := DistributedServiceCardStateFromObj(obj)
		if err != nil || !dps.stateMgr.isDscAdmitted(&snic.DistributedServiceCard.DistributedServiceCard) {
			continue
		}

		if ver, ok := nodeVersions[snic.DistributedServiceCard.Name]; ok && ver == genID {
			newProp.Updated++
		} else {
			pendingNodes = append(pendingNodes, snic.DistributedServiceCard.Name)
			newProp.Pending++
			if current.MinVersion == "" || versionToInt(ver) < versionToInt(newProp.MinVersion) {
				newProp.MinVersion = ver
			}
		}
	}
	// set status
	if newProp.Pending == 0 {
		newProp.Status = fmt.Sprintf("Propagation Complete")
		newProp.PendingNaples = []string{}
	} else {
		newProp.Status = fmt.Sprintf("Propagation pending on: %s", strings.Join(pendingNodes, ", "))
		newProp.PendingNaples = pendingNodes
	}
	return newProp
}

// Write write the object to api server
func (dps *DSCProfileState) Write() error {
	var err error
	log.Infof("push periodically status")
	dps.DSCProfile.Lock()
	defer dps.DSCProfile.Unlock()
	prop := &dps.DSCProfile.Status.PropagationStatus
	newProp := dps.updatePropagationStatus(dps.DSCProfile.GenerationID, prop, dps.NodeVersions)
	if dps.propagatationStatusDifferent(prop, newProp) {
		dps.DSCProfile.Status.PropagationStatus = *newProp
		err = dps.DSCProfile.Write()
		if err != nil {
			dps.DSCProfile.Status.PropagationStatus = *prop
		}
	}
	return err
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

// OnProfileCreateReq gets called when agent sends create request
func (sm *Statemgr) OnProfileCreateReq(nodeID string, objinfo *netproto.Profile) error {
	log.Infof("received profile create req")
	return nil
}

// OnProfileUpdateReq gets called when agent sends update request
func (sm *Statemgr) OnProfileUpdateReq(nodeID string, objinfo *netproto.Profile) error {
	log.Infof("recieved profile update req")
	return nil
}

// OnProfileDeleteReq gets called when agent sends delete request
func (sm *Statemgr) OnProfileDeleteReq(nodeID string, objinfo *netproto.Profile) error {
	log.Infof("received profile delete req")
	return nil
}

// OnProfileOperUpdate gets called when policy updates arrive from agents
func (sm *Statemgr) OnProfileOperUpdate(nodeID string, objinfo *netproto.Profile) error {
	sm.UpdateDSCProfileStatus(nodeID, objinfo.ObjectMeta.Tenant, objinfo.ObjectMeta.Name, objinfo.ObjectMeta.GenerationID)
	return nil
}

// OnProfileOperDelete gets called when policy delete arrives from agent
func (sm *Statemgr) OnProfileOperDelete(nodeID string, objinfo *netproto.Profile) error {
	return nil
}

// UpdateDSCProfileStatus updates the profile status
func (sm *Statemgr) UpdateDSCProfileStatus(nodeuuid, tenant, name, generationID string) {
	log.Infof("Update the profile status for profile %s tenant %s", name, tenant)
	dscProfile, err := sm.FindDSCProfile(tenant, name)
	if err != nil {
		return
	}
	// find smartnic object
	snic, err := sm.FindDistributedServiceCard(tenant, nodeuuid)
	if err == nil {
		if !sm.isDscHealthy(&snic.DistributedServiceCard.DistributedServiceCard) {
			log.Infof("DSC %v unhealthy but ignoring to update dscprofile status with genId %v", nodeuuid, generationID)
		}
	}

	// lock profile for concurrent modifications
	dscProfile.DSCProfile.Lock()
	defer dscProfile.DSCProfile.Unlock()
	expVersion := dscProfile.DscList[snic.DistributedServiceCard.Name]

	if expVersion.profileName == name && expVersion.agentGenID == generationID {
		// update node version
		dscProfile.NodeVersions[nodeuuid] = generationID
		// ToDo: Writing status from NPM conflicts with CMD in integ tests.
		//  We have to fix this eventually
		//snic.NodeVersion = cluster.DSCProfileVersion{ProfileName: name, GenerationID: generationID}
	}
	sm.PeriodicUpdaterPush(dscProfile)

}
