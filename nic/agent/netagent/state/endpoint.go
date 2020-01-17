// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/gogo/protobuf/proto"

	"regexp"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	dnetproto "github.com/pensando/sw/nic/agent/protos/generated/delphi/netproto/delphi"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ErrEndpointNotFound is returned when endpoint is not found
var ErrEndpointNotFound = errors.New("endpoint not found")

// EndpointType holds either local or remote endpoint type
type EndpointType int

var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)

const (
	// Local Endpoints where NodeUUID on endpoint spec matches the NAPLES NodeUUID
	Local = iota // 0
	// Remote Endpoints where NodeUUID on endpoint spec doesn't match the NAPLES NodeUUID
	Remote
)

// EndpointCreateReq creates an endpoint
func (na *Nagent) EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, *types.IntfInfo, error) {
	// make an RPC call to controller
	//ep, err := na.Ctrlerif.EndpointCreateReq(epinfo)
	//if err != nil {
	//	log.Errorf("Error resp from netctrler for ep create {%+v}. Err: %v", epinfo, err)
	//	return nil, nil, err
	//}

	// call the datapath
	err := na.CreateEndpoint(epinfo)
	if err != nil {
		return nil, nil, err
	}

	return epinfo, nil, nil
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
	return na.DeleteEndpoint(ep.Tenant, ep.Namespace, ep.Name)
}

// CreateEndpoint creates an endpoint
func (na *Nagent) CreateEndpoint(ep *netproto.Endpoint) error {
	// check if the endpoint already exists and convert it to an update
	err := na.validateMeta(ep.Kind, ep.ObjectMeta)
	if err != nil {
		return err
	}
	key := na.Solver.ObjectKey(ep.ObjectMeta, ep.TypeMeta)
	na.Lock()
	oldEp, ok := na.EndpointDB[key]
	na.Unlock()

	if ok {
		// check if endpoint contents are same
		if !proto.Equal(&oldEp.Spec, &ep.Spec) {
			log.Errorf("Endpoint %+v already exists. New ep {%+v}", oldEp, ep)
			return errors.New("Endpoint already exists")
		}

		log.Infof("Received duplicate endpoint create for ep {%+v}", ep.ObjectMeta)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(ep.ObjectMeta)
	if err != nil {
		return err
	}

	// find the corresponding vrf.
	vrf, err := na.ValidateVrf(ep.Tenant, ep.Namespace, ep.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", ep.Spec.VrfName)
		return err
	}

	// check if we have the network endpoint is referring to
	// FIXME: if network gets deleted after endpoint is created, how do we handle it?
	nw, err := na.FindNetwork(api.ObjectMeta{Tenant: ep.Tenant, Namespace: ep.Namespace, Name: ep.Spec.NetworkName})
	if err != nil {
		log.Errorf("Error finding the network %v. Err: %v", ep.Spec.NetworkName, err)
		return err
	}

	origSpecIPAddresses := ep.Spec.IPv4Addresses

	// Validate EP IP.
	for idx, address := range ep.Spec.IPv4Addresses {
		if len(address) == 0 {
			continue
		}

		if _, _, err := net.ParseCIDR(address); err != nil {
			// Try parsing as IP
			if len(net.ParseIP(address)) == 0 {
				log.Errorf("Endpoint IP Addresses %v invalid. Must either be a CIDR or IP", address)
				return fmt.Errorf("endpoint IP Addresses %v invalid. Must either be a CIDR or IP", address)
			}
			// Slap a /32 if not specified
			ep.Spec.IPv4Addresses[idx] = fmt.Sprintf("%s/32", address)
		}
	}

	// call the datapath
	if ep.Spec.NodeUUID == na.NodeUUID {
		// User specified a local ep create on a specific lif
		lifID, err := na.findIntfID(ep, Local)
		if err != nil {
			log.Errorf("could not find an interface to associate to the endpoint")
			return err
		}

		// Allocate ID only on first object creates and use existing ones during config replay
		if ep.Status.EnicID == 0 {
			enicID, err := na.Store.GetNextID(types.InterfaceID, 0)
			if err != nil {
				log.Errorf("Could not allocate enic for the local EP. %v", err)
				return err
			}
			// Ensure the ID is non-overlapping with existing hw interfaces. Allcate IDs from 10000 onwards.
			// Since HAL has some predefined interfaces which overlaps at 1K interfaces
			enicID = enicID + types.EnicOffset
			// save the enic id in the ep status for deletions
			ep.Status.EnicID = enicID
		}
		_, err = na.Datapath.CreateLocalEndpoint(ep, nw, lifID, ep.Status.EnicID, vrf)
		if err != nil {
			log.Errorf("Error creating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return err
		}

	} else {
		intfID, err := na.findIntfID(ep, Remote)
		if err != nil {
			log.Errorf("could not find an interface to associate to the endpoint")
			return err
		}
		err = na.Datapath.CreateRemoteEndpoint(ep, nw, intfID, vrf)
		if err != nil {
			log.Errorf("Error creating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return err
		}
	}
	ep.Spec.IPv4Addresses = origSpecIPAddresses
	err = na.Solver.Add(nw, ep)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", nw, ep)
		return err
	}

	err = na.Solver.Add(ns, ep)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, ep)
		return err
	}

	err = na.Solver.Add(vrf, ep)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", vrf, ep)
		return err
	}

	// add the ep to database
	err = na.saveEndpoint(ep)

	// done
	return err
}

