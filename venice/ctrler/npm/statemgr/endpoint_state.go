// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"context"
	//"fmt"
	//"net"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// EndpointState is a wrapper for endpoint object
type EndpointState struct {
	Endpoint   *ctkit.Endpoint                `json:"-"` // embedding endpoint object
	groups     map[string]*SecurityGroupState // list of security groups
	stateMgr   *Statemgr                      // state manager
	moveCtx    context.Context
	moveCancel context.CancelFunc
	moveWg     sync.WaitGroup
}

// endpointKey returns endpoint key
func (eps *EndpointState) endpointKey() string {
	return eps.Endpoint.ObjectMeta.Name
}

// EndpointStateFromObj conerts from memdb object to endpoint state
func EndpointStateFromObj(obj runtime.Object) (*EndpointState, error) {
	switch obj.(type) {
	case *ctkit.Endpoint:
		epobj := obj.(*ctkit.Endpoint)
		switch epobj.HandlerCtx.(type) {
		case *EndpointState:
			eps := epobj.HandlerCtx.(*EndpointState)
			return eps, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

func convertEndpoint(eps *workload.Endpoint) *netproto.Endpoint {
	// build endpoint
	creationTime, _ := types.TimestampProto(time.Now())
	nep := netproto.Endpoint{
		TypeMeta:   eps.TypeMeta,
		ObjectMeta: agentObjectMeta(eps.ObjectMeta),
		Spec: netproto.EndpointSpec{
			NetworkName:   eps.Status.Network,
			IPv4Addresses: []string{eps.Status.IPv4Address},
			IPv6Addresses: []string{eps.Status.IPv6Address},
			MacAddress:    eps.Status.MacAddress,
			UsegVlan:      eps.Status.MicroSegmentVlan,
			NodeUUID:      eps.Spec.NodeUUID,
		},
	}
	nep.CreationTime = api.Timestamp{Timestamp: *creationTime}

	return &nep
}

// AddSecurityGroup adds a security group to an endpoint
func (eps *EndpointState) AddSecurityGroup(sgs *SecurityGroupState) error {
	// lock the endpoint
	eps.Endpoint.Lock()
	defer eps.Endpoint.Unlock()

	// add security group to the list
	eps.Endpoint.Status.SecurityGroups = append(eps.Endpoint.Status.SecurityGroups, sgs.SecurityGroup.Name)
	eps.groups[sgs.SecurityGroup.Name] = sgs

	// save it to api server
	err := eps.Write(false)
	if err != nil {
		log.Errorf("Error writing the endpoint state to api server. Err: %v", err)
		return err
	}

	return eps.stateMgr.mbus.UpdateObjectWithReferences(eps.Endpoint.MakeKey("cluster"),
		convertEndpoint(&eps.Endpoint.Endpoint), references(eps.Endpoint))
}

// DelSecurityGroup removes a security group from an endpoint
func (eps *EndpointState) DelSecurityGroup(sgs *SecurityGroupState) error {
	// lock the endpoint
	eps.Endpoint.Lock()
	defer eps.Endpoint.Unlock()

	// remove the security group from the list
	for i, sgname := range eps.Endpoint.Status.SecurityGroups {
		if sgname == sgs.SecurityGroup.Name {
			if i < (len(eps.Endpoint.Status.SecurityGroups) - 1) {
				eps.Endpoint.Status.SecurityGroups = append(eps.Endpoint.Status.SecurityGroups[:i], eps.Endpoint.Status.SecurityGroups[i+1:]...)
			} else {
				eps.Endpoint.Status.SecurityGroups = eps.Endpoint.Status.SecurityGroups[:i]
			}
		}
	}
	delete(eps.groups, sgs.SecurityGroup.Name)

	return eps.stateMgr.mbus.UpdateObjectWithReferences(eps.Endpoint.MakeKey("cluster"),
		convertEndpoint(&eps.Endpoint.Endpoint), references(eps.Endpoint))
}

// attachSecurityGroups attach all security groups
func (eps *EndpointState) attachSecurityGroups() error {
	// get a list of security groups
	sgs, err := eps.stateMgr.ListSecurityGroups()
	if err != nil {
		log.Errorf("Error getting the list of security groups. Err: %v", err)
		return err
	}

	// walk all sgs and see if endpoint matches the selector
	for _, sg := range sgs {
		if sg.SecurityGroup.Spec.WorkloadSelector != nil && sg.SecurityGroup.Spec.WorkloadSelector.Matches(labels.Set(eps.Endpoint.Status.WorkloadAttributes)) {
			err = sg.AddEndpoint(eps)
			if err != nil {
				log.Errorf("Error adding ep %s to sg %s. Err: %v", eps.Endpoint.Name, sg.SecurityGroup.Name, err)
				return err
			}

			// add sg to endpoint
			eps.Endpoint.Status.SecurityGroups = append(eps.Endpoint.Status.SecurityGroups, sg.SecurityGroup.Name)
			eps.groups[sg.SecurityGroup.Name] = sg
		}
	}

	return nil
}

// Write writes the object to api server
func (eps *EndpointState) Write(update bool) error {
	return eps.Endpoint.Write()
}

// Delete deletes all state associated with the endpoint
func (eps *EndpointState) Delete() error {
	// detach the endpoint from security group
	for _, sg := range eps.groups {
		err := sg.DelEndpoint(eps)
		if err != nil {
			log.Errorf("Error removing endpoint from sg. Err: %v", err)
		}
	}

	return nil
}

// NewEndpointState returns a new endpoint object
func NewEndpointState(epinfo *ctkit.Endpoint, stateMgr *Statemgr) (*EndpointState, error) {
	// build the endpoint state
	eps := EndpointState{
		Endpoint: epinfo,
		groups:   make(map[string]*SecurityGroupState),
		stateMgr: stateMgr,
	}
	epinfo.HandlerCtx = &eps

	// attach security groups
	err := eps.attachSecurityGroups()
	if err != nil {
		log.Errorf("Error attaching security groups to ep %v. Err: %v", eps.Endpoint.Name, err)
		return nil, err
	}

	// save it to api server
	err = eps.Write(false)
	if err != nil {
		log.Errorf("Error writing the endpoint state to api server. Err: %v", err)
		return nil, err
	}

	return &eps, nil
}

//GetEndpointWatchOptions gets options
func (sm *Statemgr) GetEndpointWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec", "Status.Migration"}
	return &opts
}

// OnEndpointCreateReq gets called when agent sends create request
func (sm *Statemgr) OnEndpointCreateReq(nodeID string, objinfo *netproto.Endpoint) error {
	return nil
}

// OnEndpointUpdateReq gets called when agent sends update request
func (sm *Statemgr) OnEndpointUpdateReq(nodeID string, objinfo *netproto.Endpoint) error {
	return nil
}

// OnEndpointDeleteReq gets called when agent sends delete request
func (sm *Statemgr) OnEndpointDeleteReq(nodeID string, objinfo *netproto.Endpoint) error {
	return nil
}

// OnEndpointOperUpdate gets called when agent sends oper update
func (sm *Statemgr) OnEndpointOperUpdate(nodeID string, objinfo *netproto.Endpoint) error {
	// FIXME: handle endpoint status updates from agent
	return nil
}

// OnEndpointOperDelete is called when agent sends oper delete
func (sm *Statemgr) OnEndpointOperDelete(nodeID string, objinfo *netproto.Endpoint) error {
	// FIXME: handle endpoint status updates from agent
	return nil
}

// FindEndpoint finds endpointy by name
func (sm *Statemgr) FindEndpoint(tenant, name string) (*EndpointState, error) {
	// find the object
	obj, err := sm.FindObject("Endpoint", tenant, "default", name)
	if err != nil {
		return nil, err
	}

	return EndpointStateFromObj(obj)
}

// EndpointIsPending finds endpointy by name
func (sm *Statemgr) EndpointIsPending(tenant, name string) (bool, error) {
	// find the object
	return sm.IsPending("Endpoint", tenant, "default", name)
}

// ListEndpoints lists all endpoints
func (sm *Statemgr) ListEndpoints() ([]*EndpointState, error) {
	objs := sm.ListObjects("Endpoint")

	var eps []*EndpointState
	for _, obj := range objs {
		ep, err := EndpointStateFromObj(obj)
		if err != nil {
			return eps, err
		}

		eps = append(eps, ep)
	}

	return eps, nil
}

// OnEndpointCreate creates an endpoint
func (sm *Statemgr) OnEndpointCreate(epinfo *ctkit.Endpoint) error {
	log.Infof("Creating endpoint: %#v", epinfo)

	// find network
	ns, err := sm.FindNetwork(epinfo.Tenant, epinfo.Status.Network)
	if err != nil {
		//Retry again, Create network may be lagging.
		time.Sleep(20 * time.Millisecond)
		ns, err = sm.FindNetwork(epinfo.Tenant, epinfo.Status.Network)
		if err != nil {
			log.Errorf("could not find the network %s for endpoint %+v. Err: %v", epinfo.Status.Network, epinfo.ObjectMeta, err)
			return kvstore.NewKeyNotFoundError(epinfo.Status.Network, 0)
		}
	}

	// create a new endpoint instance
	eps, err := NewEndpointState(epinfo, sm)
	if err != nil {
		log.Errorf("Error creating endpoint state from spec{%+v}, Err: %v", epinfo, err)
		return err
	}

	// save the endpoint in the database
	ns.AddEndpoint(eps)
	sm.mbus.AddObjectWithReferences(epinfo.MakeKey("cluster"), convertEndpoint(&epinfo.Endpoint), references(epinfo))

	return nil
}

// OnEndpointUpdate handles update event
func (sm *Statemgr) OnEndpointUpdate(epinfo *ctkit.Endpoint, nep *workload.Endpoint) error {
	epinfo.ObjectMeta = nep.ObjectMeta
	if nep.Status.Migration == nil {
		return nil
	}

	eps, err := EndpointStateFromObj(epinfo)
	if err != nil {
		log.Errorf("failed to find the endpoint %v. Err : %v", epinfo.ObjectMeta, err)
		return ErrEndpointNotFound
	}

	if nep.Status.Migration.Status == workload.EndpointMigrationStatus_DONE.String() {
		log.Infof("EP move was declared to be successful by vCenter")
		eps.Endpoint.Status = nep.Status

		// Send DONE to the new host, so that DSC can start its terminal synch
		newEP := convertEndpoint(&epinfo.Endpoint)
		newEP.Status.Migration = netproto.MigrationState_DONE.String()
		sm.mbus.AddObjectWithReferences(epinfo.MakeKey("cluster"), newEP, references(epinfo))

		return nil
	}

	// If it's an EP update operation with Spec and Status NodeUUID same, it would mean that the migration was aborted or failed
	if nep.Spec.NodeUUID == nep.Status.NodeUUID && nep.Status.Migration.Status == workload.EndpointMigrationStatus_FAILED.String() {
		log.Infof("Endpoint %v migration was aborted.", nep.Name)
		// Set appropriate state of the endpoint object and call cancel of move
		eps.Endpoint.Spec = nep.Spec
		eps.Endpoint.Status.Migration.Status = nep.Status.Migration.Status

		eps.moveCancel()
		eps.moveWg.Wait()
		eps.moveCancel = nil

		// Reset endpoint migration to reflect the abort success
		eps.Endpoint.Status.Migration = nil
		eps.Endpoint.Write()

		return nil
	}

	// If we reached here, it would mean it is start of new migration. Ensure we stop (or wait?) any past running migration
	if eps.moveCancel != nil {
		log.Infof("EP %v migration is in progress. Stopping it.", nep.Name)
		eps.moveCancel()
		eps.moveWg.Wait()
		eps.moveCancel = nil
	}

	var t time.Duration
	ws, err := sm.FindWorkload(eps.Endpoint.Tenant, getWorkloadNameFromEPName(eps.Endpoint.Name))
	if err == nil {
		t, err = time.ParseDuration(ws.Workload.Spec.MigrationTimeout)
	}

	if err != nil {
		log.Errorf("Failed to parse migration timeout from the workload spec. Setting default timeout of 3m. Err : %v", err)
		t, _ = time.ParseDuration("3m")
	}
	deadline := time.Now().Add(t)
	eps.moveCtx, eps.moveCancel = context.WithDeadline(context.Background(), deadline)
	eps.moveWg.Add(1)
	go sm.moveEndpoint(epinfo, nep)

	return nil
}

// OnEndpointDelete deletes an endpoint
func (sm *Statemgr) OnEndpointDelete(epinfo *ctkit.Endpoint) error {
	log.Infof("Deleting Endpoint: %#v", epinfo)

	// see if we have the endpoint
	eps, err := EndpointStateFromObj(epinfo)
	if err != nil {
		log.Errorf("could not find the endpoint %+v", epinfo.ObjectMeta)
		return ErrEndpointNotFound
	}

	if eps.moveCancel != nil {
		eps.moveCancel()
		eps.moveWg.Wait()
		eps.moveCancel = nil
	}

	// find network
	ns, err := sm.FindNetwork(epinfo.Tenant, eps.Endpoint.Status.Network)
	if err != nil {
		log.Errorf("could not find the network %s for endpoint %+v. Err: %v", epinfo.Status.Network, epinfo.ObjectMeta, err)
	} else {

		// free the IPv4 address
		if eps.Endpoint.Status.IPv4Address != "" {
			ns.Lock()
			err = ns.freeIPv4Addr(eps.Endpoint.Status.IPv4Address)
			ns.Unlock()
			if err != nil {
				log.Errorf("Error freeing the endpoint address. Err: %v", err)
			}

			// write the modified network state to api server
			ns.Lock()
			err = ns.Network.Write()
			ns.Unlock()
			if err != nil {
				log.Errorf("Error writing the network object. Err: %v", err)
			}
		}
		// remove it from the database
		ns.RemoveEndpoint(eps)
	}

	// delete the endpoint
	err = eps.Delete()
	if err != nil {
		log.Errorf("Error deleting the endpoint{%+v}. Err: %v", eps, err)
	}
	sm.mbus.DeleteObjectWithReferences(epinfo.MakeKey("cluster"),
		convertEndpoint(&epinfo.Endpoint), references(epinfo))

	log.Infof("Deleted endpoint: %+v", eps)

	return nil
}

func (sm *Statemgr) moveEndpoint(epinfo *ctkit.Endpoint, nep *workload.Endpoint) {
	log.Infof("Moving Endpoint. %v from DSC %v to DSC %v", nep.Name, nep.Spec.NodeUUID, nep.Status.NodeUUID)
	eps, _ := EndpointStateFromObj(epinfo)
	defer eps.moveWg.Done()
	newEP := netproto.Endpoint{
		TypeMeta:   nep.TypeMeta,
		ObjectMeta: agentObjectMeta(nep.ObjectMeta),
		Spec: netproto.EndpointSpec{
			NetworkName: nep.Status.Network,
			MacAddress:  nep.Status.MacAddress,
			UsegVlan:    nep.Spec.MicroSegmentVlan,
			NodeUUID:    nep.Spec.NodeUUID,

			// For netproto, the homing host address is the host where the EP currently is.
			// We send the source homing host IP as part of the spec
			HomingHostAddr: nep.Status.HomingHostAddr,
			Migration:      netproto.MigrationState_START.String(),
		},
		Status: netproto.EndpointStatus{
			NodeUUID: nep.Status.NodeUUID,
		},
	}

	oldEP := netproto.Endpoint{
		TypeMeta:   nep.TypeMeta,
		ObjectMeta: agentObjectMeta(nep.ObjectMeta),
		Spec: netproto.EndpointSpec{
			NetworkName: nep.Status.Network,
			MacAddress:  nep.Status.MacAddress,
			UsegVlan:    nep.Status.MicroSegmentVlan,
			NodeUUID:    nep.Status.NodeUUID,
		},
		Status: netproto.EndpointStatus{
			NodeUUID: nep.Status.NodeUUID,
		},
	}

	sm.mbus.AddObjectWithReferences(epinfo.MakeKey("cluster"), &newEP, references(epinfo))
	ticker := time.NewTicker(1 * time.Second)

	for {
		select {
		case <-eps.moveCtx.Done():
			eps, err := EndpointStateFromObj(epinfo)
			if err != nil {
				log.Errorf("Failed to get endpoint state. Err : %v", err)
				return
			}

			// If the migration status is not set to DONE, it would mean the migration has failed or aborted or timedout
			if eps.Endpoint.Status.Migration.Status != workload.EndpointMigrationStatus_DONE.String() {
				log.Errorf("Error while moving Endpoint %v. Expected done state, got %v.", eps.Endpoint.Name, eps.Endpoint.Status.Migration.Status)
				newEP.Spec.Migration = netproto.MigrationState_ABORT.String()
				newEP.Spec.HomingHostAddr = ""

				// There can only be one object reference in the memdb for a particular object name, kind
				// To use nimbus channel, update the existingObj in memdb to point to the desired object
				sm.mbus.UpdateObjectWithReferences(epinfo.MakeKey("cluster"), &newEP, references(epinfo))

				// Send delete to the appropriate host
				sm.mbus.DeleteObjectWithReferences(epinfo.MakeKey("cluster"), &newEP, references(epinfo))

				// Clean up all the migration state in the netproto object
				oldEP.Spec.Migration = netproto.MigrationState_NONE.String()
				oldEP.Spec.HomingHostAddr = ""

				// Update the in-memory reference for a particular object name, kind
				sm.mbus.AddObjectWithReferences(epinfo.MakeKey("cluster"), &oldEP, references(epinfo))
				if eps.Endpoint.Status.Migration.Status != workload.EndpointMigrationStatus_FAILED.String() && eps.Endpoint.Status.Migration.Status != workload.EndpointMigrationStatus_ABORTED.String() {
					ws, err := sm.FindWorkload(eps.Endpoint.Tenant, getWorkloadNameFromEPName(eps.Endpoint.Name))
					if err != nil {
						log.Errorf("Failed to get workload for EP %v from state manager. Err : %v", eps.Endpoint.Name, err)
					} else {
						log.Errorf("Endpoint %v move timed out.", eps.Endpoint.Name)
						ws.Workload.Status.MigrationStatus.Status = workload.WorkloadMigrationStatus_TIMED_OUT.String()
						ws.Workload.Status.MigrationStatus.CompletedAt = &api.Timestamp{}
						ws.Workload.Status.MigrationStatus.CompletedAt.SetTime(time.Now())
						ws.Workload.Write()
					}
				}
			} else {
				log.Infof("Move was successful. Sending delete EP to old DSC. %v", oldEP.Spec.NodeUUID)
				// We will be here if the migration was successful from vcenter side. Send a delete for the old location
				oldEP.Spec.Migration = netproto.MigrationState_NONE.String()
				oldEP.Spec.HomingHostAddr = ""
				sm.mbus.UpdateObjectWithReferences(epinfo.MakeKey("cluster"), &oldEP, references(epinfo))
				sm.mbus.DeleteObjectWithReferences(epinfo.MakeKey("cluster"), &oldEP, references(epinfo))
				newEP.Spec.Migration = netproto.MigrationState_NONE.String()
				newEP.Spec.HomingHostAddr = ""
				sm.mbus.AddObjectWithReferences(epinfo.MakeKey("cluster"), &newEP, references(epinfo))
			}

			// Reset to None after deleting any migration state
			eps.Endpoint.Status.Migration = nil
			eps.Endpoint.Write()
			return
		case <-ticker.C:
			// TODO add code to query netagent for the endpoint move status
			log.Infof("moving EP")
		}
	}
}
