package state

import (
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"reflect"
	"strings"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	netAgentState "github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/nic/agent/troubleshooting/utils"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	halMaxMirrorSession         = 8
	mirrorSessionDropRuleIDType = "MirrorSessionDropRuleID"
)

// Tagent is an instance of Troubleshooting agent
type Tagent types.TsAgent

// ErrInvalidMirrorSpec error code is returned when mirror spec is invalid
var ErrInvalidMirrorSpec = errors.New("Mirror specification is incorrect")

// ErrMirrorSpecResource error code is returned when mirror create failed due to resource error
var ErrMirrorSpecResource = errors.New("Out of resource. No more mirror resource available")

// ErrMsInternal error code is returned when mirror session agent code runs into data discrepency
var ErrMsInternal = errors.New("Mirror Session internal error")

// ErrDbRead error code is returned when error is encountered when reading objects from DB
var ErrDbRead = errors.New("Error retrieving object from database")

var nAgent *netAgentState.Nagent

var getMgmtIP func() string

// NewTsAgent creates new troubleshooting agent
func NewTsAgent(dp types.TsDatapathAPI, nodeUUID string, na *netAgentState.Nagent, getMgmtIPAddr func() string) (*Tagent, error) {
	var tsa Tagent
	var err error

	nAgent = na
	getMgmtIP = getMgmtIPAddr
	emdb := na.Store

	restart := false

	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "default",
		},
	}

	_, err = emdb.Read(&tn)

	// Blank slate. Persist config and do init stuff
	if err != nil {
		err := emdb.Write(&tn)
		if err != nil {
			emdb.Close()
			return nil, err
		}
	} else {
		restart = true
	}

	err = tsa.init(emdb, nodeUUID, dp, restart)
	if err != nil {
		emdb.Close()
		return nil, err
	}

	err = dp.SetAgent(&tsa)
	if err != nil {
		// cleanup emstore and return
		emdb.Close()
		return nil, err
	}

	return &tsa, nil
}

// RegisterCtrlerIf registers a controller object
func (tsa *Tagent) RegisterCtrlerIf(ctrlerif types.CtrlerAPI) error {
	// ensure two controller plugins dont register
	if tsa.Ctrlerif != nil {
		log.Fatalf("Multiple controllers registers to trouble shooting agent.")
	}

	// add it to controller list
	tsa.Ctrlerif = ctrlerif

	return nil
}

// Stop stops the agent
func (tsa *Tagent) Stop() error {
	return nil
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta, T api.TypeMeta) string {
	switch strings.ToLower(T.Kind) {
	case "tenant":
		return fmt.Sprintf("%s", meta.Name)
	case "namespace":
		return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
	default:
		return fmt.Sprintf("%s|%s|%s", meta.Tenant, meta.Namespace, meta.Name)
	}
}

// GetAgentID returns agent UUID
func (tsa *Tagent) GetAgentID() string {
	return tsa.NodeUUID
}

func (tsa *Tagent) findMirrorSession(meta api.ObjectMeta) (*netproto.MirrorSession, error) {
	typeMeta := api.TypeMeta{
		Kind: "MirrorSession",
	}

	mirrorsession := objectKey(meta, typeMeta)
	tsa.Lock()
	ms, ok := tsa.DB.MirrorSessionDB[mirrorsession]
	tsa.Unlock()
	if !ok {
		return nil, fmt.Errorf("MirrorSession not found %v", meta.Name)
	}
	return ms, nil
}

func (tsa *Tagent) init(emdb emstore.Emstore, nodeUUID string, dp types.TsDatapathAPI, restart bool) error {
	var err error
	tsa.Store = emdb
	tsa.NodeUUID = nodeUUID
	tsa.Datapath = dp

	tsa.DB.MirrorSessionDB = make(map[string]*netproto.MirrorSession)
	tsa.DB.MirrorSessionNameToID = make(map[string]uint64)
	tsa.DB.MirrorSessionIDToObj = make(map[uint64]types.MirrorSessionObj)
	tsa.DB.FlowMonitorRuleToID = make(map[types.FlowMonitorRuleSpec]uint64)
	tsa.DB.FlowMonitorRuleIDToObj = make(map[uint64]types.FlowMonitorObj)
	tsa.DB.DropRuleToID = make(map[halproto.DropReasons]uint64)
	tsa.DB.DropRuleIDToObj = make(map[uint64]types.DropMonitorObj)
	tsa.DB.AllocatedMirrorIds = make(map[uint64]bool)

	if restart {
		log.Infof("TS Agent restoring from localDB")
		// Walk through all mirror session Objects stored in emstore DB and rebuild map DBs
		listmirrorObj := types.MirrorSessionObj{
			TypeMeta: api.TypeMeta{Kind: "MonitorSessionObject"},
		}
		mirrorObjs, err := tsa.Store.List(&listmirrorObj)
		if err == nil {
			for _, storedMirrorObj := range mirrorObjs {
				mirrorObj, ok := storedMirrorObj.(*types.MirrorSessionObj)
				if !ok {
					log.Errorf("Mirror session object retrieved from db cannot be used")
					return ErrDbRead
				}
				key := objectKey(mirrorObj.ObjectMeta, mirrorObj.TypeMeta)
				mirrorID := mirrorObj.MirrorID
				tsa.DB.MirrorSessionNameToID[key] = mirrorID
				tsa.DB.MirrorSessionIDToObj[mirrorID] = *mirrorObj
				tsa.DB.AllocatedMirrorIds[mirrorID] = true
			}
		}
		listFlowRuleObj := types.FlowMonitorObj{
			TypeMeta: api.TypeMeta{Kind: "FlowMonitorRuleObject"},
		}
		flowRuleObjs, err := tsa.Store.List(&listFlowRuleObj)
		if err == nil {
			for _, storedFlowRuleObj := range flowRuleObjs {
				flowRuleObj, ok := storedFlowRuleObj.(*types.FlowMonitorObj)
				if !ok {
					log.Errorf("Flow monitor object retrieved from db cannot be used")
					return ErrDbRead
				}
				ruleID := flowRuleObj.RuleID
				tsa.DB.FlowMonitorRuleToID[flowRuleObj.Spec] = ruleID
				tsa.DB.FlowMonitorRuleIDToObj[ruleID] = *flowRuleObj
			}
		}
		listDropRuleObj := types.DropMonitorObj{
			TypeMeta: api.TypeMeta{Kind: "DropMonitorRuleObject"},
		}
		dropRuleObjs, err := tsa.Store.List(&listDropRuleObj)
		if err == nil {
			for _, storedDropRuleObj := range dropRuleObjs {
				dropRuleObj, ok := storedDropRuleObj.(*types.DropMonitorObj)
				if !ok {
					log.Errorf("Drop monitor object retrieved from db cannot be used")
					return ErrDbRead
				}
				ruleID := dropRuleObj.RuleID
				tsa.DB.DropRuleToID[dropRuleObj.Spec.DropReasons] = ruleID
				tsa.DB.DropRuleIDToObj[ruleID] = *dropRuleObj
			}
		}
		listMirrorSession := netproto.MirrorSession{
			TypeMeta: api.TypeMeta{Kind: "MonitorSession"},
		}
		mirrorSessions, err := tsa.Store.List(&listMirrorSession)
		if err == nil {
			for _, storedMirrorSession := range mirrorSessions {
				mirrorSession, ok := storedMirrorSession.(*netproto.MirrorSession)
				if !ok {
					log.Errorf("Mirror sesion spec object retrieved from db cannot be used")
					return ErrDbRead
				}
				key := objectKey(mirrorSession.ObjectMeta, mirrorSession.TypeMeta)
				tsa.DB.MirrorSessionDB[key] = mirrorSession
			}
		}
	}
	return err
}

func (tsa *Tagent) allocateMirrorSessionID(msName string) uint64 {
	if _, v := tsa.DB.MirrorSessionNameToID[msName]; v {
		return tsa.DB.MirrorSessionNameToID[msName]
	}
	var i uint64
	for i = 0; i < halMaxMirrorSession; i++ {
		if !tsa.DB.AllocatedMirrorIds[i] {
			break
		}
	}
	if i < halMaxMirrorSession {
		tsa.DB.AllocatedMirrorIds[i] = true
		tsa.DB.MirrorSessionNameToID[msName] = i
		return i
	}
	return halMaxMirrorSession + 1
}

func (tsa *Tagent) getMirrorSessionID(msName string) uint64 {
	if _, v := tsa.DB.MirrorSessionNameToID[msName]; v {
		return tsa.DB.MirrorSessionNameToID[msName]
	}
	return halMaxMirrorSession + 1
}

func (tsa *Tagent) deleteMirrorSession(msName string) error {
	if _, v := tsa.DB.MirrorSessionNameToID[msName]; v {
		i := tsa.DB.MirrorSessionNameToID[msName]
		tsa.DB.AllocatedMirrorIds[i] = false
		delete(tsa.DB.MirrorSessionDB, msName)
		delete(tsa.DB.MirrorSessionNameToID, msName)
		delete(tsa.DB.MirrorSessionIDToObj, i)
		log.Debugf("Deleted mirror session. {%+v}", msName)
	}
	return nil
}