// saveEndpoint saves endpoint to state stores
func (na *Nagent) saveEndpoint(ep *netproto.Endpoint) error {
	key := na.Solver.ObjectKey(ep.ObjectMeta, ep.TypeMeta)

	// add the ep to database
	na.Lock()
	na.EndpointDB[key] = ep
	na.Unlock()

	// write to delphi
	if na.DelphiClient != nil && ep.Spec.NodeUUID == na.NodeUUID {
		dep := dnetproto.Endpoint{
			Key:      key,
			Endpoint: ep,
		}

		err := na.DelphiClient.SetObject(&dep)
		if err != nil {
			log.Errorf("Error writing Endpoint %s to delphi. Err: %v", key, err)
			return err
		}
	}

	dat, _ := ep.Marshal()
	err := na.Store.RawWrite(ep.GetKind(), ep.GetKey(), dat)

	return err
}

// discardEndpoint deletes endpoint from state stores
func (na *Nagent) discardEndpoint(ep *netproto.Endpoint) error {
	key := na.Solver.ObjectKey(ep.ObjectMeta, ep.TypeMeta)

	// remove from the database
	na.Lock()
	delete(na.EndpointDB, key)
	na.Unlock()

	// delete it from delphi
	if na.DelphiClient != nil && ep.Spec.NodeUUID == na.NodeUUID {
		dep := dnetproto.Endpoint{
			Key:      key,
			Endpoint: ep,
		}

		err := na.DelphiClient.DeleteObject(&dep)
		if err != nil {
			log.Errorf("Error deleting Endpoint %s from delphi. Err: %v", key, err)
			return err
		}
	}

	err := na.Store.RawDelete(ep.GetKind(), ep.GetKey())

	return err
}

// UpdateEndpoint updates an endpoint. ToDo Handle updates in datapath
func (na *Nagent) UpdateEndpoint(ep *netproto.Endpoint) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(ep.ObjectMeta)
	if err != nil {
		return err
	}

	// find the corresponding vrf.
	_, err = na.ValidateVrf(ep.Tenant, ep.Namespace, ep.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", ep.Spec.VrfName)
		return err
	}

	// check if the endpoint already exists and convert it to an update
	key := na.Solver.ObjectKey(ep.ObjectMeta, ep.TypeMeta)
	na.Lock()
	oldEp, ok := na.EndpointDB[key]
	na.Unlock()
	if !ok {
		return fmt.Errorf("endpoint %v not found", key)

	}

	// check if endpoint contents are same
	//if oldEp.ObjectMeta.ResourceVersion == ep.ObjectMeta.ResourceVersion {
	if proto.Equal(&oldEp.Spec, &ep.Spec) {
		log.Info("Nothing to update")
		return nil
	}
	log.Infof("Valid EP Update. Old: %v, New: %v", oldEp.Spec, ep.Spec)

	// find the network
	nw, err := na.FindNetwork(api.ObjectMeta{Tenant: ep.Tenant, Namespace: ep.Namespace, Name: ep.Spec.NetworkName})
	if err != nil {
		log.Errorf("Error finding the network %v. Err: %v", ep.Spec.NetworkName, err)
		return err
	}

	// find the corresponding vrf.
	vrf, err := na.ValidateVrf(ep.Tenant, ep.Namespace, ep.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", ep.Spec.VrfName)
		return err
	}

	lifID, err := na.findIntfID(ep, Local)
	if err != nil {
		log.Errorf("could not find an interface to associate to the endpoint")
		return err
	}

	// If endpoint's network changes, delete the old endpoint and create new one
	// this can happen if endpoint was deleted and re-created on Venice while this naples was disconnected from it.
	if oldEp.Spec.NetworkName != ep.Spec.NetworkName {
		// delete old endpoint
		err = na.DeleteEndpoint(ep.Tenant, ep.Namespace, ep.Name)
		if err != nil {
			log.Errorf("Error deleting endpoint %+v", ep.ObjectMeta)
			// continue with create even if delete fails
		}

		// create endpoint again
		return na.CreateEndpoint(ep)
	}

	// Validate EP IP.
	for idx, address := range ep.Spec.IPv4Addresses {
		if len(address) == 0 {
			continue
		}

		if _, _, err := net.ParseCIDR(address); err != nil {
			// Try parsing as IP
			if len(net.ParseIP(address)) == 0 {
				log.Errorf("Endpoint IP Addresses %v invalid. Must either be a CIDR or IP", address)
				return fmt.Errorf("endpoint IP Addresses %v invalid. Must either be a CIDR or IP", address)
			}
			// Slap a /32 if not specified
			ep.Spec.IPv4Addresses[idx] = fmt.Sprintf("%s/32", address)
		}
	}

	// save the enic id in the ep status for deletions
	ep.Status.EnicID = oldEp.Status.EnicID

	// call the datapath
	if ep.Spec.NodeUUID == na.NodeUUID {
		err = na.Datapath.UpdateLocalEndpoint(ep, nw, lifID, ep.Status.EnicID, vrf)
		if err != nil {
			log.Errorf("Error updating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return err
		}
	} else {
		err = na.Datapath.UpdateRemoteEndpoint(ep, nw)
		if err != nil {
			log.Errorf("Error updating the endpoint {%+v} in datapath. Err: %v", ep, err)
			return err
		}
	}

	// add the ep to database
	err = na.saveEndpoint(ep)

	return err
}

