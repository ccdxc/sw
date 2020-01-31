// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/runtime"
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
	case "LOOPBACK":
		netif.Status.Type = network.NetworkInterfaceStatus_LOOPBACK_TEP.String()
	default:
		netif.Status.Type = network.NetworkInterfaceStatus_UPLINK_ETH.String() // TBD: what if we receive unrecognized type, perhaps ingore it?
	}
	netif.Tenant = ""
	netif.Namespace = ""
	netif.Status.IFHostStatus = &network.NetworkInterfaceHostStatus{
		HostIfName: agentNetif.Status.IFHostStatus.HostIfName,
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

// OnNetworkInterfaceCreate is empty implementation for interface handling
func (sm *Statemgr) OnNetworkInterfaceCreate(obj *ctkit.NetworkInterface) error {
	return nil
}

// OnNetworkInterfaceUpdate is empty implementation for interface handling
func (sm *Statemgr) OnNetworkInterfaceUpdate(oldObj *ctkit.NetworkInterface, newObj *network.NetworkInterface) error {
	return nil
}

// OnNetworkInterfaceDelete is empty implementation for interface handling
func (sm *Statemgr) OnNetworkInterfaceDelete(obj *ctkit.NetworkInterface) error {
	return nil
}

// SmNetworkInterface is statemanager struct for NetworkInterface
type SmNetworkInterface struct {
	sm *Statemgr
}

// NetworkInterfaceState is a wrapper for NetworkInterface object
type NetworkInterfaceState struct {
	sync.Mutex
	NetworkInterfaceState *ctkit.NetworkInterface `json:"-"` // NetworkInterface object
}

var smgrNetworkInterface *SmNetworkInterface

// CompleteRegistration is the callback function statemgr calls after init is done
func (sma *SmNetworkInterface) CompleteRegistration() {
	// if featureflags.IsOVerlayRoutingEnabled() == false {
	// 	return
	// }

	smgrNetworkInterface = &SmNetworkInterface{}

	sma.sm.SetNetworkInterfaceReactor(smgrNetworkInterface)
}

func init() {
	mgr := MustGetStatemgr()
	smgrNetworkInterface = &SmNetworkInterface{
		sm: mgr,
	}

	mgr.Register("statemgrroute", smgrNetworkInterface)
}

// NewNetworkInterfaceState creates a new NetworkInterfaceState
func NewNetworkInterfaceState(intf *ctkit.NetworkInterface, sma *SmNetworkInterface) (*NetworkInterfaceState, error) {
	ifcfg := &NetworkInterfaceState{
		NetworkInterfaceState: intf,
	}
	intf.HandlerCtx = ifcfg
	return ifcfg, nil
}

func convertIFTypeToAgentProto(in string) string {
	switch in {
	case network.IFType_NONE.String():
		return netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_NONE)]
	case network.IFType_HOST_PF.String():
		return netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_HOST_PF)]
	case network.IFType_UPLINK_ETH.String():
		return netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_UPLINK_ETH)]
	case network.IFType_UPLINK_MGMT.String():
		return netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_UPLINK_MGMT)]
	case network.IFType_LOOPBACK_TEP.String():
		return netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_LOOPBACK)]
	default:
		return "unknown"
	}
}

func convertAgentIFToAPIProto(in string) string {
	switch in {
	case netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_NONE)]:
		return network.IFType_NONE.String()
	case netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_HOST_PF)]:
		return network.IFType_HOST_PF.String()
	case netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_UPLINK_ETH)]:
		return network.IFType_UPLINK_ETH.String()
	case netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_UPLINK_MGMT)]:
		return network.IFType_UPLINK_MGMT.String()
	case netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_L3)]:
		return network.IFType_NONE.String()
	case netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_LOOPBACK)]:
		return network.IFType_LOOPBACK_TEP.String()
	default:
		return "unknown"
	}
}

