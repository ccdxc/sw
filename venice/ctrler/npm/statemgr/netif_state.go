// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"reflect"
	"sync"
	"time"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/memdb/objReceiver"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/runtime"
)

func convertNetifObj(nodeID string, agentNetif *netproto.Interface) *network.NetworkInterface {
	// convert agentNetif -> veniceNetif
	creationTime, _ := types.TimestampProto(time.Now())
	netif := &network.NetworkInterface{
		TypeMeta: api.TypeMeta{
			Kind: "NetworkInterface",
		},
		ObjectMeta: agentNetif.ObjectMeta,
		Spec: network.NetworkInterfaceSpec{
			AdminStatus: agentNetif.Spec.AdminStatus,
		},
		Status: network.NetworkInterfaceStatus{
			DSC: nodeID,
			// TBD: PrimaryMac: "tbf",
		},
	}
	netif.CreationTime = api.Timestamp{Timestamp: *creationTime}

	switch netif.Spec.AdminStatus {
	case netproto.IFStatus_UP.String():
		netif.Spec.AdminStatus = network.IFStatus_UP.String()
	case netproto.IFStatus_DOWN.String():
		netif.Spec.AdminStatus = network.IFStatus_DOWN.String()
	default:
		netif.Spec.AdminStatus = network.IFStatus_UP.String()
	}
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
		netif.Spec.Type = network.NetworkInterfaceStatus_UPLINK_ETH.String()
		netif.Spec.IPAllocType = network.IPAllocTypes_Static.String()
		netif.Status.Type = network.NetworkInterfaceStatus_UPLINK_ETH.String()
	case "UPLINK_MGMT":
		netif.Spec.Type = network.NetworkInterfaceStatus_UPLINK_MGMT.String()
		netif.Spec.IPAllocType = network.IPAllocTypes_DHCP.String()
		netif.Status.Type = network.NetworkInterfaceStatus_UPLINK_MGMT.String()
	case "HOST_PF":
		netif.Spec.Type = network.NetworkInterfaceStatus_HOST_PF.String()
		netif.Spec.IPAllocType = network.IPAllocTypes_DHCP.String()
		netif.Status.Type = network.NetworkInterfaceStatus_HOST_PF.String()
	case "LOOPBACK":
		netif.Spec.Type = network.NetworkInterfaceStatus_LOOPBACK_TEP.String()
		netif.Spec.IPAllocType = network.IPAllocTypes_Static.String()
		netif.Status.Type = network.NetworkInterfaceStatus_LOOPBACK_TEP.String()
	default:
		netif.Spec.Type = network.NetworkInterfaceStatus_NONE.String()
		netif.Spec.IPAllocType = network.IPAllocTypes_Static.String()
		netif.Status.Type = network.NetworkInterfaceStatus_UPLINK_ETH.String()
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

// NetworkInterfaceStateFromObj conerts from memdb object to network state
func NetworkInterfaceStateFromObj(obj runtime.Object) (*NetworkInterfaceState, error) {
	switch obj.(type) {
	case *ctkit.NetworkInterface:
		sgobj := obj.(*ctkit.NetworkInterface)
		switch sgobj.HandlerCtx.(type) {
		case *NetworkInterfaceState:
			sgs := sgobj.HandlerCtx.(*NetworkInterfaceState)
			return sgs, nil
		default:
			return nil, ErrIncorrectObjectType
		}

	default:
		return nil, ErrIncorrectObjectType
	}
}

// FindNetworkInterface finds mirror session state
func (sm *Statemgr) FindNetworkInterface(tenant, name string) (*NetworkInterfaceState, error) {
	// find the object
	obj, err := sm.FindObject("NetworkInterface", tenant, "default", name)
	if err != nil {
		return nil, err
	}

	return NetworkInterfaceStateFromObj(obj)
}

//GetNetworkInterfaceWatchOptions gets options
func (sm *Statemgr) GetNetworkInterfaceWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"ObjectMeta.Labels", "Spec", "Status"}
	return &opts
}