// DeleteEndpoint deletes an endpoint. ToDo Handle deletes in datapath
func (na *Nagent) DeleteEndpoint(tn, namespace, name string) error {
	ep := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(ep.Kind, ep.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(ep.ObjectMeta)
	if err != nil {
		return err
	}

	// check if we have the endpoint
	key := na.Solver.ObjectKey(ep.ObjectMeta, ep.TypeMeta)
	na.Lock()
	ep, ok := na.EndpointDB[key]
	na.Unlock()
	if !ok {
		log.Errorf("Endpoint %#v was not found", key)
		return ErrEndpointNotFound
	}

	// find the network
	nw, err := na.FindNetwork(api.ObjectMeta{Tenant: ep.Tenant, Namespace: ep.Namespace, Name: ep.Spec.NetworkName})
	if err != nil {
		log.Errorf("Error finding the network %v. Err: %v", ep.Spec.NetworkName, err)
		return err
	}

	// find the corresponding vrf.
	vrf, err := na.ValidateVrf(ep.Tenant, ep.Namespace, ep.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", ep.Spec.VrfName)
		return err
	}

	// check if the current network has any objects referring to it
	err = na.Solver.Solve(ep)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", ep.Name, err)
		return err
	}

	// call the datapath
	if ep.Spec.NodeUUID == na.NodeUUID {
		err = na.Datapath.DeleteLocalEndpoint(ep, nw, ep.Status.EnicID)
		if err != nil {
			log.Errorf("Error deleting the endpoint {%+v} in datapath. Err: %v", ep, err)
			return err
		}
	} else {
		err = na.Datapath.DeleteRemoteEndpoint(ep, nw)
		if err != nil {
			log.Errorf("Error deleting the endpoint {%+v} in datapath. Err: %v", ep, err)
			return err
		}
	}
	err = na.Solver.Remove(vrf, ep)
	if err != nil {
		log.Errorf("Could not remove dependency. Parent: %v. Child: %v", vrf, ep)
		return err
	}

	err = na.Solver.Remove(ns, ep)
	if err != nil {
		log.Errorf("Could not remove dependency. Parent: %v. Child: %v", ns, ep)
		return err
	}

	err = na.Solver.Remove(nw, ep)
	if err != nil {
		log.Errorf("Could not remove dependency. Parent: %v. Child: %v", nw, ep)
		return err
	}

	// remove from the database
	err = na.discardEndpoint(ep)

	// done
	return err
}