func convertNetworkInterfaceObject(ifcfg *NetworkInterfaceState) *netproto.Interface {
	agIf := &netproto.Interface{
		ObjectMeta: api.ObjectMeta{
			Tenant:          ifcfg.NetworkInterfaceState.Tenant,
			Namespace:       ifcfg.NetworkInterfaceState.Namespace,
			Name:            ifcfg.NetworkInterfaceState.Name,
			GenerationID:    ifcfg.NetworkInterfaceState.GenerationID,
			ResourceVersion: ifcfg.NetworkInterfaceState.ResourceVersion,
		},
		Spec: netproto.InterfaceSpec{
			AdminStatus: ifcfg.NetworkInterfaceState.Spec.AdminStatus,
			Speed:       ifcfg.NetworkInterfaceState.Spec.Speed,
			MTU:         ifcfg.NetworkInterfaceState.Spec.MTU,
			VrfName:     ifcfg.NetworkInterfaceState.Spec.AttachTenant,
			Network:     ifcfg.NetworkInterfaceState.Spec.AttachNetwork,
		},
	}

	if ifcfg.NetworkInterfaceState.Spec.IPConfig != nil {
		agIf.Spec.IPAddress = ifcfg.NetworkInterfaceState.Spec.IPConfig.IPAddress
	}
	agIf.Spec.Type = convertIFTypeToAgentProto(ifcfg.NetworkInterfaceState.Status.Type)
	if ifcfg.NetworkInterfaceState.Spec.Pause != nil {
		agIf.Spec.Pause = &netproto.PauseSpec{
			Type:           ifcfg.NetworkInterfaceState.Spec.Pause.Type,
			TxPauseEnabled: ifcfg.NetworkInterfaceState.Spec.Pause.TxPauseEnabled,
			RxPauseEnabled: ifcfg.NetworkInterfaceState.Spec.Pause.RxPauseEnabled,
		}
		switch ifcfg.NetworkInterfaceState.Spec.Pause.Type {
		case network.PauseType_DISABLE.String():
			agIf.Spec.Pause.Type = netproto.PauseType_name[int32(netproto.PauseType_DISABLE)]
		case network.PauseType_LINK.String():
			agIf.Spec.Pause.Type = netproto.PauseType_name[int32(netproto.PauseType_LINK)]
		case network.PauseType_PRIORITY.String():
			agIf.Spec.Pause.Type = netproto.PauseType_name[int32(netproto.PauseType_PRIORITY)]
		}
	}
	return agIf
}

// RoutingConfigStateFromObj converts from memdb object to RoutingConfig state
func networkInterfaceStateFromObj(obj runtime.Object) (*NetworkInterfaceState, error) {
	switch obj.(type) {
	case *ctkit.NetworkInterface:
		ifcfg := obj.(*ctkit.NetworkInterface)
		switch ifcfg.HandlerCtx.(type) {
		case *NetworkInterfaceState:
			state := ifcfg.HandlerCtx.(*NetworkInterfaceState)
			return state, nil
		default:
			return nil, fmt.Errorf("incorrect object type")
		}
	default:
		return nil, fmt.Errorf("incorrect object type")
	}
}

// OnNetworkInterfaceCreate creates a NetworkInterface
func (sma *SmNetworkInterface) OnNetworkInterfaceCreate(ctkitNetif *ctkit.NetworkInterface) error {
	log.Info("received OnNetworkInterfaceCreate", ctkitNetif.Spec)
	ifcfg, err := NewNetworkInterfaceState(ctkitNetif, sma)
	if err != nil {
		log.Errorf("error creating network interface state")
	}

	err = sma.sm.AddObject(convertNetworkInterfaceObject(ifcfg))
	return nil
}

// OnNetworkInterfaceUpdate handles update event
func (sma *SmNetworkInterface) OnNetworkInterfaceUpdate(old *ctkit.NetworkInterface, new *network.NetworkInterface) error {
	log.Info("received OnNetworkInterfaceCreate", old.Spec, new.Spec)

	_, ok := ref.ObjDiff(old.Spec, new.Spec)
	if (old.GenerationID == old.GenerationID) && !ok {
		old.ObjectMeta = new.ObjectMeta
		return nil
	}

	// update old state
	old.ObjectMeta = new.ObjectMeta
	old.Spec = new.Spec

	return nil
}

// OnNetworkInterfaceDelete deletes an networkInterface
func (sma *SmNetworkInterface) OnNetworkInterfaceDelete(ctkitNetif *ctkit.NetworkInterface) error {
	// see if we already have it
	obj, err := sma.sm.FindObject("NeteorkInterface", ctkitNetif.Tenant, "default", ctkitNetif.Name)
	if err != nil {
		log.Errorf("Can not find the Network Interface %s|%s", ctkitNetif.Tenant, ctkitNetif.Name)
		return fmt.Errorf("NetworkInterface not found")
	}

	ifcfg, err := networkInterfaceStateFromObj(obj)
	if err != nil {
		log.Errorf("Can not find the Network Interface %s|%s (%s)", ctkitNetif.Tenant, ctkitNetif.Name, err)
		return err
	}

	return sma.sm.mbus.DeleteObjectWithReferences(ctkitNetif.MakeKey("network"),
		convertNetworkInterfaceObject(ifcfg), references(ctkitNetif))
}

// GetNetworkInterfaceWatchOptions is a dummy handler used in init if no one registers the handler
func (sma *SmNetworkInterface) GetNetworkInterfaceWatchOptions() *api.ListWatchOptions {
	opts := &api.ListWatchOptions{}
	return opts
}
