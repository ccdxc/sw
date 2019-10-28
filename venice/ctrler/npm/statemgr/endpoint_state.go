// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"net"
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
	Endpoint *ctkit.Endpoint                `json:"-"` // embedding endpoint object
	groups   map[string]*SecurityGroupState // list of security groups
	stateMgr *Statemgr                      // state manager
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
			WorkloadName:       eps.Status.WorkloadName,
			WorkloadAttributes: eps.Status.WorkloadAttributes,
			NetworkName:        eps.Status.Network,
			SecurityGroups:     eps.Status.SecurityGroups,
			IPv4Addresses:      []string{eps.Status.IPv4Address},
			IPv4Gateway:        eps.Status.IPv4Gateway,
			IPv6Addresses:      []string{eps.Status.IPv6Address},
			IPv6Gateway:        eps.Status.IPv6Gateway,
			MacAddress:         eps.Status.MacAddress,
			UsegVlan:           eps.Status.MicroSegmentVlan,
			NodeUUID:           eps.Status.NodeUUID,
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

	return eps.stateMgr.mbus.UpdateObject(convertEndpoint(&eps.Endpoint.Endpoint))
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

	return eps.stateMgr.mbus.UpdateObject(convertEndpoint(&eps.Endpoint.Endpoint))
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
		time.Sleep(time.Second)
		ns, err = sm.FindNetwork(epinfo.Tenant, epinfo.Status.Network)
		if err != nil {
			log.Errorf("could not find the network %s for endpoint %+v. Err: %v", epinfo.Status.Network, epinfo.ObjectMeta, err)
			return kvstore.NewKeyNotFoundError(epinfo.Status.Network, 0)
		}
	}

	if ns.Network.Spec.IPv4Subnet != "" {
		// allocate an IP address
		ns.Lock()
		ipv4Addr, err := ns.allocIPv4Addr(epinfo.Status.IPv4Address)
		ns.Unlock()
		if err != nil {
			log.Errorf("Error allocating IP address from network {%+v}. Err: %v", ns, err)
			return err
		}

		// allocate a mac address based on IP address
		macAddr := ipv4toMac([]byte{0x01, 0x01}, net.ParseIP(ipv4Addr))

		// convert address to CIDR
		_, ipNet, _ := net.ParseCIDR(ns.Network.Spec.IPv4Subnet)
		subnetMaskLen, _ := ipNet.Mask.Size()
		ipv4Addr = fmt.Sprintf("%s/%d", ipv4Addr, subnetMaskLen)

		// populate allocated values
		epinfo.Status.IPv4Address = ipv4Addr
		epinfo.Status.IPv4Gateway = ns.Network.Spec.IPv4Gateway

		// assign new mac address if we dont have one
		if epinfo.Status.MacAddress == "" {
			epinfo.Status.MacAddress = macAddr.String()
		}

		// write the modified network state to api server
		ns.Lock()
		err = ns.Network.Write()
		ns.Unlock()
		if err != nil {
			log.Errorf("Error writing the network object. Err: %v", err)
			return err
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
	sm.mbus.AddObject(convertEndpoint(&epinfo.Endpoint))

	return nil
}

// OnEndpointUpdate handles update event
func (sm *Statemgr) OnEndpointUpdate(epinfo *ctkit.Endpoint, nep *workload.Endpoint) error {
	epinfo.ObjectMeta = nep.ObjectMeta
	return nil
}

// OnEndpointDelete deletes an endpoint
func (sm *Statemgr) OnEndpointDelete(epinfo *ctkit.Endpoint) error {
	log.Infof("Deleting Endpoint: %#v", epinfo)

	// see if we have the endpoint
	eps, err := sm.FindEndpoint(epinfo.Tenant, epinfo.Name)
	if err != nil {
		log.Errorf("could not find the endpoint %+v", epinfo.ObjectMeta)
		return ErrEndpointNotFound
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
	sm.mbus.DeleteObject(convertEndpoint(&eps.Endpoint.Endpoint))

	log.Infof("Deleted endpoint: %+v", eps)

	return nil
}
