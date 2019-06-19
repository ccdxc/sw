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

// OnInterfaceAgentStatusSet gets called when agent sends create request
func (sm *Statemgr) OnInterfaceAgentStatusSet(nodeID string, agentNetif *netproto.Interface) error {
	// convert agent's netif struct to the api object
	netif := convertNetifObj(nodeID, agentNetif)

	// store it in local DB
	sm.mbus.AddObject(agentNetif)

	// trigger the create in ctkit, which adds the objec to apiserer
	return sm.ctrler.NetworkInterface().Create(netif)
}

// OnInterfaceAgentStatusDelete is called when agent sends delete request
func (sm *Statemgr) OnInterfaceAgentStatusDelete(nodeID string, agentNetif *netproto.Interface) error {
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
