// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func convertNetifObj(nodeID string, agentNetif *netproto.Interface) *network.NetworkInterface {
	// convert agentNetif -> veniceNetif
	netif := &network.NetworkInterface{
		TypeMeta:   agentNetif.TypeMeta,
		ObjectMeta: agentNetif.ObjectMeta,
		Status: network.NetworkInterfaceStatus{
			SmartNIC: nodeID,
			// TBD: PrimaryMac: "tbf",
		},
	}

	switch agentNetif.Status.OperStatus {
	case "UP", "DOWN":
		netif.Status.OperStatus = agentNetif.Status.OperStatus
	default:
		netif.Status.OperStatus = "UP" // TBD: should default be modeled to vencie user?
	}

	switch agentNetif.Spec.Type {
	case "UPLINK_ETH", "UPLINK_MGMT":
		netif.Status.Type = agentNetif.Spec.Type
	case "HOST_PF":
		netif.Status.Type = "ENIC"
	default:
		netif.Status.Type = "UPLINK_ETH" // TBD: what if we receive unrecognized type, perhaps ingore it?
	}
	netif.Tenant = ""
	netif.Namespace = ""
	netif.Status.IFHostStatus = &network.NetworkInterfaceHostStatus{
		// TBD: HostIfName: "tbf",
	}
	netif.Status.IFUplinkStatus = &network.NetworkInterfaceUplinkStatus{
		// TBD: LinkSpeed: "tbf",
	}

	return netif
}

// OnInterfaceCreateReq gets called when agent sends create request
func (sm *Statemgr) OnInterfaceCreateReq(nodeID string, agentNetif *netproto.Interface) error {
	// convert agent's netif struct to the api object
	netif := convertNetifObj(nodeID, agentNetif)

	// store it in local DB
	sm.mbus.AddObject(agentNetif)

	// trigger the create in ctkit, which adds the objec to apiserer
	return sm.ctrler.NetworkInterface().Create(netif)
}

// OnInterfaceUpdateReq gets called when agent sends update request
func (sm *Statemgr) OnInterfaceUpdateReq(nodeID string, agentNetif *netproto.Interface) error {
	return nil
}

// OnInterfaceDeleteReq gets called when agent sends delete request
func (sm *Statemgr) OnInterfaceDeleteReq(nodeID string, agentNetif *netproto.Interface) error {
	obj, err := sm.FindObject("NetworkInterface", agentNetif.ObjectMeta.Tenant, agentNetif.ObjectMeta.Namespace, agentNetif.ObjectMeta.Name)
	if err != nil {
		return err
	}

	// delete the networkInterface
	ctkitNetif, ok := obj.(*ctkit.NetworkInterface)
	if !ok {
		return ErrIncorrectObjectType
	}
	return sm.ctrler.NetworkInterface().Delete(&ctkitNetif.NetworkInterface)
}

// OnInterfaceOperUpdate gets called when agent sends create request
func (sm *Statemgr) OnInterfaceOperUpdate(nodeID string, agentNetif *netproto.Interface) error {
	// FIXME: handle status updates
	return nil
}

// OnInterfaceOperDelete is called when agent sends delete request
func (sm *Statemgr) OnInterfaceOperDelete(nodeID string, agentNetif *netproto.Interface) error {
	return nil
}

// OnNetworkInterfaceCreate creates a NetworkInterface
func (sm *Statemgr) OnNetworkInterfaceCreate(ctkitNetif *ctkit.NetworkInterface) error {
	// we do not save any local state for network if
	return nil
}

// OnNetworkInterfaceUpdate handles update event
func (sm *Statemgr) OnNetworkInterfaceUpdate(ctkitNetif *ctkit.NetworkInterface, netif *network.NetworkInterface) error {
	// we do not save any local state for network if
	return nil
}

// OnNetworkInterfaceDelete deletes an networkInterface
func (sm *Statemgr) OnNetworkInterfaceDelete(ctkitNetif *ctkit.NetworkInterface) error {
	// we do not save any local state for network if
	return nil
}
