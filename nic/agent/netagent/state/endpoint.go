// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"encoding/binary"
	"errors"
	"fmt"
	"net"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ErrEndpointNotFound is returned when endpoint is not found
var ErrEndpointNotFound = errors.New("endpoint not found")

// EndpointCreateReq creates an endpoint
func (na *Nagent) EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, *types.IntfInfo, error) {
	// make an RPC call to controller
	epinfo.Status.NodeUUID = na.NodeUUID
	ep, err := na.Ctrlerif.EndpointCreateReq(epinfo)
	if err != nil {
		log.Errorf("Error resp from netctrler for ep create {%+v}. Err: %v", epinfo, err)
		return nil, nil, err
	}

	// call the datapath
	intfInfo, err := na.CreateEndpoint(ep)
	return ep, intfInfo, err
}

// EndpointDeleteReq deletes an endpoint
func (na *Nagent) EndpointDeleteReq(epinfo *netproto.Endpoint) error {
	// make an RPC call to controller
	ep, err := na.Ctrlerif.EndpointDeleteReq(epinfo)
	if err != nil {
		log.Errorf("Error resp from netctrler for ep delete {%+v}. Err: %v", epinfo, err)
		return err
	}

	// call the datapath
	return na.DeleteEndpoint(ep)
}

// CreateEndpoint creates an endpoint
func (na *Nagent) CreateEndpoint(ep *netproto.Endpoint) (*types.IntfInfo, error) {
	// check if the endpoint already exists and convert it to an update
	err := na.validateMeta(ep.Kind, ep.ObjectMeta)
	if err != nil {
		return nil, err
	}
	key := objectKey(ep.ObjectMeta, ep.TypeMeta)
	na.Lock()
	oldEp, ok := na.EndpointDB[key]
	na.Unlock()

	if ok {
		// check if endpoint contents are same
		if !proto.Equal(oldEp, ep) {
			log.Errorf("Endpoint %+v already exists. New ep {%+v}", oldEp, ep)
			return nil, errors.New("Endpoint already exists")
		}

		log.Infof("Received duplicate endpoint create for ep {%+v}", ep)
		return nil, nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(ep.Tenant, ep.Namespace)
	if err != nil {
		return nil, err
	}

	// check if we have the network endpoint is referring to
	// FIXME: if network gets deleted after endpoint is created, how do we handle it?
	nw, err := na.FindNetwork(api.ObjectMeta{Tenant: ep.Tenant, Namespace: ep.Namespace, Name: ep.Spec.NetworkName})
	if err != nil {
		log.Errorf("Error finding the network %v. Err: %v", ep.Spec.NetworkName, err)
		return nil, err
	}

	// check if security groups its referring to exists
	// FIXME: how do we handle security group getting deleted after ep is created.
	var sgs []*netproto.SecurityGroup
	for _, sgname := range ep.Spec.SecurityGroups {
		sg, serr := na.FindSecurityGroup(api.ObjectMeta{Tenant: ep.Tenant, Namespace: ep.Namespace, Name: sgname})
		if serr != nil {
			log.Errorf("Error finding security group %v. Err: %v", sgname, serr)
			return nil, serr
		}

		sgs = append(sgs, sg)
	}

	// call the datapath
	var intfInfo *types.IntfInfo
	if ep.Status.NodeUUID == na.NodeUUID {
		// User specified a local ep create on a specific lif
		var epLIF string
		if len(ep.Spec.Interface) > 0 {
			epLIF = ep.Spec.Interface
		} else {
			lifCount, err := na.countIntfs("LIF")
			if err != nil {
				return nil, err
			}
			epLIF, err = na.findAvailableInterface(lifCount, ep.Status.IPv4Address, "LIF")
		}
		lif, ok := na.findIntfByName(epLIF)
		if !ok {
			log.Errorf("could not find a lif")
			return nil, fmt.Errorf("could not find the specified interface %v", epLIF)
		}
		// Allocate an ENIC ID. ToDo capture the allocated enic ID in status to ensure that this can be deleted in the datapath
		enicID, err := na.Store.GetNextID(types.InterfaceID)
		if err != nil {
			log.Errorf("Could not allocate enic for the local EP. %v", err)
			return nil, err
		}
		// Ensure the ID is non-overlapping with existing hw interfaces.
		enicID = enicID + maxNumUplinks
		intfInfo, err = na.Datapath.CreateLocalEndpoint(ep, nw, sgs, lif.Status.InterfaceID, enicID, ns)
		if err != nil {
			log.Errorf("Error creating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return nil, err
		}

	} else {
		var pinnedUplink string
		if len(ep.Spec.Interface) > 0 {
			pinnedUplink = ep.Spec.Interface
		} else {
			uplinkCount, err := na.countIntfs("UPLINK")
			if err != nil {
				return nil, err
			}
			pinnedUplink, err = na.findAvailableInterface(uplinkCount, ep.Status.IPv4Address, "UPLINK")
		}
		uplink, ok := na.findIntfByName(pinnedUplink)
		if !ok {
			log.Errorf("could not find an uplink")
			return nil, fmt.Errorf("could not find the specified interface %v", pinnedUplink)
		}
		err = na.Datapath.CreateRemoteEndpoint(ep, nw, sgs, uplink.Status.InterfaceID, ns)
		if err != nil {
			log.Errorf("Error creating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return nil, err
		}
	}

	// add the ep to database
	na.Lock()
	na.EndpointDB[key] = ep
	na.Unlock()
	err = na.Store.Write(ep)

	// done
	return intfInfo, err
}

// UpdateEndpoint updates an endpoint. ToDo Handle updates in datapath
func (na *Nagent) UpdateEndpoint(ep *netproto.Endpoint) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(ep.Tenant, ep.Namespace)
	if err != nil {
		return err
	}
	// check if the endpoint already exists and convert it to an update
	key := objectKey(ep.ObjectMeta, ep.TypeMeta)
	na.Lock()
	oldEp, ok := na.EndpointDB[key]
	na.Unlock()
	if !ok {
		return fmt.Errorf("endpoint not found")

	}
	// check if endpoint contents are same
	if proto.Equal(oldEp, ep) {
		log.Infof("Received duplicate endpoint create for ep {%+v}", ep)
		return nil
	}

	// verify endpoint's network is not changing
	if oldEp.Spec.NetworkName != ep.Spec.NetworkName {
		log.Errorf("Can not change network after endpoint is created. old %s, new %s", oldEp.Spec.NetworkName, ep.Spec.NetworkName)
		return errors.New("Can not change the network after endpoint is created")
	}

	// find the network
	nw, err := na.FindNetwork(api.ObjectMeta{Tenant: ep.Tenant, Namespace: ep.Namespace, Name: ep.Spec.NetworkName})
	if err != nil {
		log.Errorf("Error finding the network %v. Err: %v", ep.Spec.NetworkName, err)
		return err
	}

	// check if security groups its referring to exists
	var sgs []*netproto.SecurityGroup
	for _, sgname := range ep.Spec.SecurityGroups {
		sg, serr := na.FindSecurityGroup(api.ObjectMeta{Tenant: ep.Tenant, Namespace: ep.Namespace, Name: sgname})
		if serr != nil {
			log.Errorf("Error finding security group %v. Err: %v", sgname, serr)
			return serr
		}

		sgs = append(sgs, sg)
	}

	// call the datapath
	if ep.Status.NodeUUID == na.NodeUUID {
		err = na.Datapath.UpdateLocalEndpoint(ep, nw, sgs)
		if err != nil {
			log.Errorf("Error updating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return err
		}
	} else {
		err = na.Datapath.UpdateRemoteEndpoint(ep, nw, sgs)
		if err != nil {
			log.Errorf("Error updating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return err
		}
	}

	// add the ep to database
	na.Lock()
	na.EndpointDB[key] = ep
	na.Unlock()
	err = na.Store.Write(ep)

	return err
}

// DeleteEndpoint deletes an endpoint. ToDo Handle deletes in datapath
func (na *Nagent) DeleteEndpoint(ep *netproto.Endpoint) error {
	err := na.validateMeta(ep.Kind, ep.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	_, err = na.FindNamespace(ep.Tenant, ep.Namespace)
	if err != nil {
		return err
	}
	// check if we have the endpoint
	key := objectKey(ep.ObjectMeta, ep.TypeMeta)
	na.Lock()
	ep, ok := na.EndpointDB[key]
	na.Unlock()
	if !ok {
		log.Errorf("Endpoint %#v was not found", key)
		return ErrEndpointNotFound
	}

	// call the datapath
	if ep.Status.NodeUUID == na.NodeUUID {
		err = na.Datapath.DeleteLocalEndpoint(ep)
		if err != nil {
			log.Errorf("Error deleting the endpoint {%+v} in datapath. Err: %v", ep, err)
		}
	} else {
		err = na.Datapath.DeleteRemoteEndpoint(ep)
		if err != nil {
			log.Errorf("Error deleting the endpoint {%+v} in datapath. Err: %v", ep, err)
		}
	}

	// remove from the database
	na.Lock()
	delete(na.EndpointDB, key)
	na.Unlock()
	err = na.Store.Delete(ep)

	// done
	return err
}

// ListEndpoint returns the list of endpoints
func (na *Nagent) ListEndpoint() []*netproto.Endpoint {
	var epList []*netproto.Endpoint

	// lock the db
	na.Lock()
	defer na.Unlock()

	// walk all endpoints
	for _, ep := range na.EndpointDB {
		epList = append(epList, ep)
	}

	return epList
}

func (na *Nagent) findAvailableInterface(count uint64, IPAddress, intfType string) (string, error) {
	// convert the ip address to int
	ip, _, err := net.ParseCIDR(IPAddress)
	if err != nil {
		log.Errorf("Error parsing the IP Address. Err: %v", err)
		return "", err
	}
	switch intfType {
	case "UPLINK":
		if len(IPAddress) == 16 {
			intIP := binary.BigEndian.Uint32(ip[12:16])
			return fmt.Sprintf("default-uplink-%d", uint64(intIP)%count), nil
		}
		intIP := binary.BigEndian.Uint32(ip)
		return fmt.Sprintf("default-uplink-%d", uint64(intIP)%count), nil
	case "LIF":
		if len(IPAddress) == 16 {
			intIP := binary.BigEndian.Uint32(ip[12:16])
			return fmt.Sprintf("default-lif-%d", uint64(intIP)%count), nil
		}
		intIP := binary.BigEndian.Uint32(ip)
		return fmt.Sprintf("default-lif-%d", uint64(intIP)%count), nil
	default:
		log.Errorf("Invalid interface type.")
		return "", fmt.Errorf("invalid interface type specified. %v", intfType)
	}
}