func (tsa *Tagent) allocateDropRuleID(dropReason halproto.DropReasons) (uint64, bool, error) {
	var ruleID uint64
	var err error
	allocated := false
	if _, v := tsa.DB.DropRuleToID[dropReason]; v {
		ruleID = tsa.DB.DropRuleToID[dropReason]
	} else {
		ruleID, err = tsa.Store.GetNextID(mirrorSessionDropRuleIDType, 0)
		if err != nil {
			log.Errorf("Could not allocate drop rule id. {%+v}", err)
			return 0, allocated, err
		}
		allocated = true
		tsa.DB.DropRuleToID[dropReason] = ruleID
		log.Debugf("Drop Rule ID %d", tsa.DB.DropRuleToID[dropReason])
	}
	return ruleID, allocated, nil
}

func (tsa *Tagent) allocateFlowMonitorRuleID(flowRule types.FlowMonitorRuleSpec) (uint64, bool, error) {
	var ruleID uint64
	var err error
	allocated := false
	if _, v := tsa.DB.FlowMonitorRuleToID[flowRule]; v {
		ruleID = tsa.DB.FlowMonitorRuleToID[flowRule]
	} else {
		ruleID, err = tsa.Store.GetNextID(types.FlowMonitorRuleIDType, 0)
		if err != nil {
			log.Errorf("Could not allocate drop rule id. {%+v}", err)
			return 0, allocated, err
		}
		allocated = true
		tsa.DB.FlowMonitorRuleToID[flowRule] = ruleID
	}
	return ruleID, allocated, nil
}

func getAllDropReasons() []halproto.DropReasons {
	allDropReasons := []halproto.DropReasons{
		{DropMalformedPkt: true},
		{DropInputMapping: true},
		{DropInputMappingDejavu: true},
		{DropFlowHit: true},
		{DropFlowMiss: true},
		{DropNacl: true},
		{DropIpsg: true},
		{DropIpNormalization: true},
		{DropTcpNormalization: true},
		{DropTcpRstWithInvalidAckNum: true},
		{DropTcpNonSynFirstPkt: true},
		{DropIcmpNormalization: true},
		{DropInputPropertiesMiss: true},
		{DropTcpOutOfWindow: true},
		{DropTcpSplitHandshake: true},
		{DropTcpWinZeroDrop: true},
		{DropTcpDataAfterFin: true},
		{DropTcpNonRstPktAfterRst: true},
		{DropTcpInvalidResponderFirstPkt: true},
		{DropTcpUnexpectedPkt: true},
		{DropSrcLifMismatch: true},
		{DropParserIcrcError: true},
		{DropParseLenError: true},
		{DropHardwareError: true},
	}
	return allDropReasons
}

func getNetWorkPolicyDropReasons() []halproto.DropReasons {
	NwPolicyDropReasons := []halproto.DropReasons{
		{DropInputMapping: true},
		{DropInputMappingDejavu: true},
		{DropFlowHit: true},
		{DropFlowMiss: true},
		{DropNacl: true},
		{DropIpsg: true},
		{DropInputPropertiesMiss: true},
		{DropSrcLifMismatch: true},
	}
	return NwPolicyDropReasons
}

func getFireWallPolicyDropReasons() []halproto.DropReasons {
	FwPolicyDropReasons := []halproto.DropReasons{
		{DropIpNormalization: true},
		{DropTcpNormalization: true},
		{DropTcpRstWithInvalidAckNum: true},
		{DropTcpNonSynFirstPkt: true},
		{DropIcmpNormalization: true},
		{DropTcpOutOfWindow: true},
		{DropTcpSplitHandshake: true},
		{DropTcpWinZeroDrop: true},
		{DropTcpDataAfterFin: true},
		{DropTcpNonRstPktAfterRst: true},
		{DropTcpInvalidResponderFirstPkt: true},
		{DropTcpUnexpectedPkt: true},
	}
	return FwPolicyDropReasons
}

func buildIPAddrProtoObj(ipaddr *types.IPAddrDetails) *halproto.IPAddress {
	IPAddr := &halproto.IPAddress{}
	if ipaddr.IsIpv4 && !ipaddr.IsSubnet {
		//v4, no-cidr
		IPAddr = &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: binary.BigEndian.Uint32(ipaddr.IP),
			},
		}
	} else if !ipaddr.IsIpv4 && !ipaddr.IsSubnet {
		//v6, no-cidr
		IPAddr = &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET6,
			V4OrV6: &halproto.IPAddress_V6Addr{
				V6Addr: ipaddr.IP,
			},
		}
	}
	return IPAddr
}

//func buildVeniceCollectorProtoObj(mirrorSession *netproto.MirrorSession, mirrorSessID uint64) []*halproto.MirrorSessionSpec_LocalSpanIf {
//	var mirrorCollectors []*halproto.MirrorSessionSpec_LocalSpanIf
//	for _, mirrorCollector := range mirrorSession.Spec.Collectors {
//		if mirrorCollector.Type == "venice" {
//			mirrorDestObj := &halproto.MirrorSessionSpec_LocalSpanIf{}
//			//TODO
//			mirrorCollectors = append(mirrorCollectors, mirrorDestObj)
//		}
//	}
//	return mirrorCollectors
//}

func buildErspanCollectorProtoObj(mirrorSession *netproto.MirrorSession, mirrorSessID uint64) []*halproto.MirrorSessionSpec_ErspanSpec {
	var mirrorCollectors []*halproto.MirrorSessionSpec_ErspanSpec
	for _, mirrorCollector := range mirrorSession.Spec.Collectors {
		//if mirrorCollector.Type == "erspan" {
		//mirrorCollector.ExportCfg.Transport -- Does this info from ctrler need to be used ??
		destIPDetails := utils.BuildIPAddrDetails(mirrorCollector.ExportCfg.Destination)
		mirrorDestObj := &halproto.MirrorSessionSpec_ErspanSpec{
			ErspanSpec: &halproto.ERSpanSpec{
				DestIp: buildIPAddrProtoObj(destIPDetails),
				SpanId: uint32(mirrorSessID), //For now this value is same as mirrorSessionID.
				//SrcIp: HAL can for now choose appropriate value.
				//Dscp:  HAL can for now choose appropriate value.
			},
		}
		mirrorCollectors = append(mirrorCollectors, mirrorDestObj)
		//}
	}
	return mirrorCollectors
}

func buildMirrorTrafficCollectorProtoObj(mirrorSession *netproto.MirrorSession, mirrorSessID uint64) []*halproto.MirrorSessionSpec_ErspanSpec {
	//veniceCollectors := buildVeniceCollectorProtoObj(mirrorSession, mirrorSessID)
	erspanCollectors := buildErspanCollectorProtoObj(mirrorSession, mirrorSessID)
	return erspanCollectors
}

func getVrfID(mirrorSession *netproto.MirrorSession) (vrfID uint64, err error) {
	vrfObj, err := nAgent.ValidateVrf(mirrorSession.Tenant, mirrorSession.Namespace, mirrorSession.Spec.VrfName)
	if err != nil {
		log.Errorf("failed to find vrf %s. Err: %v", mirrorSession.Spec.VrfName, err)
		return uint64(0), fmt.Errorf("failed to find vrf %s. Err: %v", mirrorSession.Spec.VrfName, err)
	}
	return vrfObj.Status.VrfID, nil
}

func (tsa *Tagent) createHALMirrorSessionProtoObj(mirrorSession *netproto.MirrorSession, sessID uint64) (*halproto.MirrorSessionRequestMsg, error) {

	erspanCollectors := buildMirrorTrafficCollectorProtoObj(mirrorSession, sessID)
	//if len(veniceCollectors) == 0 && len(erspanCollectors) == 0 {
	//	log.Errorf("mirror session collector is neither erspan nor venice/local")
	//	return nil, ErrInvalidMirrorSpec
	//}

	// TODO: LocalSpan/Venice; for now populate only ERSPAN collector details
	if len(erspanCollectors) == 0 {
		log.Errorf("mirror session erspan collector is empty")
		return nil, ErrInvalidMirrorSpec
	}

	vrfID, err := getVrfID(mirrorSession)
	if err != nil {
		log.Errorf("mirror session tenant is invalid")
		return nil, ErrInvalidMirrorSpec
	}
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}
	mirrorSpec := halproto.MirrorSessionSpec{
		KeyOrHandle: &halproto.MirrorSessionKeyHandle{
			KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
				MirrorsessionId: sessID,
			},
		},
		VrfKeyHandle: vrfKey,
		Snaplen:      mirrorSession.Spec.PacketSize,
		Destination:  erspanCollectors[0], // HAL accepts only one collector per mirrorSessionSpec. TODO
	}
	ReqMsg := halproto.MirrorSessionRequestMsg{
		Request: []*halproto.MirrorSessionSpec{&mirrorSpec},
	}

	return &ReqMsg, nil
}

func checkIDContainment(IDs []uint64, ID uint64) (bool, int) {
	for i, v := range IDs {
		if v == ID {
			return true, i
		}
	}
	return false, 0
}