// OnNetworkInterfaceCreate is empty implementation for interface handling
func (sm *Statemgr) OnNetworkInterfaceCreate(obj *ctkit.NetworkInterface) error {
	log.Infof("Statemanager got OnNetworkInterfaceCreate [%v]", obj.Name)
	return nil
}

// OnNetworkInterfaceUpdate is empty implementation for interface handling
func (sm *Statemgr) OnNetworkInterfaceUpdate(oldObj *ctkit.NetworkInterface, newObj *network.NetworkInterface) error {
	log.Infof("Statemanager got OnNetworkInterfaceUpdate [%v]", newObj.Name)
	return nil
}

// OnNetworkInterfaceDelete is empty implementation for interface handling
func (sm *Statemgr) OnNetworkInterfaceDelete(obj *ctkit.NetworkInterface) error {
	log.Infof("Statemanager got OnNetworkInterfaceDelete [%v]", obj.Name)
	return nil
}

type labelInterfaces struct {
	label map[string]string
	intfs map[string]*NetworkInterfaceState
}

// SmNetworkInterface is statemanager struct for NetworkInterface
type SmNetworkInterface struct {
	sync.Mutex
	sm           *Statemgr
	intfsByLabel map[string]*labelInterfaces //intferfaces by labels
}

// NetworkInterfaceState is a wrapper for NetworkInterface object
type NetworkInterfaceState struct {
	sync.Mutex
	NetworkInterfaceState *ctkit.NetworkInterface `json:"-"` // NetworkInterface object
	pushObject            memdb.PushObjectHandle
	txCollectors          []string
	rxCollectors          []string
	mirrorSessions        []string
}

var smgrNetworkInterface *SmNetworkInterface

// CompleteRegistration is the callback function statemgr calls after init is done
func (sma *SmNetworkInterface) CompleteRegistration() {
	// if featureflags.IsOVerlayRoutingEnabled() == false {
	// 	return
	// }
	initSmNetworkInterface()

	log.Infof("Got CompleteRegistration for SmNetworkInterface")
	sma.sm.SetNetworkInterfaceReactor(smgrNetworkInterface)
	//Send network interface selectively.
	sma.sm.EnableSelectivePushForKind("Interface")
}

func initSmNetworkInterface() {
	mgr := MustGetStatemgr()
	smgrNetworkInterface = &SmNetworkInterface{
		sm:           mgr,
		intfsByLabel: make(map[string]*labelInterfaces),
	}
	mgr.Register("statemgrnetif", smgrNetworkInterface)
}

func init() {
	initSmNetworkInterface()
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
		return netproto.InterfaceSpec_NONE.String()
	case network.IFType_HOST_PF.String():
		return netproto.InterfaceSpec_HOST_PF.String()
	case network.IFType_UPLINK_ETH.String():
		return netproto.InterfaceSpec_UPLINK_ETH.String()
	case network.IFType_UPLINK_MGMT.String():
		return netproto.InterfaceSpec_UPLINK_MGMT.String()
	case network.IFType_LOOPBACK_TEP.String():
		return netproto.InterfaceSpec_LOOPBACK.String()
	default:
		return "unknown"
	}
}

func convertAgentIFToAPIProto(in string) string {
	switch in {
	case netproto.InterfaceSpec_NONE.String():
		return network.IFType_NONE.String()
	case netproto.InterfaceSpec_HOST_PF.String():
		return network.IFType_HOST_PF.String()
	case netproto.InterfaceSpec_UPLINK_ETH.String():
		return network.IFType_UPLINK_ETH.String()
	case netproto.InterfaceSpec_UPLINK_MGMT.String():
		return network.IFType_UPLINK_MGMT.String()
	case netproto.InterfaceSpec_L3.String():
		return network.IFType_NONE.String()
	case netproto.InterfaceSpec_LOOPBACK.String():
		return network.IFType_LOOPBACK_TEP.String()
	default:
		return "unknown"
	}
}

