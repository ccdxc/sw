// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func convertNetifObj(nodeID string, agentNetif *netproto.Interface) *network.NetworkInterface {
	// convert agentNetif -> veniceNetif
	creationTime, _ := types.TimestampProto(time.Now())
	netif := &network.NetworkInterface{
		TypeMeta:   agentNetif.TypeMeta,
		ObjectMeta: agentNetif.ObjectMeta,
		Status: network.NetworkInterfaceStatus{
			DSC: nodeID,
			// TBD: PrimaryMac: "tbf",
		},
	}
	netif.CreationTime = api.Timestamp{Timestamp: *creationTime}

	switch agentNetif.Status.OperStatus {
	case "UP":
		netif.Status.OperStatus = network.IFStatus_UP.String()
	case "DOWN":
		netif.Status.OperStatus = network.IFStatus_DOWN.String()
	default:
		netif.Status.OperStatus = network.IFStatus_UP.String() // TBD: should default be modeled to vencie user?
	}

	switch agentNetif.Spec.Type {
	case "UPLINK_ETH":
		netif.Status.Type = network.NetworkInterfaceStatus_UPLINK_ETH.String()
	case "UPLINK_MGMT":
		netif.Status.Type = network.NetworkInterfaceStatus_UPLINK_MGMT.String()
	case "HOST_PF":
		netif.Status.Type = network.NetworkInterfaceStatus_HOST_PF.String()
	default:
		netif.Status.Type = network.NetworkInterfaceStatus_UPLINK_ETH.String() // TBD: what if we receive unrecognized type, perhaps ingore it?
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

//GetInterfaceWatchOptions gets options
func (sm *Statemgr) GetInterfaceWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	return &opts
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

//GetNetworkInterfaceWatchOptions gets options
func (sm *Statemgr) GetNetworkInterfaceWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec"}
	return &opts
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