func (tsa *Tagent) buildDropRuleUpdateProtoObj(mirrorSession *netproto.MirrorSession,
	dropReason *halproto.DropReasons, updateDropRuleIDs []uint64) (*halproto.DropMonitorRuleSpec, error) {

	ruleID, allocated, err := tsa.allocateDropRuleID(*dropReason)
	if err != nil {
		return nil, err
	}
	if allocated {
		//Should not be allcated; RuleID should have already existed...
		log.Errorf("Missing drop monitor rule in local DB")
		return nil, nil
	}
	contains, _ := checkIDContainment(updateDropRuleIDs, ruleID)
	if !contains {
		return nil, nil
	}
	//Build list of mirror sessions proto obj matching list
	//of mirrorSession the rule is part of
	dropRuleObj := tsa.DB.DropRuleIDToObj[ruleID]
	var msIDs []*halproto.MirrorSessionKeyHandle
	for _, mid := range dropRuleObj.MirrorSessionIDs {
		midObj := &halproto.MirrorSessionKeyHandle{
			KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
				MirrorsessionId: mid,
			},
		}
		msIDs = append(msIDs, midObj)
	}

	dropRuleSpec := halproto.DropMonitorRuleSpec{
		KeyOrHandle: &halproto.DropMonitorRuleKeyHandle{
			KeyOrHandle: &halproto.DropMonitorRuleKeyHandle_DropmonitorruleId{
				DropmonitorruleId: ruleID,
			},
		},
		MsKeyHandle: msIDs,
		Reasons:     dropReason,
	}
	return &dropRuleSpec, nil
}

func (tsa *Tagent) buildDropRuleDeleteProtoObj(mirrorSession *netproto.MirrorSession,
	dropReason *halproto.DropReasons, deleteDropRuleIDs []uint64) (*halproto.DropMonitorRuleDeleteRequest, error) {

	ruleID, allocated, err := tsa.allocateDropRuleID(*dropReason)
	if err != nil {
		return nil, err
	}
	if allocated {
		//Should not be allcated; RuleID should have already existed...
		log.Errorf("Missing drop monitor rule in local DB")
		return nil, nil
	}
	contains, _ := checkIDContainment(deleteDropRuleIDs, ruleID)
	if !contains {
		return nil, nil
	}
	dropRule := halproto.DropMonitorRuleDeleteRequest{
		KeyOrHandle: &halproto.DropMonitorRuleKeyHandle{
			KeyOrHandle: &halproto.DropMonitorRuleKeyHandle_DropmonitorruleId{
				DropmonitorruleId: ruleID,
			},
		},
	}
	return &dropRule, nil
}

func (tsa *Tagent) createDropMonitorRuleIDMatchingHALProtoObj(mirrorSession *netproto.MirrorSession,
	mirrorSessID uint64, updateDropRuleIDs, deleteDropRuleIDs []uint64) ([]*halproto.DropMonitorRuleRequestMsg, []*halproto.DropMonitorRuleDeleteRequestMsg, error) {

	var updateReqMsgList []*halproto.DropMonitorRuleRequestMsg
	var deleteReqMsgList []*halproto.DropMonitorRuleDeleteRequestMsg

	//for _, filter := range mirrorSession.Spec.PacketFilters {
	//	updateReqMsg := halproto.DropMonitorRuleRequestMsg{}
	//	deleteReqMsg := halproto.DropMonitorRuleDeleteRequestMsg{}
	//	if filter == "ALL_DROPS" {
	//		allDropReasons := getAllDropReasons()
	//		for i := range allDropReasons {
	//			dropRuleSpec, _ := tsa.buildDropRuleUpdateProtoObj(mirrorSession, &allDropReasons[i], updateDropRuleIDs)
	//			if dropRuleSpec != nil {
	//				updateReqMsg.Request = append(updateReqMsg.Request, dropRuleSpec)
	//			} else {
	//				deleteDropRule, _ := tsa.buildDropRuleDeleteProtoObj(mirrorSession, &allDropReasons[i], deleteDropRuleIDs)
	//				if deleteDropRule != nil {
	//					deleteReqMsg.Request = append(deleteReqMsg.Request, deleteDropRule)
	//				}
	//			}
	//		}
	//	} else if filter == "NETWORK_POLICY_DROP" {
	//		nwPolicyDrops := getNetWorkPolicyDropReasons()
	//		for i := range nwPolicyDrops {
	//			dropRuleSpec, _ := tsa.buildDropRuleUpdateProtoObj(mirrorSession, &nwPolicyDrops[i], updateDropRuleIDs)
	//			if dropRuleSpec != nil {
	//				updateReqMsg.Request = append(updateReqMsg.Request, dropRuleSpec)
	//			} else {
	//				deleteDropRule, _ := tsa.buildDropRuleDeleteProtoObj(mirrorSession, &nwPolicyDrops[i], deleteDropRuleIDs)
	//				if deleteDropRule != nil {
	//					deleteReqMsg.Request = append(deleteReqMsg.Request, deleteDropRule)
	//				}
	//			}
	//		}
	//	} else if filter == "FIREWALL_POLICY_DROP" {
	//		fwPolicyDrops := getFireWallPolicyDropReasons()
	//		for i := range fwPolicyDrops {
	//			dropRuleSpec, _ := tsa.buildDropRuleUpdateProtoObj(mirrorSession, &fwPolicyDrops[i], updateDropRuleIDs)
	//			if dropRuleSpec != nil {
	//				updateReqMsg.Request = append(updateReqMsg.Request, dropRuleSpec)
	//			} else {
	//				deleteDropRule, _ := tsa.buildDropRuleDeleteProtoObj(mirrorSession, &fwPolicyDrops[i], deleteDropRuleIDs)
	//				if deleteDropRule != nil {
	//					deleteReqMsg.Request = append(deleteReqMsg.Request, deleteDropRule)
	//				}
	//			}
	//		}
	//	}
	//	updateReqMsgList = append(updateReqMsgList, &updateReqMsg)
	//	deleteReqMsgList = append(deleteReqMsgList, &deleteReqMsg)
	//}
	return updateReqMsgList, deleteReqMsgList, nil
}

func (tsa *Tagent) buildDropRuleCreateProtoObj(mirrorSession *netproto.MirrorSession, mirrorSessID uint64, dropReason *halproto.DropReasons) (*halproto.DropMonitorRuleSpec, uint64, bool, error) {
	ruleID, allocated, err := tsa.allocateDropRuleID(*dropReason)
	if err != nil {
		return nil, 0, false, err
	}
	if allocated {
		// create drop rule using drop reason
		dropRuleObj := types.DropMonitorObj{
			TypeMeta: api.TypeMeta{Kind: "DropMonitorRuleObject"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    mirrorSession.ObjectMeta.Tenant,
				Namespace: "",
				Name:      "",
			},
			Spec: types.DropRuleSpec{
				DropReasons: *dropReason,
			},
			RuleID: ruleID,
		}
		dropRuleObj.MirrorSessionIDs = append(dropRuleObj.MirrorSessionIDs, mirrorSessID)
		tsa.DB.DropRuleIDToObj[ruleID] = dropRuleObj
	} else {
		dropRuleObj := tsa.DB.DropRuleIDToObj[ruleID]
		// Rule already existed. Check if mirrorSession is already associated with the rule.
		contains, _ := checkIDContainment(dropRuleObj.MirrorSessionIDs, mirrorSessID)
		if !contains {
			dropRuleObj.MirrorSessionIDs = append(dropRuleObj.MirrorSessionIDs, mirrorSessID)
		}
	}
	//List of mirror sessions obj
	dropRuleObj := tsa.DB.DropRuleIDToObj[ruleID]
	var msIDs []*halproto.MirrorSessionKeyHandle
	for _, mid := range dropRuleObj.MirrorSessionIDs {
		midObj := &halproto.MirrorSessionKeyHandle{
			KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
				MirrorsessionId: mid,
			},
		}
		msIDs = append(msIDs, midObj)
	}
	dropRuleSpec := halproto.DropMonitorRuleSpec{
		KeyOrHandle: &halproto.DropMonitorRuleKeyHandle{
			KeyOrHandle: &halproto.DropMonitorRuleKeyHandle_DropmonitorruleId{
				DropmonitorruleId: ruleID,
			},
		},
		MsKeyHandle: msIDs,
		Reasons:     dropReason,
	}
	return &dropRuleSpec, ruleID, allocated, nil
}

func (tsa *Tagent) createHALDropMonitorRulesProtoObj(mirrorSession *netproto.MirrorSession, mirrorSessID uint64) ([]*halproto.DropMonitorRuleRequestMsg, []uint64, error) {

	var newRuleIDs []uint64

	var ReqMsgList []*halproto.DropMonitorRuleRequestMsg
	//for _, filter := range mirrorSession.Spec.PacketFilters {
	//	ReqMsg := halproto.DropMonitorRuleRequestMsg{}
	//	// Iterate over drops reasons and create one ruleSpec for each drop condition.
	//	// HAL expects each droprule to contain only one dropreason code.
	//	//create array of all drop reasons
	//	if filter == "ALL_DROPS" {
	//		allDropReasons := getAllDropReasons()
	//		for i := range allDropReasons {
	//			dropRuleSpec, ruleID, allocated, err := tsa.buildDropRuleCreateProtoObj(mirrorSession, mirrorSessID, &allDropReasons[i])
	//			if err == nil {
	//				ReqMsg.Request = append(ReqMsg.Request, dropRuleSpec)
	//				if allocated {
	//					newRuleIDs = append(newRuleIDs, ruleID)
	//				}
	//			} else {
	//				return nil, nil, err
	//			}
	//		}
	//	} else if filter == "NETWORK_POLICY_DROP" {
	//		nwPolicyDrops := getNetWorkPolicyDropReasons()
	//		for i := range nwPolicyDrops {
	//			dropRuleSpec, ruleID, allocated, err := tsa.buildDropRuleCreateProtoObj(mirrorSession, mirrorSessID, &nwPolicyDrops[i])
	//			if err == nil {
	//				ReqMsg.Request = append(ReqMsg.Request, dropRuleSpec)
	//				if allocated {
	//					newRuleIDs = append(newRuleIDs, ruleID)
	//				}
	//			} else {
	//				return nil, nil, err
	//			}
	//		}
	//	} else if filter == "FIREWALL_POLICY_DROP" {
	//		fwPolicyDrops := getFireWallPolicyDropReasons()
	//		for i := range fwPolicyDrops {
	//			dropRuleSpec, ruleID, allocated, err := tsa.buildDropRuleCreateProtoObj(mirrorSession, mirrorSessID, &fwPolicyDrops[i])
	//			if err == nil {
	//				ReqMsg.Request = append(ReqMsg.Request, dropRuleSpec)
	//				if allocated {
	//					newRuleIDs = append(newRuleIDs, ruleID)
	//				}
	//			} else {
	//				return nil, nil, err
	//			}
	//		}
	//	}
	//	ReqMsgList = append(ReqMsgList, &ReqMsg)
	//}
	return ReqMsgList, newRuleIDs, nil
}