func convertNetworkInterfaceObject(ifcfg *NetworkInterfaceState) *netproto.Interface {
	agIf := &netproto.Interface{
		TypeMeta: api.TypeMeta{
			Kind: "Interface",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:          globals.DefaultTenant,
			Namespace:       globals.DefaultNamespace,
			Name:            ifcfg.NetworkInterfaceState.Name,
			GenerationID:    ifcfg.NetworkInterfaceState.GenerationID,
			ResourceVersion: ifcfg.NetworkInterfaceState.ResourceVersion,
			UUID:            ifcfg.NetworkInterfaceState.UUID,
		},
		Spec: netproto.InterfaceSpec{
			Type:         convertIFTypeToAgentProto(ifcfg.NetworkInterfaceState.Spec.Type),
			AdminStatus:  ifcfg.NetworkInterfaceState.Spec.AdminStatus,
			Speed:        ifcfg.NetworkInterfaceState.Spec.Speed,
			MTU:          ifcfg.NetworkInterfaceState.Spec.MTU,
			VrfName:      ifcfg.NetworkInterfaceState.Spec.AttachTenant,
			Network:      ifcfg.NetworkInterfaceState.Spec.AttachNetwork,
			TxCollectors: ifcfg.txCollectors,
			RxCollectors: ifcfg.rxCollectors,
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

func (sma *SmNetworkInterface) updateLabelMap(ifcfg *NetworkInterfaceState) {

	sma.Lock()
	defer sma.Unlock()

	ls := labels.Set(ifcfg.NetworkInterfaceState.Labels).String()
	var ok bool
	var labelIntfs *labelInterfaces
	if labelIntfs, ok = sma.intfsByLabel[ls]; !ok {
		//intfsMap = make(map[string]*NetworkInterfaceState)
		labelIntfs = &labelInterfaces{
			label: ifcfg.NetworkInterfaceState.Labels,
			intfs: make(map[string]*NetworkInterfaceState),
		}

		sma.intfsByLabel[ls] = labelIntfs
	}
	labelIntfs.intfs[ifcfg.NetworkInterfaceState.GetKey()] = ifcfg
}

func (sma *SmNetworkInterface) findInterfacesByLabel(label map[string]string) ([]*NetworkInterfaceState, error) {

	sma.Lock()
	defer sma.Unlock()
	ls := labels.Set(label).String()

	var ok bool
	var labelIntfs *labelInterfaces

	networkStates := []*NetworkInterfaceState{}
	if labelIntfs, ok = sma.intfsByLabel[ls]; ok {
		for _, intf := range labelIntfs.intfs {
			networkStates = append(networkStates, intf)
		}
	}

	return networkStates, nil
}

func (sma *SmNetworkInterface) getInterfacesMatchingSelector(selector *labels.Selector) ([]*NetworkInterfaceState, error) {

	networkStates := []*NetworkInterfaceState{}

	for _, labelIntfs := range sma.intfsByLabel {
		label := labelIntfs.label
		if selector.Matches(labels.Set(label)) {
			for _, intf := range labelIntfs.intfs {
				networkStates = append(networkStates, intf)
			}
		}
	}

	return networkStates, nil
}

type collectorIntState int

type intfCollectorState struct {
	nwIntf   *NetworkInterfaceState
	colState map[collectorDir]map[string]collectorIntState
}

const (
	collIntfStateInitial collectorIntState = iota
	collIntfStateAdded
	collIntfStateRemoved
)

type collectorOp int
type collectorDir int

const (
	collectorAdd collectorOp = iota
	collectorDelete
)

const (
	collectorTxDir collectorDir = iota
	collectorRxDir
)

func setupInitialStates(nwInterfaceStates []*NetworkInterfaceState, collectorIntfState map[string]*intfCollectorState) {
	//Setup the initial state to figure out if something has changed
	for _, nwIntf := range nwInterfaceStates {
		intfColState := &intfCollectorState{
			nwIntf: nwIntf,
		}
		intfColState.colState = make(map[collectorDir]map[string]collectorIntState)
		intfColState.colState[collectorTxDir] = make(map[string]collectorIntState)
		intfColState.colState[collectorRxDir] = make(map[string]collectorIntState)
		for _, collector := range nwIntf.txCollectors {
			intfColState.colState[collectorTxDir][collector] = collIntfStateInitial
		}
		for _, collector := range nwIntf.rxCollectors {
			intfColState.colState[collectorRxDir][collector] = collIntfStateInitial
		}

		collectorIntfState[nwIntf.NetworkInterfaceState.Name] = intfColState
	}

}

func updateCollectors(intfState *intfCollectorState,
	op collectorOp, dir collectorDir, intfName string, receiver objReceiver.Receiver,
	collectors []string, opCollectors []*mirrorCollector) []string {
	for _, newCol := range opCollectors {
		if _, ok := intfState.colState[dir][newCol.obj.Name]; !ok {
			if op == collectorAdd {
				intfState.colState[dir][newCol.obj.Name] = collIntfStateAdded
				log.Infof("Sending collector %v (dir : %v) for DSC %v Intf %v",
					newCol.obj.Name, dir, intfState.nwIntf.NetworkInterfaceState.Status.DSC,
					intfState.nwIntf.NetworkInterfaceState.Name)
				err := newCol.pushObj.AddObjReceivers([]objReceiver.Receiver{receiver})
				if err != nil {
					log.Errorf("Error adding receiver %v", err)
				}
				collectors = append(collectors, newCol.obj.Name)
			}
		} else {
			if op == collectorDelete {
				intfState.colState[dir][newCol.obj.Name] = collIntfStateRemoved
				for i, col := range collectors {
					if col == newCol.obj.Name {
						collectors[i] = collectors[len(collectors)-1]
						collectors[len(collectors)-1] = ""
						collectors = collectors[:len(collectors)-1]
					}
				}
			} else {
				//Make sure collector added back
				intfState.colState[dir][newCol.obj.Name] = collIntfStateInitial
				found := false
				for _, col := range collectors {
					if col == newCol.obj.Name {
						found = true
					}
				}
				if !found {
					collectors = append(collectors, newCol.obj.Name)
				}
			}
		}
	}
	return collectors
}

func removeMirrorSession(nw *NetworkInterfaceState, session string) {

	for i, msession := range nw.mirrorSessions {
		if msession == session {
			nw.mirrorSessions[i] = nw.mirrorSessions[len(nw.mirrorSessions)-1]
			nw.mirrorSessions[len(nw.mirrorSessions)-1] = ""
			nw.mirrorSessions = nw.mirrorSessions[:len(nw.mirrorSessions)-1]
		}
	}

}

func addMirrorSession(nw *NetworkInterfaceState, session string) {

	for _, msession := range nw.mirrorSessions {
		if msession == session {
			return
		}
	}

	nw.mirrorSessions = append(nw.mirrorSessions, session)
}

func updateInterfaceRequired(intfColState *intfCollectorState) bool {
	for _, dirColState := range intfColState.colState {
		for _, state := range dirColState {
			if state != collIntfStateInitial {
				return true
			}
		}
	}
	return false
}

func (sma *SmNetworkInterface) updateMirror(nw *NetworkInterfaceState) error {

	collectorIntfState := make(map[string]*intfCollectorState)
	setupInitialStates([]*NetworkInterfaceState{nw}, collectorIntfState)

	receiver, err := sma.sm.mbus.FindReceiver(nw.NetworkInterfaceState.Status.DSC)
	if err != nil {
		log.Errorf("Error finding receiver %v", err.Error())
		return err
	}

	intfState, _ := collectorIntfState[nw.NetworkInterfaceState.Name]

	mcollectors := smgrMirrorInterface.getAllMirrorSessionCollectors()

	for _, mcol := range mcollectors {
		if mcol.selector.Matches(labels.Set(nw.NetworkInterfaceState.Labels)) {
			//Mirror selector still matches
			nw.txCollectors = updateCollectors(intfState, collectorAdd, collectorTxDir, nw.NetworkInterfaceState.Name, receiver, nw.txCollectors, mcol.txCollectors)
			nw.rxCollectors = updateCollectors(intfState, collectorAdd, collectorRxDir, nw.NetworkInterfaceState.Name, receiver, nw.rxCollectors, mcol.rxCollectors)
			log.Infof("Adding mirror %v for DSC %v Intf %v",
				mcol.mirrorSession, intfState.nwIntf.NetworkInterfaceState.Status.DSC, intfState.nwIntf.NetworkInterfaceState.Name)
			addMirrorSession(nw, mcol.mirrorSession)

		} else {
			//Mirror selector does not match
			nw.txCollectors = updateCollectors(intfState, collectorDelete, collectorTxDir, nw.NetworkInterfaceState.Name, receiver, nw.txCollectors, mcol.txCollectors)
			nw.rxCollectors = updateCollectors(intfState, collectorDelete, collectorRxDir, nw.NetworkInterfaceState.Name, receiver, nw.rxCollectors, mcol.rxCollectors)
			log.Infof("Removing mirror %v for DSC %v Intf %v",
				mcol.mirrorSession, intfState.nwIntf.NetworkInterfaceState.Status.DSC, intfState.nwIntf.NetworkInterfaceState.Name)
			removeMirrorSession(nw, mcol.mirrorSession)
		}

	}

	if updateInterfaceRequired(intfState) {
		//Sendupdate
		log.Infof("Sending mirror update for DSC %v Intf %v Collectors %v ",
			nw.NetworkInterfaceState.Status.DSC, nw.NetworkInterfaceState.Name, nw.txCollectors)
		err = nw.pushObject.UpdateObjectWithReferences(nw.NetworkInterfaceState.MakeKey(string(apiclient.GroupNetwork)),
			convertNetworkInterfaceObject(nw), references(nw.NetworkInterfaceState))
		if err != nil {
			log.Errorf("Error updating interface %v", err.Error())
			return err
		}
	} else {
		log.Infof("No mirror update for DSC %v Intf %v Collectors %v ",
			nw.NetworkInterfaceState.Status.DSC, nw.NetworkInterfaceState.Name, nw.txCollectors)
	}

	return nil
}

//UpdateCollectorsMatchingSelector  updates collectors matching selector
func (sma *SmNetworkInterface) UpdateCollectorsMatchingSelector(oldSelCol *mirrorSelectorCollectors,
	newSelCol *mirrorSelectorCollectors) error {

	sma.Lock()
	defer sma.Unlock()
	collectorIntfState := make(map[string]*intfCollectorState)

	if newSelCol != nil {
		nwInterfaceStates, err := sma.getInterfacesMatchingSelector(newSelCol.selector)
		if err != nil {
			log.Errorf("Error finding interfaces matching selector %v %v", newSelCol.selector.String(), err.Error())
			return err
		}
		setupInitialStates(nwInterfaceStates, collectorIntfState)
	}

	if oldSelCol != nil && oldSelCol.selector != nil {
		oldNwInterfaceStates, err := sma.getInterfacesMatchingSelector(oldSelCol.selector)
		if err != nil {
			log.Errorf("Error finding interfaces matching selector %v %v", oldSelCol.selector.String(), err.Error())
			return err
		}
		setupInitialStates(oldNwInterfaceStates, collectorIntfState)
	}

	for _, nwColState := range collectorIntfState {
		nw := nwColState.nwIntf
		receiver, err := sma.sm.mbus.FindReceiver(nw.NetworkInterfaceState.Status.DSC)
		if err != nil {
			log.Errorf("Error finding receiver %v", err.Error())
			continue
		}
		if oldSelCol != nil && oldSelCol.selector.Matches(labels.Set(nw.NetworkInterfaceState.Labels)) {
			nw.txCollectors = updateCollectors(nwColState, collectorDelete, collectorTxDir, nw.NetworkInterfaceState.Name, receiver, nw.txCollectors, oldSelCol.txCollectors)
			nw.rxCollectors = updateCollectors(nwColState, collectorDelete, collectorRxDir, nw.NetworkInterfaceState.Name, receiver, nw.rxCollectors, oldSelCol.rxCollectors)
			removeMirrorSession(nw, oldSelCol.mirrorSession)
			log.Infof("Removing mirror %v for DSC %v Intf %v",
				oldSelCol.mirrorSession, nw.NetworkInterfaceState.Status.DSC, nw.NetworkInterfaceState.Name)
		}
		if newSelCol != nil && newSelCol.selector.Matches(labels.Set(nw.NetworkInterfaceState.Labels)) {
			nw.txCollectors = updateCollectors(nwColState, collectorAdd, collectorTxDir, nw.NetworkInterfaceState.Name, receiver, nw.txCollectors, newSelCol.txCollectors)
			nw.rxCollectors = updateCollectors(nwColState, collectorAdd, collectorRxDir, nw.NetworkInterfaceState.Name, receiver, nw.rxCollectors, newSelCol.rxCollectors)
			addMirrorSession(nw, newSelCol.mirrorSession)
			log.Infof("Adding mirror %v for DSC %v Intf %v",
				newSelCol.mirrorSession, nw.NetworkInterfaceState.Status.DSC, nw.NetworkInterfaceState.Name)
		}

		if updateInterfaceRequired(nwColState) {
			//Sendupdate
			log.Infof("Sending mirror update for DSC %v Intf %v Collectors %v ",
				nw.NetworkInterfaceState.Status.DSC, nw.NetworkInterfaceState.Name, nw.txCollectors)
			err = nw.pushObject.UpdateObjectWithReferences(nw.NetworkInterfaceState.MakeKey(string(apiclient.GroupNetwork)),
				convertNetworkInterfaceObject(nw), references(nw.NetworkInterfaceState))
			if err != nil {
				log.Errorf("Error updating interface %v", err.Error())
				continue
			}
			sma.sm.PeriodicUpdaterPush(nw)
		} else {
			log.Infof("No mirror update for DSC %v Intf %v Collectors %v ",
				nw.NetworkInterfaceState.Status.DSC, nw.NetworkInterfaceState.Name, nw.txCollectors)
		}
	}

	return nil

}

// FindNetworkInterface finds network interface
func (sma *SmNetworkInterface) FindNetworkInterface(name string) (*NetworkInterfaceState, error) {
	// find the object
	obj, err := sma.sm.FindObject("NetworkInterface", "", "", name)
	if err != nil {
		return nil, err
	}

	return NetworkInterfaceStateFromObj(obj)
}

func (sma *SmNetworkInterface) clearLabelMap(ifcfg *NetworkInterfaceState) {
	sma.Lock()
	defer sma.Unlock()
	var ok bool
	var labelIntfs *labelInterfaces

	ls := labels.Set(ifcfg.NetworkInterfaceState.Labels).String()
	if labelIntfs, ok = sma.intfsByLabel[ls]; ok {
		delete(labelIntfs.intfs, ifcfg.NetworkInterfaceState.GetKey())
		if len(labelIntfs.intfs) == 0 {
			delete(sma.intfsByLabel, ls)
		}
	}
}

// GetKey returns the key of Network
func (nw *NetworkInterfaceState) GetKey() string {
	return nw.NetworkInterfaceState.GetKey()
}

// Write writes the object to api server
func (nw *NetworkInterfaceState) Write() error {
	var err error

	nw.Lock()
	defer nw.Unlock()

	nw.NetworkInterfaceState.Status.MirroSessions = nw.mirrorSessions
	err = nw.NetworkInterfaceState.Write()

	return err
}

// OnNetworkInterfaceCreate creates a NetworkInterface
func (sma *SmNetworkInterface) OnNetworkInterfaceCreate(ctkitNetif *ctkit.NetworkInterface) error {
	log.Info("received OnNetworkInterfaceCreate", ctkitNetif.Spec)
	ifcfg, err := NewNetworkInterfaceState(ctkitNetif, sma)
	if err != nil {
		log.Errorf("error creating network interface state")
	}

	sma.updateLabelMap(ifcfg)

	receiver, err := sma.sm.mbus.FindReceiver(ifcfg.NetworkInterfaceState.Status.DSC)

	pushObj, err := sma.sm.mbus.AddPushObject(ctkitNetif.MakeKey(string(apiclient.GroupNetwork)), convertNetworkInterfaceObject(ifcfg), references(ctkitNetif),
		[]objReceiver.Receiver{receiver})
	if err != nil {
		log.Errorf("error adding push object %v", err)
		return err
	}

	ifcfg.pushObject = pushObj

	sma.sm.PeriodicUpdaterPush(ifcfg)

	return nil
}

// OnNetworkInterfaceUpdate handles update event
func (sma *SmNetworkInterface) OnNetworkInterfaceUpdate(ctkitNetif *ctkit.NetworkInterface, nctkitNetif *network.NetworkInterface) error {
	log.Info("received OnNetworkInterfaceUpdate", ctkitNetif.Spec, nctkitNetif.Spec)

	// update old state
	currIntf, err := networkInterfaceStateFromObj(ctkitNetif)
	if err != nil {
		log.Errorf("error finding exisiting network interface state %v", err)
		return err
	}

	//Update labels only if changed.
	if !reflect.DeepEqual(ctkitNetif.Labels, nctkitNetif.Labels) {
		sma.clearLabelMap(currIntf)
		ctkitNetif.ObjectMeta = nctkitNetif.ObjectMeta
		sma.updateLabelMap(currIntf)
		currIntf.NetworkInterfaceState.Labels = nctkitNetif.Labels
		sma.updateMirror(currIntf)
	}
	ctkitNetif.ObjectMeta = nctkitNetif.ObjectMeta
	ctkitNetif.Spec = nctkitNetif.Spec

	err = currIntf.pushObject.UpdateObjectWithReferences(ctkitNetif.MakeKey(string(apiclient.GroupNetwork)),
		convertNetworkInterfaceObject(currIntf), references(ctkitNetif))
	if err != nil {
		log.Errorf("error updating  push object %v", err)
		return err
	}

	sma.sm.PeriodicUpdaterPush(currIntf)
	return nil
}

// OnNetworkInterfaceDelete deletes an networkInterface
func (sma *SmNetworkInterface) OnNetworkInterfaceDelete(ctkitNetif *ctkit.NetworkInterface) error {
	// see if we already have it
	obj, err := sma.sm.FindObject("NetworkInterface", ctkitNetif.Tenant, "", ctkitNetif.Name)
	if err != nil {
		log.Errorf("Can not find the Network Interface %s|%s", ctkitNetif.Tenant, ctkitNetif.Name)
		return fmt.Errorf("NetworkInterface not found")
	}

	ifcfg, err := networkInterfaceStateFromObj(obj)
	if err != nil {
		log.Errorf("Can not find the Network Interface %s|%s (%s)", ctkitNetif.Tenant, ctkitNetif.Name, err)
		return err
	}

	sma.clearLabelMap(ifcfg)
	return sma.sm.mbus.DeleteObjectWithReferences(ctkitNetif.MakeKey("network"),
		convertNetworkInterfaceObject(ifcfg), references(ctkitNetif))
}

// GetNetworkInterfaceWatchOptions is a dummy handler used in init if no one registers the handler
func (sma *SmNetworkInterface) GetNetworkInterfaceWatchOptions() *api.ListWatchOptions {
	opts := &api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"ObjectMeta.Labels", "Spec", "Status"}
	return opts
}
