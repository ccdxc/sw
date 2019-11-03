// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

// SecurityGroupState security group state
type SecurityGroupState struct {
	SecurityGroup *ctkit.SecurityGroup      `json:"-"` // security group object
	policies      map[string]*SgpolicyState // list of policies attached to this security group
	endpoints     map[string]*EndpointState // list of all matching endpoints in this security group
	stateMgr      *Statemgr                 // pointer to network manager
}

// Delete cleans up all state associated with the sg
func (sg *SecurityGroupState) Delete() error {
	// check if sg still has policies
	if len(sg.policies) != 0 {
		log.Errorf("Can not delete the sg, still has policies attached {%+v}", sg)
		return fmt.Errorf("Security group still has policies")
	}

	return nil
}

// attachEndpoints finds all the matching endpoints based on selector
func (sg *SecurityGroupState) attachEndpoints() error {
	// get a list of all endpoints
	eps, err := sg.stateMgr.ListEndpoints()
	if err != nil {
		log.Errorf("Error getting endpoint list. Err: %v", err)
		return err
	}

	// walk the endpoints and see which ones match
	for _, ep := range eps {
		if ep.Endpoint.Tenant == sg.SecurityGroup.Tenant {
			if sg.SecurityGroup.Spec.WorkloadSelector != nil && sg.SecurityGroup.Spec.WorkloadSelector.Matches(labels.Set(ep.Endpoint.Status.WorkloadAttributes)) {
				err = ep.AddSecurityGroup(sg)
				if err != nil {
					log.Errorf("Error attaching endpoint %s to sg %s. Err: %v", ep.Endpoint.Name, sg.SecurityGroup.Name, err)
					return err
				}

				// add the ep to local list
				sg.endpoints[ep.Endpoint.Name] = ep
				sg.SecurityGroup.Status.Workloads = append(sg.SecurityGroup.Status.Workloads, ep.Endpoint.Name)
			}

		}
	}

	return nil
}

// AddEndpoint adds an endpoint to sg
func (sg *SecurityGroupState) AddEndpoint(ep *EndpointState) error {
	// add the ep to local list
	sg.endpoints[ep.Endpoint.Name] = ep
	sg.SecurityGroup.Status.Workloads = append(sg.SecurityGroup.Status.Workloads, ep.Endpoint.Name)
	sg.SecurityGroup.Write()
	return sg.stateMgr.mbus.UpdateObjectWithReferences(sg.SecurityGroup.MakeKey("security"),
		convertSecurityGroup(sg), references(sg.SecurityGroup))
}

// DelEndpoint removes an endpoint from sg
func (sg *SecurityGroupState) DelEndpoint(ep *EndpointState) error {
	// delete the endpoint
	delete(sg.endpoints, ep.Endpoint.Name)
	for i, epname := range sg.SecurityGroup.Status.Workloads {
		if epname == ep.Endpoint.Name {
			sg.SecurityGroup.Status.Workloads = append(sg.SecurityGroup.Status.Workloads[:i], sg.SecurityGroup.Status.Workloads[i+1:]...)
		}
	}
	sg.SecurityGroup.Write()
	return sg.stateMgr.mbus.UpdateObjectWithReferences(sg.SecurityGroup.MakeKey("security"),
		convertSecurityGroup(sg), references(sg.SecurityGroup))
}

// AddPolicy adds a policcy to sg
func (sg *SecurityGroupState) AddPolicy(sgp *SgpolicyState) error {
	// if policy exists, dont try to add it
	_, ok := sg.policies[sgp.NetworkSecurityPolicy.Name]
	if ok {
		return nil
	}

	// add to db
	sg.policies[sgp.NetworkSecurityPolicy.Name] = sgp
	sg.SecurityGroup.Status.Policies = append(sg.SecurityGroup.Status.Policies, sgp.NetworkSecurityPolicy.Name)

	// trigger an update
	sg.SecurityGroup.Write()
	return sg.stateMgr.mbus.UpdateObjectWithReferences(sg.SecurityGroup.MakeKey("security"),
		convertSecurityGroup(sg), references(sg.SecurityGroup))
}

// DeletePolicy deletes a policcy from sg
func (sg *SecurityGroupState) DeletePolicy(sgp *SgpolicyState) error {
	// delete from db
	delete(sg.policies, sgp.NetworkSecurityPolicy.Name)
	for i, pname := range sg.SecurityGroup.Status.Policies {
		if pname == sgp.NetworkSecurityPolicy.Name {
			sg.SecurityGroup.Status.Policies = append(sg.SecurityGroup.Status.Policies[:i], sg.SecurityGroup.Status.Policies[i+1:]...)
		}
	}
	// trigger an update
	sg.SecurityGroup.Write()
	return sg.stateMgr.mbus.UpdateObjectWithReferences(sg.SecurityGroup.MakeKey("security"),
		convertSecurityGroup(sg), references(sg.SecurityGroup))
}