func (tsa *Tagent) createFlowMonitorRuleIDMatchingHALProtoObj(mirrorSession *netproto.MirrorSession,
	mirrorSessID uint64, updateFmRuleIDs, deleteFmRuleIDs []uint64) ([]*halproto.FlowMonitorRuleRequestMsg, []*halproto.FlowMonitorRuleDeleteRequestMsg, error) {

	var updateReqMsgList []*halproto.FlowMonitorRuleRequestMsg
	var deleteReqMsgList []*halproto.FlowMonitorRuleDeleteRequestMsg
	var deleteProtoObj bool
	var updateProtoObj bool

	//NB: Here mirrorSessionSpec is already stored spec; which is old spec and the one that is under update

	vrfID, err := getVrfID(mirrorSession)
	if err != nil {
		log.Errorf("mirror session tenant is invalid")
		return nil, nil, ErrInvalidMirrorSpec
	}
	for _, rule := range mirrorSession.Spec.MatchRules {
		srcIPs, destIPs, srcMACs, destMACs, appPorts, srcIPStrings, destIPStrings := utils.ExpandCompositeMatchRule(mirrorSession.ObjectMeta, &rule, nAgent.FindEndpoint)
		// Create protobuf requestMsgs on cross product of
		//  - srcIPs, destIPs, Apps
		//  - srcMACs, destMACs, Apps
		flowMonitorRuleSpecified := false
		ipFmRuleList, err := utils.CreateIPAddrCrossProductRuleList(srcIPs, destIPs, appPorts, srcIPStrings, destIPStrings)
		if err == nil {
			flowMonitorRuleSpecified = true
		}

		macFmRuleList, err := utils.CreateMACAddrCrossProductRuleList(srcMACs, destMACs, appPorts)
		if err == nil {
			flowMonitorRuleSpecified = true
		}

		//TODO: Fold in ether-type in monitor rule

		if !flowMonitorRuleSpecified {
			log.Errorf("Match Rules specified with only AppPort Selector without IP/MAC")
			return nil, nil, utils.ErrInvalidFlowMonitorRule
		}

		updateReqMsg := halproto.FlowMonitorRuleRequestMsg{}
		deleteReqMsg := halproto.FlowMonitorRuleDeleteRequestMsg{}
		for _, ipFmRule := range ipFmRuleList {
			flowRule := types.FlowMonitorRuleSpec{
				SourceIP:     ipFmRule.SrcIPString,
				DestIP:       ipFmRule.DestIPString,
				Protocol:     uint32(ipFmRule.AppPortObj.Ipproto),
				SourceL4Port: uint32(ipFmRule.AppPortObj.L4port),
				DestL4Port:   uint32(ipFmRule.AppPortObj.L4port),
				VrfID:        vrfID,
			}
			ruleID, allocated, err := tsa.allocateFlowMonitorRuleID(flowRule)
			if err != nil {
				return nil, nil, err
			}
			if allocated {
				//Should not be allcated; RuleID should have already existed...
				log.Errorf("Missing flow monitor rule in local DB")
				continue
			}
			deleteProtoObj = false
			updateProtoObj = false
			contains, _ := checkIDContainment(updateFmRuleIDs, ruleID)
			if !contains {
				contains, _ := checkIDContainment(deleteFmRuleIDs, ruleID)
				if !contains {
					continue
				}
				deleteProtoObj = true
			} else {
				updateProtoObj = true
			}
			if updateProtoObj {
				//Build list of mirror sessions proto obj matching list
				//of mirrorSession the rule is part of
				flowRuleObj := tsa.DB.FlowMonitorRuleIDToObj[ruleID]
				var msIDs []*halproto.MirrorSessionKeyHandle
				for _, mid := range flowRuleObj.MirrorSessionIDs {
					midObj := &halproto.MirrorSessionKeyHandle{
						KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
							MirrorsessionId: mid,
						},
					}
					msIDs = append(msIDs, midObj)
				}
				srcAddrObj := utils.BuildIPAddrObjProtoObj(ipFmRule.SrcIPObj)
				destAddrObj := utils.BuildIPAddrObjProtoObj(ipFmRule.DestIPObj)
				appMatchObj := utils.BuildAppMatchInfoObj(ipFmRule.AppPortObj)
				flowRuleSpec := halproto.FlowMonitorRuleSpec{
					KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
						KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
							FlowmonitorruleId: ruleID,
						},
					},
					VrfKeyHandle: &halproto.VrfKeyHandle{
						KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
							VrfId: vrfID,
						},
					},
					Match: &halproto.RuleMatch{
						SrcAddress: []*halproto.IPAddressObj{srcAddrObj},
						DstAddress: []*halproto.IPAddressObj{destAddrObj},
						Protocol:   halproto.IPProtocol_value[halproto.IPProtocol(ipFmRule.AppPortObj.Ipproto).String()],
						AppMatch:   appMatchObj,
					},
					Action: &halproto.MonitorAction{
						MsKeyHandle: msIDs,
						Action:      []halproto.RuleAction{halproto.RuleAction_MIRROR},
					},
				}
				updateReqMsg.Request = append(updateReqMsg.Request, &flowRuleSpec)
			} else if deleteProtoObj {
				flowRule := halproto.FlowMonitorRuleDeleteRequest{
					KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
						KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
							FlowmonitorruleId: ruleID,
						},
					},
					VrfKeyHandle: &halproto.VrfKeyHandle{
						KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
							VrfId: vrfID,
						},
					},
				}
				deleteReqMsg.Request = append(deleteReqMsg.Request, &flowRule)
			}
		}

		for _, macRule := range macFmRuleList {
			flowRule := types.FlowMonitorRuleSpec{
				SourceIP:  "",
				DestIP:    "",
				SourceMac: macRule.SrcMAC,
				DestMac:   macRule.DestMAC,
				//EtherType: 0,
				// ??? : Does it make sense to use proto, l4ports when src/dest MAC based matching is used ?
				Protocol:     uint32(macRule.AppPortObj.Ipproto),
				SourceL4Port: uint32(macRule.AppPortObj.L4port),
				DestL4Port:   uint32(macRule.AppPortObj.L4port),
				VrfID:        vrfID,
			}
			ruleID, allocated, err := tsa.allocateFlowMonitorRuleID(flowRule)
			if err != nil {
				return nil, nil, err
			}
			if allocated {
				//Should not be allcated; RuleID should have already existed...
				log.Errorf("Missing flow monitor rule in local DB")
				continue
			}
			deleteProtoObj = false
			updateProtoObj = false
			contains, _ := checkIDContainment(updateFmRuleIDs, ruleID)
			if !contains {
				contains, _ := checkIDContainment(deleteFmRuleIDs, ruleID)
				if !contains {
					continue
				}
				deleteProtoObj = true
			} else {
				updateProtoObj = true
			}
			if updateProtoObj {
				//Build list of mirror sessions proto obj matching list
				//of mirrorSession the rule is part of
				flowRuleObj := tsa.DB.FlowMonitorRuleIDToObj[ruleID]
				var msIDs []*halproto.MirrorSessionKeyHandle
				for _, mid := range flowRuleObj.MirrorSessionIDs {
					midObj := &halproto.MirrorSessionKeyHandle{
						KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
							MirrorsessionId: mid,
						},
					}
					msIDs = append(msIDs, midObj)
				}

				appMatchObj := utils.BuildAppMatchInfoObj(macRule.AppPortObj)
				flowRuleSpec := halproto.FlowMonitorRuleSpec{
					KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
						KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
							FlowmonitorruleId: ruleID,
						},
					},
					VrfKeyHandle: &halproto.VrfKeyHandle{
						KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
							VrfId: vrfID,
						},
					},
					Match: &halproto.RuleMatch{
						SrcMacAddress: []uint64{macRule.SrcMAC},
						DstMacAddress: []uint64{macRule.DestMAC},
						Protocol:      halproto.IPProtocol_value[halproto.IPProtocol(macRule.AppPortObj.Ipproto).String()],
						AppMatch:      appMatchObj,
					},
					Action: &halproto.MonitorAction{
						MsKeyHandle: msIDs,
						Action:      []halproto.RuleAction{halproto.RuleAction_MIRROR},
					},
				}
				updateReqMsg.Request = append(updateReqMsg.Request, &flowRuleSpec)
			} else if deleteProtoObj {
				flowRule := halproto.FlowMonitorRuleDeleteRequest{
					KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
						KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
							FlowmonitorruleId: ruleID,
						},
					},
					VrfKeyHandle: &halproto.VrfKeyHandle{
						KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
							VrfId: vrfID,
						},
					},
				}
				deleteReqMsg.Request = append(deleteReqMsg.Request, &flowRule)
			}
		}
		if len(updateReqMsg.Request) > 0 {
			updateReqMsgList = append(updateReqMsgList, &updateReqMsg)
		}
		if len(deleteReqMsg.Request) > 0 {
			deleteReqMsgList = append(deleteReqMsgList, &deleteReqMsg)
		}
	}
	return updateReqMsgList, deleteReqMsgList, nil
}

