// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"errors"

	log "github.com/Sirupsen/logrus"
	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
)

// ErrEndpointNotFound is returned when endpoint is not found
var ErrEndpointNotFound = errors.New("Endpoint not found")

// EndpointCreateReq creates an endpoint
func (ag *NetAgent) EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, *IntfInfo, error) {
	// make an RPC call to controller
	epinfo.Status.NodeUUID = ag.NodeUUID.String()
	ep, err := ag.ctrlerif.EndpointCreateReq(epinfo)
	if err != nil {
		log.Errorf("Error resp from netctrler for ep create {%+v}. Err: %v", epinfo, err)
		return nil, nil, err
	}

	// call the datapath
	intfInfo, err := ag.CreateEndpoint(ep)
	return ep, intfInfo, err
}

// EndpointDeleteReq deletes an endpoint
func (ag *NetAgent) EndpointDeleteReq(epinfo *netproto.Endpoint) error {
	// make an RPC call to controller
	ep, err := ag.ctrlerif.EndpointDeleteReq(epinfo)
	if err != nil {
		log.Errorf("Error resp from netctrler for ep delete {%+v}. Err: %v", epinfo, err)
		return err
	}

	// call the datapath
	return ag.DeleteEndpoint(ep)
}

// CreateEndpoint creates an endpoint
func (ag *NetAgent) CreateEndpoint(ep *netproto.Endpoint) (*IntfInfo, error) {
	// lock the agent for ep state change
	ag.Lock()
	defer ag.Unlock()

	// check if the endpoint already exists and convert it to an update
	key := objectKey(ep.ObjectMeta)
	oldEp, ok := ag.endpointDB[key]
	if ok {
		// check if endpoint contents are same
		if !proto.Equal(oldEp, ep) {
			log.Errorf("Endpoint %+v already exists. New ep {%+v}", oldEp, ep)
			return nil, errors.New("Endpoint already exists")
		}

		log.Infof("Received duplicate endpoint create for ep {%+v}", ep)
		return nil, nil
	}

	// check if we have the network endpoint is refering to
	// FIXME: if network gets deleted after endpoint is created, how do we handle it?
	nw, err := ag.FindNetwork(api.ObjectMeta{Tenant: ep.Tenant, Name: ep.Spec.NetworkName})
	if err != nil {
		log.Errorf("Error finding the network %v. Err: %v", ep.Spec.NetworkName, err)
		return nil, err
	}

	// check if security groups its refering to exists
	// FIXME: how do we handle security group getting deleted after ep is created.
	var sgs []*netproto.SecurityGroup
	for _, sgname := range ep.Spec.SecurityGroups {
		sg, serr := ag.FindSecurityGroup(api.ObjectMeta{Tenant: ep.Tenant, Name: sgname})
		if serr != nil {
			log.Errorf("Error finding security group %v. Err: %v", sgname, serr)
			return nil, serr
		}

		sgs = append(sgs, sg)
	}

	// call the datapath
	var intfInfo *IntfInfo
	if ep.Status.NodeUUID == ag.NodeUUID.String() {
		intfInfo, err = ag.datapath.CreateLocalEndpoint(ep, nw, sgs)
		if err != nil {
			log.Errorf("Error creating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return nil, err
		}

	} else {
		err = ag.datapath.CreateRemoteEndpoint(ep, nw, sgs)
		if err != nil {
			log.Errorf("Error creating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return nil, err
		}
	}

	// add the ep to database
	ag.endpointDB[key] = ep

	// done
	return intfInfo, err
}

// UpdateEndpoint updates an endpoint
func (ag *NetAgent) UpdateEndpoint(ep *netproto.Endpoint) error {
	// check if the endpoint already exists and convert it to an update
	key := objectKey(ep.ObjectMeta)
	oldEp, ok := ag.endpointDB[key]
	if ok {
		// check if endpoint contents are same
		if proto.Equal(oldEp, ep) {
			log.Infof("Received duplicate endpoint create for ep {%+v}", ep)
			return nil
		}
	}

	// verify endpoint's network is not changing
	if oldEp.Spec.NetworkName != ep.Spec.NetworkName {
		log.Errorf("Can not change network after endpoint is created. old %s, new %s", oldEp.Spec.NetworkName, ep.Spec.NetworkName)
		return errors.New("Can not change the network after endpoint is created")
	}

	// find the network
	nw, err := ag.FindNetwork(api.ObjectMeta{Tenant: ep.Tenant, Name: ep.Spec.NetworkName})
	if err != nil {
		log.Errorf("Error finding the network %v. Err: %v", ep.Spec.NetworkName, err)
		return err
	}

	// check if security groups its refering to exists
	var sgs []*netproto.SecurityGroup
	for _, sgname := range ep.Spec.SecurityGroups {
		sg, serr := ag.FindSecurityGroup(api.ObjectMeta{Tenant: ep.Tenant, Name: sgname})
		if serr != nil {
			log.Errorf("Error finding security group %v. Err: %v", sgname, serr)
			return serr
		}

		sgs = append(sgs, sg)
	}

	// call the datapath
	if ep.Status.NodeUUID == ag.NodeUUID.String() {
		err = ag.datapath.UpdateLocalEndpoint(ep, nw, sgs)
		if err != nil {
			log.Errorf("Error updating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return err
		}
	} else {
		err = ag.datapath.UpdateRemoteEndpoint(ep, nw, sgs)
		if err != nil {
			log.Errorf("Error updating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return err
		}
	}

	// add the ep to database
	ag.endpointDB[key] = ep

	return nil
}

// DeleteEndpoint deletes an endpoint
func (ag *NetAgent) DeleteEndpoint(ep *netproto.Endpoint) error {
	// lock the agent for ep state change
	ag.Lock()
	defer ag.Unlock()

	// check if we have the endpoint
	key := objectKey(ep.ObjectMeta)
	_, ok := ag.endpointDB[key]
	if !ok {
		log.Errorf("Endpoint %v was not found", ep.ObjectMeta)
		return ErrEndpointNotFound
	}

	// call the datapath
	var err error
	if ep.Status.NodeUUID == ag.NodeUUID.String() {
		err = ag.datapath.DeleteLocalEndpoint(ep)
		if err != nil {
			log.Errorf("Error deleting the endpoint {%+v} in datapath. Err: %v", ep, err)
		}
	} else {
		err = ag.datapath.DeleteRemoteEndpoint(ep)
		if err != nil {
			log.Errorf("Error deleting the endpoint {%+v} in datapath. Err: %v", ep, err)
		}
	}

	// remove from the database
	delete(ag.endpointDB, key)

	// done
	return err
}