// SecurityGroupStateFromObj conerts from memdb object to network state
func SecurityGroupStateFromObj(obj runtime.Object) (*SecurityGroupState, error) {
	switch obj.(type) {
	case *ctkit.SecurityGroup:
		sgobj := obj.(*ctkit.SecurityGroup)
		switch sgobj.HandlerCtx.(type) {
		case *SecurityGroupState:
			sgs := sgobj.HandlerCtx.(*SecurityGroupState)
			return sgs, nil
		default:
			return nil, ErrIncorrectObjectType
		}

	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewSecurityGroupState creates a new security group state object
func NewSecurityGroupState(sg *ctkit.SecurityGroup, stateMgr *Statemgr) (*SecurityGroupState, error) {
	// create sg state object
	sgs := SecurityGroupState{
		SecurityGroup: sg,
		policies:      make(map[string]*SgpolicyState),
		endpoints:     make(map[string]*EndpointState),
		stateMgr:      stateMgr,
	}

	sg.HandlerCtx = &sgs

	return &sgs, nil
}

func convertSecurityGroup(sgs *SecurityGroupState) *netproto.SecurityGroup {
	creationTime, _ := types.TimestampProto(time.Now())
	sg := netproto.SecurityGroup{
		TypeMeta:   sgs.SecurityGroup.TypeMeta,
		ObjectMeta: agentObjectMeta(sgs.SecurityGroup.ObjectMeta),
		Spec: netproto.SecurityGroupSpec{
			SecurityProfile: "", // FIXME: fill in security profile
		},
	}
	sg.CreationTime = api.Timestamp{Timestamp: *creationTime}

	return &sg
}

// FindSecurityGroup finds security group by name
func (sm *Statemgr) FindSecurityGroup(tenant, name string) (*SecurityGroupState, error) {
	// find the object
	obj, err := sm.FindObject("SecurityGroup", tenant, "default", name)
	if err != nil {
		return nil, err
	}

	return SecurityGroupStateFromObj(obj)
}

// ListSecurityGroups lists all security groups
func (sm *Statemgr) ListSecurityGroups() ([]*SecurityGroupState, error) {
	objs := sm.ListObjects("SecurityGroup")

	var sgs []*SecurityGroupState
	for _, obj := range objs {
		sg, err := SecurityGroupStateFromObj(obj)
		if err != nil {
			return sgs, err
		}

		sgs = append(sgs, sg)
	}

	return sgs, nil
}

// OnSecurityGroupCreate creates a security group
func (sm *Statemgr) OnSecurityGroupCreate(sg *ctkit.SecurityGroup) error {
	// create new sg state
	sgs, err := NewSecurityGroupState(sg, sm)
	if err != nil {
		log.Errorf("Error creating new sg state. Err: %v", err)
		return err
	}

	// attach all matching endpoints
	err = sgs.attachEndpoints()
	if err != nil {
		log.Errorf("Error attaching endpoints to security group %s. Err: %v", sgs.SecurityGroup.Name, err)
		return err
	}
	log.Infof("Created Security Group state {%+v}", sgs)

	// store it in local DB
	return sm.mbus.AddObjectWithReferences(sg.MakeKey("security"), convertSecurityGroup(sgs), references(sg))
}

// OnSecurityGroupUpdate handles sg updates
func (sm *Statemgr) OnSecurityGroupUpdate(sg *ctkit.SecurityGroup, nsg *security.SecurityGroup) error {
	// see if anything changed
	_, ok := ref.ObjDiff(sg.Spec, nsg.Spec)
	if (nsg.GenerationID == sg.GenerationID) && !ok {
		sg.ObjectMeta = nsg.ObjectMeta
		return nil
	}
	sg.ObjectMeta = nsg.ObjectMeta
	sg.Spec = nsg.Spec
	return nil
}

// OnSecurityGroupDelete deletes a security group
func (sm *Statemgr) OnSecurityGroupDelete(sgo *ctkit.SecurityGroup) error {
	// see if we already have it
	obj, err := sm.FindObject("SecurityGroup", sgo.Tenant, "default", sgo.Name)
	if err != nil {
		log.Errorf("Can not find the sg %s|%s", sgo.Tenant, sgo.Name)
		return fmt.Errorf("SecurityGroup not found")
	}

	// convert it to security group state
	sg, err := SecurityGroupStateFromObj(obj)
	if err != nil {
		return err
	}

	// walk all endpoints and remove the sg
	for _, ep := range sg.endpoints {
		err = ep.DelSecurityGroup(sg)
		if err != nil {
			log.Errorf("Error removing security group %s from endpoint %s. Err: %v", sg.SecurityGroup.Name, ep.Endpoint.Name, err)
		}
	}

	// cleanup sg state
	err = sg.Delete()
	if err != nil {
		log.Errorf("Error deleting the sg {%+v}. Err: %v", sg, err)
		return err
	}
	// delete it from the DB
	return sm.mbus.DeleteObjectWithReferences(sg.SecurityGroup.MakeKey("security"),
		convertSecurityGroup(sg), references(sg.SecurityGroup))
}