func (tsa *Tagent) createHALFlowMonitorRulesProtoObj(mirrorSession *netproto.MirrorSession, mirrorSessID uint64) ([]*halproto.FlowMonitorRuleRequestMsg, []uint64, error) {

	var ReqMsgList []*halproto.FlowMonitorRuleRequestMsg
	var newRuleIDs []uint64

	vrfID, err := getVrfID(mirrorSession)
	if err != nil {
		log.Errorf("mirror session tenant is invalid")
		return nil, nil, ErrInvalidMirrorSpec
	}

	for _, rule := range mirrorSession.Spec.MatchRules {
		srcIPs, destIPs, srcMACs, destMACs, appPorts, srcIPStrings, destIPStrings := utils.ExpandCompositeMatchRule(mirrorSession.ObjectMeta, &rule, nAgent.FindEndpoint)
		// Create protobuf requestMsgs on cross product of
		//  - srcIPs, destIPs, Apps
		//  - srcMACs, destMACs, Apps
		flowMonitorRuleSpecified := false
		ipFmRuleList, err := utils.CreateIPAddrCrossProductRuleList(srcIPs, destIPs, appPorts, srcIPStrings, destIPStrings)
		if err == nil {
			flowMonitorRuleSpecified = true
		}

		macFmRuleList, err := utils.CreateMACAddrCrossProductRuleList(srcMACs, destMACs, appPorts)
		if err == nil {
			flowMonitorRuleSpecified = true
		}

		//TODO: Fold in ether-type in monitor rule

		if !flowMonitorRuleSpecified {
			log.Errorf("Match Rules specified with only AppPort Selector without IP/MAC")
			return nil, nil, utils.ErrInvalidFlowMonitorRule
		}

		ReqMsg := halproto.FlowMonitorRuleRequestMsg{}
		for _, ipFmRule := range ipFmRuleList {
			flowRule := types.FlowMonitorRuleSpec{
				SourceIP:     ipFmRule.SrcIPString,
				DestIP:       ipFmRule.DestIPString,
				Protocol:     uint32(ipFmRule.AppPortObj.Ipproto),
				SourceL4Port: uint32(ipFmRule.AppPortObj.L4port),
				DestL4Port:   uint32(ipFmRule.AppPortObj.L4port),
				VrfID:        vrfID,
			}
			ruleID, allocated, err := tsa.allocateFlowMonitorRuleID(flowRule)
			if err != nil {
				return nil, nil, err
			}
			if allocated {
				newRuleIDs = append(newRuleIDs, ruleID)
				flowRuleObj := types.FlowMonitorObj{
					TypeMeta: api.TypeMeta{Kind: "FlowMonitorRuleObject"},
					ObjectMeta: api.ObjectMeta{
						Tenant:    mirrorSession.ObjectMeta.Tenant,
						Namespace: "",
						Name:      "",
					},
					Spec:   flowRule,
					RuleID: ruleID,
				}
				flowRuleObj.MirrorSessionIDs = append(flowRuleObj.MirrorSessionIDs, mirrorSessID)
				tsa.DB.FlowMonitorRuleIDToObj[ruleID] = flowRuleObj
			} else {
				flowRuleObj := tsa.DB.FlowMonitorRuleIDToObj[ruleID]
				// Rule already existed. Check if mirrorSession is already associated with the rule.
				contains, _ := checkIDContainment(flowRuleObj.MirrorSessionIDs, mirrorSessID)
				if !contains {
					flowRuleObj.MirrorSessionIDs = append(flowRuleObj.MirrorSessionIDs, mirrorSessID)
				}
			}
			//List of mirror sessions proto obj matching list of mirrorSession the rule is part of
			flowRuleObj := tsa.DB.FlowMonitorRuleIDToObj[ruleID]
			var msIDs []*halproto.MirrorSessionKeyHandle
			for _, mid := range flowRuleObj.MirrorSessionIDs {
				midObj := &halproto.MirrorSessionKeyHandle{
					KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
						MirrorsessionId: mid,
					},
				}
				msIDs = append(msIDs, midObj)
			}
			srcAddrObj := utils.BuildIPAddrObjProtoObj(ipFmRule.SrcIPObj)
			destAddrObj := utils.BuildIPAddrObjProtoObj(ipFmRule.DestIPObj)
			appMatchObj := utils.BuildAppMatchInfoObj(ipFmRule.AppPortObj)
			flowRuleSpec := halproto.FlowMonitorRuleSpec{
				KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
					KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
						FlowmonitorruleId: ruleID,
					},
				},
				VrfKeyHandle: &halproto.VrfKeyHandle{
					KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
						VrfId: vrfID,
					},
				},
				Match: &halproto.RuleMatch{
					SrcAddress: []*halproto.IPAddressObj{srcAddrObj},
					DstAddress: []*halproto.IPAddressObj{destAddrObj},
					Protocol:   halproto.IPProtocol_value[halproto.IPProtocol(ipFmRule.AppPortObj.Ipproto).String()],
					AppMatch:   appMatchObj,
				},
				Action: &halproto.MonitorAction{
					MsKeyHandle: msIDs,
					Action:      []halproto.RuleAction{halproto.RuleAction_MIRROR},
				},
			}
			ReqMsg.Request = append(ReqMsg.Request, &flowRuleSpec)
		}

		for _, macRule := range macFmRuleList {
			flowRule := types.FlowMonitorRuleSpec{
				SourceIP:  "",
				DestIP:    "",
				SourceMac: macRule.SrcMAC,
				DestMac:   macRule.DestMAC,
				//EtherType: 0,
				// ??? : Does it make sense to use proto, l4ports when src/dest MAC based matching is used ?
				Protocol:     uint32(macRule.AppPortObj.Ipproto),
				SourceL4Port: uint32(macRule.AppPortObj.L4port),
				DestL4Port:   uint32(macRule.AppPortObj.L4port),
				VrfID:        vrfID,
			}
			ruleID, allocated, err := tsa.allocateFlowMonitorRuleID(flowRule)
			if err != nil {
				return nil, nil, err
			}
			if allocated {
				newRuleIDs = append(newRuleIDs, ruleID)
				flowRuleObj := types.FlowMonitorObj{
					TypeMeta: api.TypeMeta{Kind: "FlowMonitorRuleObject"},
					ObjectMeta: api.ObjectMeta{
						Tenant:    mirrorSession.ObjectMeta.Tenant,
						Namespace: "",
						Name:      "",
					},
					Spec:   flowRule,
					RuleID: ruleID,
				}
				flowRuleObj.MirrorSessionIDs = append(flowRuleObj.MirrorSessionIDs, mirrorSessID)
				tsa.DB.FlowMonitorRuleIDToObj[ruleID] = flowRuleObj
			} else {
				flowRuleObj := tsa.DB.FlowMonitorRuleIDToObj[ruleID]
				// Rule already existed. Check if mirrorSession is already associated with the rule.
				contains, _ := checkIDContainment(flowRuleObj.MirrorSessionIDs, mirrorSessID)
				if !contains {
					flowRuleObj.MirrorSessionIDs = append(flowRuleObj.MirrorSessionIDs, mirrorSessID)
				}
			}
			//List of mirror sessions proto obj matching list of mirrorSession the rule is part of
			flowRuleObj := tsa.DB.FlowMonitorRuleIDToObj[ruleID]
			var msIDs []*halproto.MirrorSessionKeyHandle
			for _, mid := range flowRuleObj.MirrorSessionIDs {
				midObj := &halproto.MirrorSessionKeyHandle{
					KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
						MirrorsessionId: mid,
					},
				}
				msIDs = append(msIDs, midObj)
			}

			appMatchObj := utils.BuildAppMatchInfoObj(macRule.AppPortObj)
			flowRuleSpec := halproto.FlowMonitorRuleSpec{
				KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
					KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
						FlowmonitorruleId: ruleID,
					},
				},
				VrfKeyHandle: &halproto.VrfKeyHandle{
					KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
						VrfId: vrfID,
					},
				},
				Match: &halproto.RuleMatch{
					SrcMacAddress: []uint64{macRule.SrcMAC},
					DstMacAddress: []uint64{macRule.DestMAC},
					Protocol:      halproto.IPProtocol_value[halproto.IPProtocol(macRule.AppPortObj.Ipproto).String()],
					AppMatch:      appMatchObj,
				},
				Action: &halproto.MonitorAction{
					MsKeyHandle: msIDs,
					Action:      []halproto.RuleAction{halproto.RuleAction_MIRROR},
				},
			}
			ReqMsg.Request = append(ReqMsg.Request, &flowRuleSpec)
		}

		// A list of rules are prepared using L3 addrs and L4 ports
		// A list of rules are prepared using L2 addrs and L4 ports
		// Note: There is no rule with both L2 and L3 addr in a single rule...needed ???

		// Filter/Drop rules
		// For flow match rule based sessions, its only possible to support
		// mirroring on ruleMatch and if pkt matching monitoring rule is dropped for any reason.
		// It is not possible to apply specific dropreason filter that also matched flow monitor rule..
		//filterAllPkts := false
		//for _, filter := range mirrorSession.Spec.PacketFilters {
		//	if filter == "ALL_PKTS" {
		//		filterAllPkts = true
		//		break
		//	}
		//	if filterAllPkts {
		//	}
		//}
		ReqMsgList = append(ReqMsgList, &ReqMsg)
	}
	log.Debugf("FMRule ReqMsg List %v  NewRuleIDs %v", ReqMsgList, newRuleIDs)
	return ReqMsgList, newRuleIDs, nil
}

