// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"net"

	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
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

func convertEndpoint(eps *EndpointState) *netproto.Endpoint {
	// build endpoint
	nep := netproto.Endpoint{
		TypeMeta:   eps.Endpoint.TypeMeta,
		ObjectMeta: eps.Endpoint.ObjectMeta,
		Spec: netproto.EndpointSpec{
			WorkloadName:       eps.Endpoint.Status.WorkloadName,
			WorkloadAttributes: eps.Endpoint.Status.WorkloadAttributes,
			NetworkName:        eps.Endpoint.Status.Network,
			SecurityGroups:     eps.Endpoint.Status.SecurityGroups,
			IPv4Address:        eps.Endpoint.Status.IPv4Address,
			IPv4Gateway:        eps.Endpoint.Status.IPv4Gateway,
			IPv6Address:        eps.Endpoint.Status.IPv6Address,
			IPv6Gateway:        eps.Endpoint.Status.IPv6Gateway,
			MacAddress:         eps.Endpoint.Status.MacAddress,
			UsegVlan:           eps.Endpoint.Status.MicroSegmentVlan,
			NodeUUID:           eps.Endpoint.Status.NodeUUID,
		},
	}

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

	return eps.stateMgr.mbus.UpdateObject(convertEndpoint(eps))
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

	return eps.stateMgr.mbus.UpdateObject(convertEndpoint(eps))
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
		if sg.SecurityGroup.Spec.WorkloadSelector.Matches(labels.Set(eps.Endpoint.Status.WorkloadAttributes)) {
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

// OnEndpointAgentStatusSet gets called when agent sends create request
func (sm *Statemgr) OnEndpointAgentStatusSet(nodeID string, objinfo *netproto.Endpoint) error {
	// build the endpoint params
	epp := workload.Endpoint{
		TypeMeta:   objinfo.TypeMeta,
		ObjectMeta: objinfo.ObjectMeta,
		Status: workload.EndpointStatus{
			WorkloadName:   objinfo.Spec.WorkloadName,
			Network:        objinfo.Spec.NetworkName,
			HomingHostAddr: objinfo.Spec.HomingHostAddr,
			HomingHostName: objinfo.Spec.HomingHostName,
			NodeUUID:       objinfo.Spec.NodeUUID,
		},
	}

	return sm.ctrler.Endpoint().Create(&epp)
}

// OnEndpointAgentStatusDelete is called when agent sends delete request
func (sm *Statemgr) OnEndpointAgentStatusDelete(nodeID string, objinfo *netproto.Endpoint) error {
	// find the endpoint
	eps, err := sm.FindEndpoint(objinfo.Tenant, objinfo.Name)
	if err != nil {
		log.Errorf("Could not find the endpoint %+v", objinfo.ObjectMeta)
		return err
	}

	// delete the endpoint
	return sm.ctrler.Endpoint().Delete(&eps.Endpoint.Endpoint)
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
	// find network
	ns, err := sm.FindNetwork(epinfo.Tenant, epinfo.Status.Network)
	if err != nil {
		log.Errorf("could not find the network %s for endpoint %+v. Err: %v", epinfo.Status.Network, epinfo.ObjectMeta, err)
		return err
	}

	if ns.Network.Spec.IPv4Subnet != "" {
		// allocate an IP address
		ipv4Addr, err := ns.allocIPv4Addr(epinfo.Status.IPv4Address)

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

	}

	// create a new endpoint instance
	eps, err := NewEndpointState(epinfo, sm)
	if err != nil {
		log.Errorf("Error creating endpoint state from spec{%+v}, Err: %v", epinfo, err)
		return err
	}

	// save the endpoint in the database
	ns.Lock()
	ns.endpointDB[eps.endpointKey()] = eps
	ns.Unlock()
	sm.mbus.AddObject(convertEndpoint(eps))

	// write the modified network state to api server
	err = ns.Network.Write()
	if err != nil {
		log.Errorf("Error writing the network object. Err: %v", err)
		return err
	}
	return nil
}

// OnEndpointUpdate handles update event
func (sm *Statemgr) OnEndpointUpdate(epinfo *ctkit.Endpoint) error {
	return fmt.Errorf("Endpoint update not implemented")
}

// OnEndpointDelete deletes an endpoint
func (sm *Statemgr) OnEndpointDelete(epinfo *ctkit.Endpoint) error {
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
		return err
	}

	// free the IPv4 address
	err = ns.freeIPv4Addr(eps.Endpoint.Status.IPv4Address)
	if err != nil {
		log.Errorf("Error freeing the endpoint address. Err: %v", err)
	}

	// delete the endpoint
	err = eps.Delete()
	if err != nil {
		log.Errorf("Error deleting the endpoint{%+v}. Err: %v", eps, err)
	}
	// remove it from the database
	ns.Lock()
	delete(ns.endpointDB, eps.endpointKey())
	ns.Unlock()
	sm.mbus.DeleteObject(convertEndpoint(eps))

	log.Infof("Deleted endpoint: %+v", eps)

	// write the modified network state to api server
	err = ns.Network.Write()
	if err != nil {
		log.Errorf("Error writing the network object. Err: %v", err)
	}

	return nil
}