// FindEndpoint finds an endpoint by name
func (na *Nagent) FindEndpoint(meta api.ObjectMeta) (*netproto.Endpoint, error) {
	ep := &netproto.Endpoint{
		TypeMeta:   api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: meta,
	}

	// check if we have the endpoint
	key := na.Solver.ObjectKey(ep.ObjectMeta, ep.TypeMeta)
	na.Lock()
	eps, ok := na.EndpointDB[key]
	na.Unlock()
	if !ok {
		return nil, ErrEndpointNotFound
	}

	return eps, nil
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

// FindLocalEndpoint returns a local endpoint
func (na *Nagent) FindLocalEndpoint(tn, namespace string) (*netproto.Endpoint, error) {
	// lock the db
	na.Lock()
	defer na.Unlock()

	// check uuid match
	for _, ep := range na.EndpointDB {
		if ep.Tenant == tn && ep.Namespace == namespace && ep.Spec.GetNodeUUID() == na.NodeUUID {
			return ep, nil
		}
	}

	return nil, fmt.Errorf("no local endpoints found")
}

func (na *Nagent) findAvailableInterface(count uint64, epMAC string, intfType string) (string, error) {
	// convert the ip address to int
	var ifIdx uint64
	lifs := na.getLifs()
	uplinks := na.getUplinks()

	mac, err := na.macTouint64(epMAC)
	if err != nil {
		log.Errorf("could not parse mac address from %v", epMAC)
		return "", fmt.Errorf("could not parse mac address from %v", epMAC)
	}
	ifIdx = mac % count

	switch strings.ToLower(intfType) {
	case "uplink":
		return uplinks[ifIdx].Name, nil
	case "lif":
		return lifs[ifIdx].Name, nil
	default:
		log.Errorf("Invalid interface type.")
		return "", fmt.Errorf("invalid interface type specified. %v", intfType)
	}
}

// findTunnelID associates an remote ep to a tunnel ID
func (na *Nagent) findTunnelID(epMeta api.ObjectMeta, intfName string) (uint64, error) {
	var tunMeta api.ObjectMeta

	tun := strings.Split(intfName, "/")
	switch len(tun) {
	// tunnel in local namespace
	case 1:
		tunMeta.Tenant = epMeta.Tenant
		tunMeta.Namespace = epMeta.Namespace
		tunMeta.Name = intfName
		t, err := na.FindTunnel(tunMeta)
		if err != nil {
			log.Errorf("could not find the tunnel %v. Err: %v", intfName, err)
			return 0, fmt.Errorf("could not find a tunnel %v interface to associate to the endpoint. Err: %v ", intfName, err)
		}
		return t.Status.TunnelID, nil
	// tunnel in remote namespace
	case 2:
		tunMeta.Tenant = epMeta.Tenant
		tunMeta.Namespace = tun[0]
		tunMeta.Name = tun[1]
		t, err := na.FindTunnel(tunMeta)
		if err != nil {
			log.Errorf("could not find the tunnel %v. Err: %v", intfName, err)
			return 0, fmt.Errorf("could not find a tunnel %v interface to associate to the endpoint. Err: %v ", intfName, err)
		}
		return t.Status.TunnelID, nil
	default:
		return 0, fmt.Errorf("endpoint interface %v should point to a valid tunnel", intfName)
	}
}

func (na *Nagent) findIntfID(ep *netproto.Endpoint, epType EndpointType) (uint64, error) {
	switch {
	// Local EP Create, interface types unspecified by user. Don't associate any LIFs
	case epType == Local:
		return 0, nil
		// Remote EP Create, interface types unspecified by user. Pass through
	case epType == Remote:
		// check if internal EPs for lateral objects. This is to enable OOB tests in IOTA. TODO Revisit this with HAL
		log.Infof("Management Link for Remote EP Creates: %v", na.MgmtLink)
		if ep.Spec.NetworkName == types.InternalUntaggedNetwork && na.MgmtLink == types.OOBManagementInterface {
			mgmtUplink, ok := na.findIntfByName(types.ManagementUplink)
			if !ok {
				log.Error("Failed to find management uplink.")
				return 0, nil
			}
			return mgmtUplink.Status.InterfaceID, nil
		}
		uplinkCount, err := na.countIntfs("UPLINK_ETH")
		if err != nil {
			log.Errorf("could not enumerate uplinks. Err: %v", err)
			return 0, fmt.Errorf("could not enumerate uplinks. Err: %v", err)
		}
		epUplink, err := na.findAvailableInterface(uplinkCount, ep.Spec.MacAddress, "uplink")
		if err != nil {
			log.Errorf("could not find an available uplink. Err: %v", err)
			return 0, fmt.Errorf("could not find an available uplink. Err: %v", err)
		}
		uplink, ok := na.findIntfByName(epUplink)
		if !ok {
			log.Errorf("could not find uplink %v in state", uplink)
			return 0, fmt.Errorf("could not find uplink %v in state", epUplink)
		}
		return uplink.Status.InterfaceID, nil
	}
	log.Errorf("invalid endpoint create spec. {%v} ", ep)
	return 0, fmt.Errorf("ep create should either be remote or local. Remote EPs should point to either a valid uplink or a tunnel")
}

func (na *Nagent) macTouint64(mac string) (uint64, error) {
	hex := macStripRegexp.ReplaceAllLiteralString(mac, "")
	return strconv.ParseUint(hex, 16, 64)
}