type mirrorProtoObjs struct {
	FlowRuleProtoObjs      []*halproto.FlowMonitorRuleRequestMsg
	DropRuleProtoObjs      []*halproto.DropMonitorRuleRequestMsg
	MirrorSessionProtoObjs *halproto.MirrorSessionRequestMsg
	NewDropRuleIDs         []uint64
	NewFlowRuleIDs         []uint64
}

// CreatePacketCaptureSessionProtoObjects creates all proto objects needed to create mirror session.
func (tsa *Tagent) createPacketCaptureSessionProtoObjs(mirrorSession *netproto.MirrorSession) (*mirrorProtoObjs, error) {
	var flowRuleProtoObjs []*halproto.FlowMonitorRuleRequestMsg
	var dropRuleProtoObjs []*halproto.DropMonitorRuleRequestMsg
	var mirrorSessionProtoObjs *halproto.MirrorSessionRequestMsg
	var err error
	var mirrorObjs mirrorProtoObjs
	// Check if flow rules or drop rules needs to be processed.
	dropMonitor := true
	if len(mirrorSession.Spec.MatchRules) > 0 {
		dropMonitor = false
	}
	if err := utils.ValidateMatchRules(mirrorSession.ObjectMeta, mirrorSession.Spec.MatchRules, nAgent.FindEndpoint); err != nil && !dropMonitor {
		// Rule santy check failed.
		log.Errorf("Neither drop rule nor match selector rules specified in mirror spec, err:%s", err)
		return nil, ErrInvalidMirrorSpec
	}

	createMirrorSession := true
	key := objectKey(mirrorSession.ObjectMeta, mirrorSession.TypeMeta)
	sessID := tsa.getMirrorSessionID(key)
	if sessID < halMaxMirrorSession {
		if tsa.DB.MirrorSessionIDToObj[sessID].Created &&
			tsa.DB.MirrorSessionIDToObj[sessID].Handle != 0 {
			// mirror create already posted to HAL.
			createMirrorSession = false
		}
	} else {
		sessID = tsa.allocateMirrorSessionID(key)
		if sessID > halMaxMirrorSession {
			log.Errorf("Attempting to configure over the limit of Maximum allowed mirror session by hardware %v", key)
			return nil, ErrMirrorSpecResource
		}
	}

	//Mirror Session  proto objs
	mirrorSessionProtoObjs, err = tsa.createHALMirrorSessionProtoObj(mirrorSession, sessID)
	if err != nil {
		log.Errorf("Could not create mirror session object")
		return nil, err
	}
	// FlowMonitor rules
	var newDropRuleIDs, newFlowRuleIDs []uint64
	flowRuleProtoObjs, newFlowRuleIDs, err = tsa.createHALFlowMonitorRulesProtoObj(mirrorSession, sessID)
	if err != nil {
		if err != nil {
			log.Errorf("Could not create mirror flow monitor rule object")
			return nil, err
		}
	}
	// Drop monitor rules TODO uncomment when drop monitoring is supported fully
	//if dropMonitor {
	//	dropRuleProtoObjs, newDropRuleIDs, err = tsa.createHALDropMonitorRulesProtoObj(mirrorSession, sessID)
	//	if err != nil {
	//		log.Errorf("Could not create mirror drop monitor rule object")
	//		return nil, err
	//	}
	//}
	if createMirrorSession {
		tsa.DB.MirrorSessionIDToObj[sessID] = types.MirrorSessionObj{
			TypeMeta:   api.TypeMeta{Kind: "MonitorSessionObject"},
			ObjectMeta: mirrorSession.ObjectMeta,
			Created:    false,
			MirrorID:   sessID,
			Handle:     0,
			//List of flow/drop ruleIDs that are part of mirrorSession
			//are added to mirrorsession obj once datapath is programmed.
			//Also Created flag is set to True and Handle is programmed
			//datapath code.
		}
	}
	mirrorObjs.FlowRuleProtoObjs = flowRuleProtoObjs
	mirrorObjs.DropRuleProtoObjs = dropRuleProtoObjs
	mirrorObjs.MirrorSessionProtoObjs = mirrorSessionProtoObjs
	mirrorObjs.NewDropRuleIDs = newDropRuleIDs
	mirrorObjs.NewFlowRuleIDs = newFlowRuleIDs
	return &mirrorObjs, err
}

func (tsa *Tagent) deleteRuleObjsFromDB(deleteFmRuleIDs, deleteDropRuleIDs []uint64) {
	// list of deleted FmRules from datapath should also be removed from emstore.
	for _, f := range deleteFmRuleIDs {
		flowRuleObj := tsa.DB.FlowMonitorRuleIDToObj[f]
		tsa.Store.Delete(&flowRuleObj)
	}
	for _, d := range deleteDropRuleIDs {
		dropRuleObj := tsa.DB.DropRuleIDToObj[d]
		tsa.Store.Delete(&dropRuleObj)
	}
}

func (tsa *Tagent) storePacketCaptureSessionInDB(pcSession *netproto.MirrorSession, mirrorProtoObjs *mirrorProtoObjs) error {
	var err error

	// Emstore new rules (new flowmonitor rule and new drop rules) and mirror
	key := objectKey(pcSession.ObjectMeta, pcSession.TypeMeta)
	mirrorSessID := tsa.DB.MirrorSessionNameToID[key]
	mirrorSessObj := tsa.DB.MirrorSessionIDToObj[mirrorSessID]

	for _, dropRuleID := range mirrorSessObj.DropMonitorRuleIDs {
		dropRuleObj := tsa.DB.DropRuleIDToObj[dropRuleID]
		tsa.Store.Write(&dropRuleObj)
	}
	for _, flowRuleID := range mirrorSessObj.FlowMonitorRuleIDs {
		flowRuleObj := tsa.DB.FlowMonitorRuleIDToObj[flowRuleID]
		tsa.Store.Write(&flowRuleObj)
	}
	err = tsa.Store.Write(&mirrorSessObj)
	if err == nil {
		err = tsa.Store.Write(pcSession)
		if err == nil {
			log.Debugf("Storing mirror session proto using key %s", key)
			tsa.DB.MirrorSessionDB[key] = pcSession
		}
	}
	return err
}

func (tsa *Tagent) deleteModifyMirrorSessionRules(pcSession *netproto.MirrorSession, purgedFlowRuleIDs, purgedDropRuleIDs []uint64) ([]uint64, []uint64, error) {
	var deleteFmRuleIDs, deleteDropRuleIDs []uint64
	var updateFmRuleIDs, updateDropRuleIDs []uint64

	key := objectKey(pcSession.ObjectMeta, pcSession.TypeMeta)
	mirrorSessID := tsa.getMirrorSessionID(key)

	for _, f := range purgedFlowRuleIDs {
		flowRuleObj, ok := tsa.DB.FlowMonitorRuleIDToObj[f]
		if ok {
			if len(flowRuleObj.MirrorSessionIDs) > 1 {
				// recreate rule with modified list of mirrorSessionIDs
				present, position := checkIDContainment(flowRuleObj.MirrorSessionIDs, mirrorSessID)
				if !present {
					log.Errorf("Internal error. MirrorSession ID missing in lookup table")
					return nil, nil, ErrMsInternal
				}
				// To delete, just swap element to be deleted with last element and
				// recreate array with one less element
				flowRuleObj.MirrorSessionIDs[position] = flowRuleObj.MirrorSessionIDs[len(flowRuleObj.MirrorSessionIDs)-1]
				flowRuleObj.MirrorSessionIDs = flowRuleObj.MirrorSessionIDs[:len(flowRuleObj.MirrorSessionIDs)-1]
				updateFmRuleIDs = append(updateFmRuleIDs, f)
			} else {
				if flowRuleObj.MirrorSessionIDs[0] != mirrorSessID {
					log.Errorf("Internal error. MirrorSession ID mismatch")
					return nil, nil, ErrMsInternal
				}
				deleteFmRuleIDs = append(deleteFmRuleIDs, f)
			}
		}
	}
	for _, d := range purgedDropRuleIDs {
		dropRuleObj, ok := tsa.DB.DropRuleIDToObj[d]
		if ok {
			if len(dropRuleObj.MirrorSessionIDs) > 1 {
				// recreate rule with modified list of mirrorSessionIDs
				present, position := checkIDContainment(dropRuleObj.MirrorSessionIDs, mirrorSessID)
				if !present {
					log.Errorf("Internal error. MirrorSession ID missing in lookup table")
					return nil, nil, ErrMsInternal
				}
				// To delete, just swap element to be deleted with last element and
				// recreate array with one less element
				dropRuleObj.MirrorSessionIDs[position] = dropRuleObj.MirrorSessionIDs[len(dropRuleObj.MirrorSessionIDs)-1]
				dropRuleObj.MirrorSessionIDs = dropRuleObj.MirrorSessionIDs[:len(dropRuleObj.MirrorSessionIDs)-1]
				updateDropRuleIDs = append(updateDropRuleIDs, d)
			} else {
				if dropRuleObj.MirrorSessionIDs[0] != mirrorSessID {
					log.Errorf("Internal error. MirrorSession ID mismatch")
					return nil, nil, ErrMsInternal
				}
				deleteDropRuleIDs = append(deleteDropRuleIDs, d)
			}
		}
	}
	// Using oldMirrorSessionSpec, create list of FlowRuleProtoObjs that match FlowRuleID.
	// The new list of FlowRuleProtoObjs will not contain mirrorSessionID if the rule is part
	// of more than one mirrorSession. If old rule is the not part of any mirrorSession after
	// update, that rule will be deleted in datapath.
	oldMirrorSession := tsa.DB.MirrorSessionDB[key]
	updateFmProtoObjs, deleteFmProtoObjs, err1 := tsa.createFlowMonitorRuleIDMatchingHALProtoObj(
		oldMirrorSession, mirrorSessID, updateFmRuleIDs, deleteFmRuleIDs)
	// Using oldMirrorSessionSpec, create list of DropRuleProtoObjs that match DropRuleID.
	// The new list of DropRuleProtoObjs will not contain mirrorSessionID if the rule is part
	// of more than one mirrorSession. If old rule is the not part of any mirrorSession after
	// update, that rule will be deleted in datapath.
	updateDropProtoObjs, deleteDropProtoObjs, err2 := tsa.createDropMonitorRuleIDMatchingHALProtoObj(
		oldMirrorSession, mirrorSessID, updateDropRuleIDs, deleteDropRuleIDs)
	// Program flowRules in dataplane with modified list of mirrorIDs
	// Delete flowRules in dataplane that are dropped in update mirrorSession operation.
	if len(updateFmRuleIDs) > 0 && err1 == nil {
		tsa.Datapath.UpdateFlowMonitorRule(updateFmProtoObjs)
	}
	if len(deleteFmRuleIDs) > 0 && err1 == nil {
		tsa.Datapath.DeleteFlowMonitorRule(deleteFmProtoObjs)
	}
	// Program DropRules in dataplane with modified list of mirrorIDs
	// Delete DropRules in dataplane that are dropped in update mirrorSession operation.
	if len(updateDropRuleIDs) > 0 && err2 == nil {
		tsa.Datapath.UpdateDropMonitorRule(updateDropProtoObjs)
	}
	if len(deleteDropRuleIDs) > 0 && err2 == nil {
		tsa.Datapath.DeleteDropMonitorRule(deleteDropProtoObjs)
	}

	return deleteFmRuleIDs, deleteDropRuleIDs, nil
}

func (tsa *Tagent) createUpdatePacketCaptureSession(pcSession, oldMs *netproto.MirrorSession, update bool) error {
	// Create lateral objects here
	if update && oldMs != nil {
		for _, oldCollectors := range oldMs.Spec.Collectors {
			// Intercept the update call and issue a lateral object deletion.
			err := nAgent.DeleteLateralNetAgentObjects(oldMs.GetKey(), getMgmtIP(), oldCollectors.ExportCfg.Destination, true)
			if err != nil {
				log.Errorf("Failed to trigged a delete of lateral objects during the update path. Err: %v", err)
			}
		}
	}
	for _, mirrorCollector := range pcSession.Spec.Collectors {
		err := nAgent.CreateLateralNetAgentObjects(pcSession.GetKey(), getMgmtIP(), mirrorCollector.ExportCfg.Destination, true)
		if err != nil {
			log.Errorf("Failed to create lateral objects. Err: %v", err)
			return fmt.Errorf("failed to create lateral objects. Err: %v", err)
		}
	}

	var deleteFmRuleIDs, deleteDropRuleIDs []uint64
	var err error

	vrfID, err := getVrfID(pcSession)
	if err != nil {
		log.Errorf("mirror session tenant is invalid")
		return ErrInvalidMirrorSpec
	}
	tsa.Lock()
	defer tsa.Unlock()
	mirrorProtoObjs, err := tsa.createPacketCaptureSessionProtoObjs(pcSession)
	if err == nil {
		key := objectKey(pcSession.ObjectMeta, pcSession.TypeMeta)
		if update {
			// mirror session update moved from disable --> enable
			// create all match selectors and drop rules in both DB and HAL.
			// There is no purge list to process.
			// mirror session update moved from enable --> disable
			// Delete all match selector rules in both DB and HAL. Keep Mirror Obj

			// Enabled mirrorSession updated with spec changes in enable mode again.
			// Program datapath with updated packet capture session.
			// Updated spec can drop match rules. Modify/Delete
			// match rules if update spec is programmed on datapath.
			// Upaate/delete of prior/old ruleIDs that are
			// associated with mirrorSession are done after datapath is
			// programmed successfully.
			purgedFlowRuleIDs, purgedDropRuleIDs, err := tsa.Datapath.UpdatePacketCaptureSession(key, vrfID, &tsa.DB,
				mirrorProtoObjs.MirrorSessionProtoObjs, mirrorProtoObjs.FlowRuleProtoObjs, mirrorProtoObjs.DropRuleProtoObjs)
			if err == nil {
				deleteFmRuleIDs, deleteDropRuleIDs, err = tsa.deleteModifyMirrorSessionRules(pcSession,
					purgedFlowRuleIDs, purgedDropRuleIDs)
			}
		} else {
			// create with  enable
			// create with  disable
			//      Just create mirrorSession Obj in DB
			//      Do not create rules/collectorSpec in both DB and HAL
			err = tsa.Datapath.CreatePacketCaptureSession(key, vrfID, &tsa.DB, mirrorProtoObjs.MirrorSessionProtoObjs,
				mirrorProtoObjs.FlowRuleProtoObjs, mirrorProtoObjs.DropRuleProtoObjs)
		}
		if err == nil {
			// Mirror Session has been created or updated in datapath. Also all rules of the mirror
			// sessions are programmed in datapath. For agent restart handling, store mirror session
			// related rules in DB
			err = tsa.storePacketCaptureSessionInDB(pcSession, mirrorProtoObjs)
		}
	}
	if err != nil {
		log.Errorf("Packet capture session create or update request errored. Err: %v", err)
		if mirrorProtoObjs != nil {
			//clean up new rules added to DB maps
			for _, d := range mirrorProtoObjs.NewDropRuleIDs {
				//clean up locally maintained map for this rule.
				dropMonitorObj := tsa.DB.DropRuleIDToObj[d]
				delete(tsa.DB.DropRuleToID, dropMonitorObj.Spec.DropReasons)
				delete(tsa.DB.DropRuleIDToObj, d)
			}
			for _, f := range mirrorProtoObjs.NewFlowRuleIDs {
				flowMonitorObj := tsa.DB.FlowMonitorRuleIDToObj[f]
				delete(tsa.DB.FlowMonitorRuleToID, flowMonitorObj.Spec)
				delete(tsa.DB.FlowMonitorRuleIDToObj, f)
			}
		}
	}
	//As part of mirrorSpec update, if any rules are deleted, remove them from local map/DB and emstore.
	if err == nil {
		tsa.deleteRuleObjsFromDB(deleteFmRuleIDs, deleteDropRuleIDs)
		for _, f := range deleteFmRuleIDs {
			flowRuleObj := tsa.DB.FlowMonitorRuleIDToObj[f]
			delete(tsa.DB.FlowMonitorRuleToID, flowRuleObj.Spec)
			delete(tsa.DB.FlowMonitorRuleIDToObj, f)
		}
		for _, d := range deleteDropRuleIDs {
			dropRuleObj := tsa.DB.DropRuleIDToObj[d]
			delete(tsa.DB.DropRuleToID, dropRuleObj.Spec.DropReasons)
			delete(tsa.DB.DropRuleIDToObj, d)
		}
	}
	log.Debugf("Complete packet capture session create/update... %v", pcSession.Name)
	return err
}

func (tsa *Tagent) deletePacketCaptureSession(pcSession *netproto.MirrorSession) error {
	for _, mirrorCollector := range pcSession.Spec.Collectors {
		err := nAgent.DeleteLateralNetAgentObjects(pcSession.GetKey(), getMgmtIP(), mirrorCollector.ExportCfg.Destination, true)
		if err != nil {
			log.Errorf("Failed to delete lateral objects. Err: %v", err)
			return fmt.Errorf("failed to delete lateral objects. Err: %v", err)
		}
	}
	key := objectKey(pcSession.ObjectMeta, pcSession.TypeMeta)
	tsa.Lock()
	defer tsa.Unlock()
	_, ok := tsa.DB.MirrorSessionDB[key]
	if !ok {
		log.Errorf("Internal error. MirrorSession lookup failure, %s", key)
		return ErrMsInternal
	}
	mirrorSessID, ok := tsa.DB.MirrorSessionNameToID[key]
	if !ok {
		log.Errorf("Internal error. MirrorSession lookup failure, %s", key)
		return ErrMsInternal
	}
	mirrorSessObj, ok := tsa.DB.MirrorSessionIDToObj[mirrorSessID]
	if !ok {
		log.Errorf("Internal error. MirrorSession lookup failure, %s", key)
		return ErrMsInternal
	}
	//delete rules of the mirrorSession from the datapath. Modify match rules
	//that are part of mirrorSession under delete as well other mirrorSession
	//to exclude mirrorSession to be deleted.
	deleteFmRuleIDs, deleteDropRuleIDs, err := tsa.deleteModifyMirrorSessionRules(pcSession, mirrorSessObj.FlowMonitorRuleIDs, mirrorSessObj.DropMonitorRuleIDs)
	//All rules that were only part of the mirrorSession under delete should be removed from local map/DB and emstore.
	if err == nil {
		tsa.deleteRuleObjsFromDB(deleteFmRuleIDs, deleteDropRuleIDs)
		for _, f := range deleteFmRuleIDs {
			flowRuleObj := tsa.DB.FlowMonitorRuleIDToObj[f]
			delete(tsa.DB.FlowMonitorRuleToID, flowRuleObj.Spec)
			delete(tsa.DB.FlowMonitorRuleIDToObj, f)
		}
		for _, d := range deleteDropRuleIDs {
			dropRuleObj := tsa.DB.DropRuleIDToObj[d]
			delete(tsa.DB.DropRuleToID, dropRuleObj.Spec.DropReasons)
			delete(tsa.DB.DropRuleIDToObj, d)
		}
		mirrorDeleteReq := halproto.MirrorSessionDeleteRequest{
			KeyOrHandle: &halproto.MirrorSessionKeyHandle{
				KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
					MirrorsessionId: mirrorSessID,
				},
			},
		}
		ReqMsg := halproto.MirrorSessionDeleteRequestMsg{
			Request: []*halproto.MirrorSessionDeleteRequest{&mirrorDeleteReq},
		}
		err = tsa.Datapath.DeletePacketCaptureSession(key, &ReqMsg)
		//Delete mirrorSession from local DB and set mirrorID (0--7) as unused.
		tsa.deleteMirrorSession(key)
	} else {
		log.Errorf("mirror session %v not cleaned up", key)
	}
	log.Debugf("Complete packet capture session delete... %v", pcSession.Name)
	return err
}

// CreateMirrorSession creates mirror session to enable packet capture
func (tsa *Tagent) CreateMirrorSession(pcSession *netproto.MirrorSession) error {
	log.Debugf("Processing packet capture session create... {%+v}", pcSession.Name)
	if pcSession.Name == "" {
		log.Errorf("mirror session name is empty")
		return ErrInvalidMirrorSpec
	}
	oldMs, err := tsa.findMirrorSession(pcSession.ObjectMeta)
	if err == nil {
		// MirrorSession already created. return
		if !proto.Equal(oldMs, pcSession) {
			log.Errorf("MirrorSession %+v already exists", oldMs)
			return errors.New("MirrorSession already exists")
		}
		log.Infof("Received duplicate mirror session create {%+v}", pcSession.Name)
		return nil
	}
	//if !pcSession.Spec.Enable {
	// Session created in disable mode. Just store the mirrorSession in DB
	// At later time, update mirrorsession will trigger setting up mirror
	// session in HW. Update mirrorSession object will also carry required
	// drop/monitor rules.
	key := objectKey(pcSession.ObjectMeta, pcSession.TypeMeta)
	tsa.Lock()
	tsa.DB.MirrorSessionDB[key] = pcSession
	tsa.Unlock()
	//return nil
	//}
	return tsa.createUpdatePacketCaptureSession(pcSession, oldMs, false)
}

// UpdateMirrorSession updates mirror session
func (tsa *Tagent) UpdateMirrorSession(pcSession *netproto.MirrorSession) error {
	log.Debugf("Processing packet capture session update... %+v", pcSession.Name)
	if pcSession.Name == "" {
		log.Errorf("mirror session name is empty")
		return ErrInvalidMirrorSpec
	}
	oldMs, err := tsa.findMirrorSession(pcSession.ObjectMeta)
	if err == nil {
		if reflect.DeepEqual(oldMs.Spec, pcSession.Spec) {
			log.Infof("no change in mirrorsession %+v", oldMs.Name)
			return nil
		}
	} else {
		log.Errorf("MirrorSession %v does not exist to update", pcSession.Name)
		return fmt.Errorf("mirrorsession %v does not exist", pcSession.Name)
	}
	//if pcSession.Spec.Enable {
	return tsa.createUpdatePacketCaptureSession(pcSession, oldMs, true)
	//}
	//return nil
}

// DeleteMirrorSession deletes packet capture session.
func (tsa *Tagent) DeleteMirrorSession(pcSession *netproto.MirrorSession) error {
	log.Debugf("Processing packet capture session delete... %v", pcSession.Name)
	existingPcSession, err := tsa.findMirrorSession(pcSession.ObjectMeta)
	if err != nil {
		log.Errorf("MirrorSession %v does not exist to delete it", pcSession.Name)
		return fmt.Errorf("mirror session %v does not exist", pcSession.Name)
	}
	return tsa.deletePacketCaptureSession(existingPcSession)
}

// GetMirrorSession gets a mirror session
func (tsa *Tagent) GetMirrorSession(pcSession *netproto.MirrorSession) *netproto.MirrorSession {
	log.Debugf("Processing packet capture session get... %v", pcSession)
	if pcSession.Name == "" {
		log.Errorf("mirror session name is empty")
		return nil
	}
	oldMs, err := tsa.findMirrorSession(pcSession.ObjectMeta)
	if err != nil {
		log.Errorf("mirror session does not exist")
		return nil
	}
	return oldMs
}

// Debug gets debug info
func (tsa *Tagent) Debug(w http.ResponseWriter, r *http.Request) {
	type dbgInfo struct {
		MirrorSessionDB       map[string]*netproto.MirrorSession
		MirrorSessionNameToID map[string]uint64
		MirrorSessionIDToObj  map[uint64]types.MirrorSessionObj
		// key: types.FlowMonitorRuleSpec
		FlowMonitorRuleToID    map[string]uint64
		FlowMonitorRuleIDToObj map[uint64]types.FlowMonitorObj
		// key: halproto.DropReasons
		DropRuleToID       map[string]uint64
		DropRuleIDToObj    map[uint64]types.DropMonitorObj
		AllocatedMirrorIds map[uint64]bool
	}

	dbg := dbgInfo{
		MirrorSessionDB:        tsa.DB.MirrorSessionDB,
		MirrorSessionNameToID:  tsa.DB.MirrorSessionNameToID,
		MirrorSessionIDToObj:   tsa.DB.MirrorSessionIDToObj,
		FlowMonitorRuleIDToObj: tsa.DB.FlowMonitorRuleIDToObj,
		DropRuleIDToObj:        tsa.DB.DropRuleIDToObj,
		AllocatedMirrorIds:     tsa.DB.AllocatedMirrorIds,
	}

	FlowMonitorRuleToID := map[string]uint64{}
	for k, v := range tsa.DB.FlowMonitorRuleToID {
		b, err := json.Marshal(k)
		if err != nil {
			log.Warnf("failed to marshal %+v", k)
			continue
		}
		FlowMonitorRuleToID[string(b)] = v
	}

	DropRuleToID := map[string]uint64{}
	for k, v := range tsa.DB.DropRuleToID {
		b, err := json.Marshal(k)
		if err != nil {
			log.Warnf("failed to marshal %+v", k)
			continue
		}
		DropRuleToID[string(b)] = v
	}
	dbg.FlowMonitorRuleToID = FlowMonitorRuleToID
	dbg.DropRuleToID = DropRuleToID
	json.NewEncoder(w).Encode(dbg)
}

// ListMirrorSession lists all mirror sessions
func (tsa *Tagent) ListMirrorSession() []*netproto.MirrorSession {
	log.Debugf("Processing packet capture session list...")
	//TODO
	tsa.Lock()
	defer tsa.Unlock()
	var mirrList []*netproto.MirrorSession

	// walk all mirror sessions
	for _, mr := range tsa.DB.MirrorSessionDB {
		mirrList = append(mirrList, mr)
	}

	return mirrList
}

//// CreateTechSupportRequest is not implemented
//func (tsa *Tagent) CreateTechSupportRequest(*tsproto.TechSupportRequest) error {
//	return errors.New("not implemented")
//}
//
//// UpdateTechSupportRequest is not implemented
//func (tsa *Tagent) UpdateTechSupportRequest(pcSession *tsproto.TechSupportRequest) error {
//	return errors.New("not implemented")
//}
//
//// DeleteTechSupportRequest is not implemented
//func (tsa *Tagent) DeleteTechSupportRequest(pcSession *tsproto.TechSupportRequest) error {
//	return errors.New("not implemented")
//}
//
//// GetTechSupportRequest is not implemented
//func (tsa *Tagent) GetTechSupportRequest(pcSession *tsproto.TechSupportRequest) *tsproto.TechSupportRequest {
//	return nil
//}
//
//// ListTechSupportRequest is not implemented
//func (tsa *Tagent) ListTechSupportRequest() []*tsproto.TechSupportRequest {
//	return nil
//}
