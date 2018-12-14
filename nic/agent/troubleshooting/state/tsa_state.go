package state

import (
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	config "github.com/pensando/sw/nic/agent/netagent/protos"
	netAgentState "github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	halMaxMirrorSession         = 8
	mirrorSessionDropRuleIDType = "MirrorSessionDropRuleID"
	mirrorSessionFlowRuleIDType = "MirrorSessionFlowRuleID"
)

// Tagent is an instance of Troubleshooting agent
type Tagent types.TsAgent

// ErrInvalidMirrorSpec error code is returned when mirror spec is invalid
var ErrInvalidMirrorSpec = errors.New("Mirror specification is incorrect")

// ErrMirrorSpecResource error code is returned when mirror create failed due to resource error
var ErrMirrorSpecResource = errors.New("Out of resource. No more mirror resource available")

// ErrInvalidFlowMonitorRule error code is returned when flow monitor rule is invalid
var ErrInvalidFlowMonitorRule = errors.New("Flow monitor rule is incorrect")

// ErrMsInternal error code is returned when mirror session agent code runs into data discrepency
var ErrMsInternal = errors.New("Mirror Session internal error")

// ErrDbRead error code is returned when error is encountered when reading objects from DB
var ErrDbRead = errors.New("Error retrieving object from database")

var nAgent *netAgentState.Nagent

// NewTsAgent creates new troubleshooting agent
func NewTsAgent(dp types.TsDatapathAPI, mode config.AgentMode, nodeUUID string, na *netAgentState.Nagent) (*Tagent, error) {
	var tsa Tagent
	var err error

	nAgent = na
	emdb := na.Store

	restart := false

	c := config.Agent{
		ObjectMeta: api.ObjectMeta{
			Name: "TsAgentConfig",
		},
		TypeMeta: api.TypeMeta{
			Kind: "TsAgent",
		},
		Spec: config.AgentSpec{
			Mode: mode,
		},
	}

	_, err = emdb.Read(&c)

	// Blank slate. Persist config and do init stuff
	if err != nil {
		err := emdb.Write(&c)
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

func (tsa *Tagent) findMirrorSession(meta api.ObjectMeta) (*tsproto.MirrorSession, error) {
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

	tsa.DB.MirrorSessionDB = make(map[string]*tsproto.MirrorSession)
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
		listMirrorSession := tsproto.MirrorSession{
			TypeMeta: api.TypeMeta{Kind: "MonitorSession"},
		}
		mirrorSessions, err := tsa.Store.List(&listMirrorSession)
		if err == nil {
			for _, storedMirrorSession := range mirrorSessions {
				mirrorSession, ok := storedMirrorSession.(*tsproto.MirrorSession)
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
		ruleID, err = tsa.Store.GetNextID(mirrorSessionDropRuleIDType)
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
		ruleID, err = tsa.Store.GetNextID(mirrorSessionFlowRuleIDType)
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

func isIpv4(ip string) bool {
	for i := 0; i < len(ip); i++ {
		switch ip[i] {
		case '.':
			return true
		case ':':
			return false
		}
	}
	return false
}
func isRangeAddr(ip string) bool {
	for i := 0; i < len(ip); i++ {
		switch ip[i] {
		case '-':
			return true
		}
	}
	return false
}
func isSubnetAddr(ip string) bool {
	for i := 0; i < len(ip); i++ {
		switch ip[i] {
		case '/':
			return true
		}
	}
	return false
}
func getPrefixLen(ip string) int32 {
	strs := strings.Split(ip, "/")
	if len(strs) > 1 {
		v, err := strconv.Atoi(strs[1])
		if err == nil {
			return int32(v)
		}
	}
	return -1
}

//returns ipaddr type, is Range (only last byte is allowed to have range), is ip prefix
func getIPAddrDetails(ipAddr string) (bool, bool, bool) {
	return isIpv4(ipAddr), isRangeAddr(ipAddr), isSubnetAddr(ipAddr)
}

func buildIPAddrDetails(ipaddr string) *types.IPAddrDetails {
	var ip net.IP

	if ipaddr == "any" {
		ipaddr = "0.0.0.0/0"
	}
	isIpv4, isRange, isSubnet := getIPAddrDetails(ipaddr)
	ipAddr := &types.IPAddrDetails{}
	if !isRange {
		if !isSubnet {
			if isIpv4 {
				ip = net.ParseIP(ipaddr).To4()
			} else {
				ip = net.ParseIP(ipaddr)
			}
			ipAddr = &types.IPAddrDetails{
				IP:       ip,
				IsIpv4:   isIpv4,
				IsSubnet: isSubnet,
				//prefixLen: 0
			}
		} else {
			ip, _, _ = net.ParseCIDR(ipaddr)
			if isIpv4 {
				ip = ip.To4()
			}
			ipAddr = &types.IPAddrDetails{
				IP:        ip,
				IsIpv4:    isIpv4,
				IsSubnet:  isSubnet,
				PrefixLen: uint32(getPrefixLen(ipaddr)),
			}
		}
	} else {
		// TODO Handle address range.
	}
	return ipAddr
}

//Given protocol string "TCP/123", return IPProtocol value corresponding to "TCP"
func getProtocol(portString string) int32 {
	strs := strings.Split(portString, "/")
	if len(strs) > 1 {
		protoStr := "IPPROTO_" + strings.ToUpper(strs[0])
		return halproto.IPProtocol_value[protoStr]
	}
	return -1
}

//Given protocol string "TCP/123", return IPProtocol value corresponding to "TCP"
func getPort(portString string) int32 {
	strs := strings.Split(portString, "/")
	if len(strs) > 1 {
		v, err := strconv.Atoi(strs[1])
		if err == nil {
			return int32(v)
		}
	}
	return -1
}

// If all rules in the spec pass sanity check, then return true.
// sanity check include correctness of  IPaddr string, mac addr string,
// application selectors
func matchRuleSanityCheck(mirrorSession *tsproto.MirrorSession) bool {
	for _, rule := range mirrorSession.Spec.MatchRules {
		srcSelectors := rule.Src
		destSelectors := rule.Dst
		appSelectors := rule.AppProtoSel
		if srcSelectors != nil {
			if len(srcSelectors.Endpoints) > 0 {
				tmeta := api.TypeMeta{Kind: "Endpoint"}
				ometa := api.ObjectMeta{
					Tenant:    mirrorSession.Tenant,
					Namespace: mirrorSession.Namespace,
				}
				for _, ep := range srcSelectors.Endpoints {
					ometa.Name = ep
					nAgent.Lock()
					_, ok := nAgent.EndpointDB[nAgent.Solver.ObjectKey(ometa, tmeta)]
					nAgent.Unlock()
					if !ok {
						log.Errorf("Src Endpoint %s not found", nAgent.Solver.ObjectKey(ometa, tmeta))
						return false
					}
				}
			} else if len(srcSelectors.IPAddresses) > 0 {
				for _, ipAddr := range srcSelectors.IPAddresses {
					if ipAddr == "any" {
						continue
					}
					_, isRange, isSubnet := getIPAddrDetails(ipAddr)
					if !isRange {
						if !isSubnet {
							if net.ParseIP(ipAddr) == nil {
								return false
							}
						} else {
							_, _, err := net.ParseCIDR(ipAddr)
							if err != nil {
								return false
							}
						}
					} else {
						// TODO: Handle IPaddr range
					}
				}
			} else if len(srcSelectors.MACAddresses) > 0 {
				for _, macAddr := range srcSelectors.MACAddresses {
					_, err := net.ParseMAC(macAddr)
					if err != nil {
						return false
					}
				}
			}
		}
		if destSelectors != nil {
			if len(destSelectors.Endpoints) > 0 {
				tmeta := api.TypeMeta{Kind: "Endpoint"}
				ometa := api.ObjectMeta{
					Tenant:    mirrorSession.Tenant,
					Namespace: mirrorSession.Namespace,
				}
				for _, ep := range destSelectors.Endpoints {
					ometa.Name = ep
					nAgent.Lock()
					_, ok := nAgent.EndpointDB[nAgent.Solver.ObjectKey(ometa, tmeta)]
					nAgent.Unlock()
					if !ok {
						log.Errorf("Dest Endpoint %s not found", nAgent.Solver.ObjectKey(ometa, tmeta))
						return false
					}
				}
			} else if len(destSelectors.IPAddresses) > 0 {
				for _, ipAddr := range destSelectors.IPAddresses {
					if ipAddr == "any" {
						continue
					}
					_, isRange, isSubnet := getIPAddrDetails(ipAddr)
					if !isRange {
						if !isSubnet {
							if net.ParseIP(ipAddr) == nil {
								return false
							}
						} else {
							_, _, err := net.ParseCIDR(ipAddr)
							if err != nil {
								return false
							}
						}
					} else {
						// TODO: Handle IPaddr range
					}
				}
			} else if len(destSelectors.MACAddresses) > 0 {
				for _, macAddr := range destSelectors.MACAddresses {
					_, err := net.ParseMAC(macAddr)
					if err != nil {
						return false
					}
				}
			}
		}

		if appSelectors != nil {
			if len(appSelectors.Ports) > 0 {
				// Ports specified by controller will be in the form
				// "tcp/5000"
				// When Protocol is invalid or when port# is not specified
				// fail sanity check.
				for _, protoPort := range appSelectors.Ports {
					if !strings.Contains(protoPort, "any") && getProtocol(protoPort) == -1 {
						return false
					}
					if !strings.Contains(protoPort, "any") && getPort(protoPort) == -1 {
						return false
					}
				}
			} else if len(appSelectors.Apps) > 0 {
				//TODO: Handle Application selection later. "Ex: Redis"
			}
		}
	}
	// TODO need to check if both srcIP and destIP are of same type (either both v4 or both v6)
	return true
}

// Process trouble shooting controller sent MatchRule (Which can be composite)
// and prepare list of first class match items.
//      List of source IPAddr or source MacAddrs
//      List of Destination IPAddr or Destination  MacAddrs
//      List of Application Selectors
// The caller of the function is expected to create cross product of
// these 3 lists and use each tuple (src, dest, app) as an atomic rule
// that can be sent to HAL
func expandCompositeMatchRule(mirrorSession *tsproto.MirrorSession, rule *tsproto.MatchRule) ([]*types.IPAddrDetails, []*types.IPAddrDetails, []uint64, []uint64, []*types.AppPortDetails, []string, []string) {
	srcSelectors := rule.Src
	destSelectors := rule.Dst
	appSelectors := rule.AppProtoSel
	var srcIPs []*types.IPAddrDetails
	var srcIPStrings []string
	var srcMACs []uint64
	var destIPs []*types.IPAddrDetails
	var destIPStrings []string
	var destMACs []uint64
	var appPorts []*types.AppPortDetails
	if srcSelectors != nil {
		if len(srcSelectors.Endpoints) > 0 {
			tmeta := api.TypeMeta{Kind: "Endpoint"}
			ometa := api.ObjectMeta{
				Tenant:    mirrorSession.Tenant,
				Namespace: mirrorSession.Namespace,
			}
			for _, ep := range srcSelectors.Endpoints {
				ometa.Name = ep
				nAgent.Lock()
				epObj, ok := nAgent.EndpointDB[nAgent.Solver.ObjectKey(ometa, tmeta)]
				nAgent.Unlock()
				if ok {
					if epObj.Spec.IPv4Address != "" {
						srcIPs = append(srcIPs, buildIPAddrDetails(epObj.Spec.IPv4Address))
						srcIPStrings = append(srcIPStrings, epObj.Spec.IPv4Address)
					}
					if epObj.Spec.IPv6Address != "" {
						srcIPs = append(srcIPs, buildIPAddrDetails(epObj.Spec.IPv6Address))
						srcIPStrings = append(srcIPStrings, epObj.Spec.IPv6Address)
					}
				}
			}
		} else if len(srcSelectors.IPAddresses) > 0 {
			for _, ipaddr := range srcSelectors.IPAddresses {
				srcIPs = append(srcIPs, buildIPAddrDetails(ipaddr))
				srcIPStrings = append(srcIPStrings, ipaddr)
			}
		} else if len(srcSelectors.MACAddresses) > 0 {
			for _, macAddr := range srcSelectors.MACAddresses {
				hwMac, _ := net.ParseMAC(macAddr)
				srcMACs = append(srcMACs, binary.BigEndian.Uint64(hwMac))
			}
		}
	} else {
		srcIPs = append(srcIPs, buildIPAddrDetails("0.0.0.0/0"))
		srcIPStrings = append(srcIPStrings, "0.0.0.0/0")
	}
	if destSelectors != nil {
		if len(destSelectors.Endpoints) > 0 {
			tmeta := api.TypeMeta{Kind: "Endpoint"}
			ometa := api.ObjectMeta{
				Tenant:    mirrorSession.Tenant,
				Namespace: mirrorSession.Namespace,
			}
			for _, ep := range destSelectors.Endpoints {
				ometa.Name = ep
				nAgent.Lock()
				epObj, ok := nAgent.EndpointDB[nAgent.Solver.ObjectKey(ometa, tmeta)]
				nAgent.Unlock()
				if ok {
					if epObj.Spec.IPv4Address != "" {
						destIPs = append(destIPs, buildIPAddrDetails(epObj.Spec.IPv4Address))
						destIPStrings = append(destIPStrings, epObj.Spec.IPv4Address)
					}
					if epObj.Spec.IPv6Address != "" {
						destIPs = append(destIPs, buildIPAddrDetails(epObj.Spec.IPv6Address))
						destIPStrings = append(destIPStrings, epObj.Spec.IPv6Address)
					}
				}
			}
		} else if len(destSelectors.IPAddresses) > 0 {
			for _, ipaddr := range destSelectors.IPAddresses {
				destIPs = append(destIPs, buildIPAddrDetails(ipaddr))
				destIPStrings = append(destIPStrings, ipaddr)
			}
		} else if len(destSelectors.MACAddresses) > 0 {
			for _, macAddr := range destSelectors.MACAddresses {
				hwMac, _ := net.ParseMAC(macAddr)
				destMACs = append(destMACs, binary.BigEndian.Uint64(hwMac))
			}
		}
	} else {
		destIPs = append(destIPs, buildIPAddrDetails("0.0.0.0/0"))
		destIPStrings = append(destIPStrings, "0.0.0.0/0")
	}
	if appSelectors != nil {
		if len(appSelectors.Ports) > 0 {
			// Ports specified by controller will be in the form
			// "tcp/5000"
			for _, protoPort := range appSelectors.Ports {
				protoAny := false
				portAny := false
				protoType := int32(0)
				portNum := int32(0)
				strs := strings.Split(protoPort, "/")
				if !strings.Contains(strs[0], "any") {
					protoType = getProtocol(protoPort)
				} else {
					protoAny = true
				}
				if len(strs) > 1 && !strings.Contains(strs[1], "any") {
					portNum = getPort(protoPort)
				} else {
					portAny = true
				}
				appPort := &types.AppPortDetails{}
				if !protoAny {
					appPort.Ipproto = protoType
				}
				if !portAny {
					appPort.L4port = portNum
				}
				appPorts = append(appPorts, appPort)
			}
		} else if len(appSelectors.Apps) > 0 {
			//TODO: Handle Application selection later. "Ex: Redis"
		}
	} else {
		appPort := &types.AppPortDetails{}
		appPorts = append(appPorts, appPort)
	}
	return srcIPs, destIPs, srcMACs, destMACs, appPorts, srcIPStrings, destIPStrings
}

func createIPAddrCrossProductRuleList(srcIPs, destIPs []*types.IPAddrDetails, appPorts []*types.AppPortDetails, srcIPStrings []string, destIPStrings []string) ([]*types.FlowMonitorIPRuleDetails, error) {

	if len(srcIPs) == 0 && len(destIPs) == 0 {
		return nil, ErrInvalidFlowMonitorRule
	}
	var flowMonitorRules []*types.FlowMonitorIPRuleDetails

	for i := 0; i < len(srcIPs); i++ {
		for j := 0; j < len(destIPs); j++ {
			for k := 0; k < len(appPorts); k++ {
				iprule := &types.FlowMonitorIPRuleDetails{
					SrcIPObj:     srcIPs[i],
					SrcIPString:  srcIPStrings[i],
					DestIPObj:    destIPs[j],
					DestIPString: destIPStrings[j],
					AppPortObj:   appPorts[k],
				}
				flowMonitorRules = append(flowMonitorRules, iprule)
			}
		}
	}
	return flowMonitorRules, nil
}

func createMACAddrCrossProductRuleList(srcMACs, destMACs []uint64, appPorts []*types.AppPortDetails) ([]*types.FlowMonitorMACRuleDetails, error) {

	if len(srcMACs) == 0 && len(destMACs) == 0 {
		return nil, ErrInvalidFlowMonitorRule
	}
	var flowMonitorRules []*types.FlowMonitorMACRuleDetails

	for i := 0; i < len(srcMACs); i++ {
		for j := 0; j < len(destMACs); j++ {
			for k := 0; k < len(appPorts); k++ {
				iprule := &types.FlowMonitorMACRuleDetails{
					SrcMAC:     srcMACs[i],
					DestMAC:    destMACs[j],
					AppPortObj: appPorts[k],
				}
				flowMonitorRules = append(flowMonitorRules, iprule)
			}
		}
	}
	return flowMonitorRules, nil
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

func buildIPAddrObjProtoObj(ipaddr *types.IPAddrDetails) *halproto.IPAddressObj {

	addrObj := &halproto.IPAddressObj{}

	if ipaddr.IsIpv4 && !ipaddr.IsSubnet {
		//v4, no-cidr
		addrObj.Formats = &halproto.IPAddressObj_Address{
			Address: &halproto.Address{
				Address: &halproto.Address_Range{
					Range: &halproto.AddressRange{
						Range: &halproto.AddressRange_Ipv4Range{
							Ipv4Range: &halproto.IPRange{
								LowIpaddr: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET,
									V4OrV6: &halproto.IPAddress_V4Addr{
										V4Addr: binary.BigEndian.Uint32(ipaddr.IP),
									},
								},
								HighIpaddr: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET,
									V4OrV6: &halproto.IPAddress_V4Addr{
										V4Addr: binary.BigEndian.Uint32(ipaddr.IP),
									},
								},
							},
						},
					},
				},
			},
		}
	} else if ipaddr.IsIpv4 && ipaddr.IsSubnet {
		//v4, cidr
		addrObj.Formats = &halproto.IPAddressObj_Address{
			Address: &halproto.Address{
				Address: &halproto.Address_Prefix{
					Prefix: &halproto.IPSubnet{
						Subnet: &halproto.IPSubnet_Ipv4Subnet{
							Ipv4Subnet: &halproto.IPPrefix{
								Address: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET,
									V4OrV6: &halproto.IPAddress_V4Addr{
										V4Addr: binary.BigEndian.Uint32(ipaddr.IP),
									},
								},
								PrefixLen: ipaddr.PrefixLen,
							},
						},
					},
				},
			},
		}
	} else if !ipaddr.IsIpv4 && !ipaddr.IsSubnet {
		//v6, no-cidr
		addrObj.Formats = &halproto.IPAddressObj_Address{
			Address: &halproto.Address{
				Address: &halproto.Address_Range{
					Range: &halproto.AddressRange{
						Range: &halproto.AddressRange_Ipv6Range{
							Ipv6Range: &halproto.IPRange{
								LowIpaddr: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET6,
									V4OrV6: &halproto.IPAddress_V6Addr{
										V6Addr: ipaddr.IP,
									},
								},
								HighIpaddr: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET6,
									V4OrV6: &halproto.IPAddress_V6Addr{
										V6Addr: ipaddr.IP,
									},
								},
							},
						},
					},
				},
			},
		}
	} else if !ipaddr.IsIpv4 && ipaddr.IsSubnet {
		//v6, cidr
		addrObj.Formats = &halproto.IPAddressObj_Address{
			Address: &halproto.Address{
				Address: &halproto.Address_Prefix{
					Prefix: &halproto.IPSubnet{
						Subnet: &halproto.IPSubnet_Ipv6Subnet{
							Ipv6Subnet: &halproto.IPPrefix{
								Address: &halproto.IPAddress{
									IpAf: halproto.IPAddressFamily_IP_AF_INET6,
									V4OrV6: &halproto.IPAddress_V6Addr{
										V6Addr: ipaddr.IP,
									},
								},
								PrefixLen: ipaddr.PrefixLen,
							},
						},
					},
				},
			},
		}
	}

	return addrObj
}

func buildAppMatchInfoObj(appPort *types.AppPortDetails) *halproto.RuleMatch_AppMatch {

	appMatch := &halproto.RuleMatch_AppMatch{}
	if appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_TCP) || appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_UDP) {
		l4port := &halproto.L4PortRange{
			PortLow:  uint32(appPort.L4port),
			PortHigh: uint32(appPort.L4port),
		}
		appMatch.App = &halproto.RuleMatch_AppMatch_PortInfo{
			PortInfo: &halproto.RuleMatch_L4PortAppInfo{
				//Controller provided port# and proto is applied on only dest
				// srcPort is not populated.
				DstPortRange: []*halproto.L4PortRange{l4port},
				//SrcPortRange:
			},
		}
	}
	if appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_ICMP) {
		appMatch.App = &halproto.RuleMatch_AppMatch_IcmpInfo{
			IcmpInfo: &halproto.RuleMatch_ICMPAppInfo{
				IcmpType: 0, //TODO : Get this value from ctrler
				IcmpCode: 0, //TODO : Get this value from ctrler
			},
		}
	}
	if appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_ICMPV6) {
		appMatch.App = &halproto.RuleMatch_AppMatch_Icmpv6Info{
			Icmpv6Info: &halproto.RuleMatch_ICMPv6AppInfo{
				Icmpv6Type: 0, //TODO : Get this value from ctrler
				Icmpv6Code: 0, //TODO : Get this value from ctrler
			},
		}
	}

	return appMatch
}

func buildVeniceCollectorProtoObj(mirrorSession *tsproto.MirrorSession, mirrorSessID uint64) []*halproto.MirrorSessionSpec_LocalSpanIf {
	var mirrorCollectors []*halproto.MirrorSessionSpec_LocalSpanIf
	for _, mirrorCollector := range mirrorSession.Spec.Collectors {
		if mirrorCollector.Type == "VENICE" {
			mirrorDestObj := &halproto.MirrorSessionSpec_LocalSpanIf{}
			//TODO
			mirrorCollectors = append(mirrorCollectors, mirrorDestObj)
		}
	}
	return mirrorCollectors
}

func buildErspanCollectorProtoObj(mirrorSession *tsproto.MirrorSession, mirrorSessID uint64) []*halproto.MirrorSessionSpec_ErspanSpec {
	var mirrorCollectors []*halproto.MirrorSessionSpec_ErspanSpec
	for _, mirrorCollector := range mirrorSession.Spec.Collectors {
		if mirrorCollector.Type == "ERSPAN" {
			//mirrorCollector.ExportCfg.Transport -- Does this info from ctrler need to be used ??
			destIPDetails := buildIPAddrDetails(mirrorCollector.ExportCfg.Destination)
			mirrorDestObj := &halproto.MirrorSessionSpec_ErspanSpec{
				ErspanSpec: &halproto.ERSpanSpec{
					DestIp: buildIPAddrProtoObj(destIPDetails),
					SpanId: uint32(mirrorSessID), //For now this value is same as mirrorSessionID.
					//SrcIp: HAL can for now choose appropriate value.
					//Dscp:  HAL can for now choose appropriate value.
				},
			}
			mirrorCollectors = append(mirrorCollectors, mirrorDestObj)
		}
	}
	return mirrorCollectors
}

func buildMirrorTrafficCollectorProtoObj(mirrorSession *tsproto.MirrorSession, mirrorSessID uint64) ([]*halproto.MirrorSessionSpec_LocalSpanIf, []*halproto.MirrorSessionSpec_ErspanSpec) {
	veniceCollectors := buildVeniceCollectorProtoObj(mirrorSession, mirrorSessID)
	erspanCollectors := buildErspanCollectorProtoObj(mirrorSession, mirrorSessID)
	return veniceCollectors, erspanCollectors
}

func getVrfID(mirrorSession *tsproto.MirrorSession) (vrfID uint64, err error) {
	// Map Namespace-Name to Vrf
	nsObj, err := nAgent.FindNamespace(mirrorSession.Tenant, mirrorSession.Namespace)
	if err == nil {
		vrfID = nsObj.Status.NamespaceID
		return
	}
	log.Errorf("mirror session tenant %s not found", mirrorSession.Tenant)
	return
}

func (tsa *Tagent) createHALMirrorSessionProtoObj(mirrorSession *tsproto.MirrorSession, sessID uint64) (*halproto.MirrorSessionRequestMsg, error) {

	veniceCollectors, erspanCollectors := buildMirrorTrafficCollectorProtoObj(mirrorSession, sessID)
	if len(veniceCollectors) == 0 && len(erspanCollectors) == 0 {
		log.Errorf("mirror session collector is neither erspan nor venice/local")
		return nil, ErrInvalidMirrorSpec
	}

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

func (tsa *Tagent) buildDropRuleUpdateProtoObj(mirrorSession *tsproto.MirrorSession,
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

func (tsa *Tagent) buildDropRuleDeleteProtoObj(mirrorSession *tsproto.MirrorSession,
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

func (tsa *Tagent) createDropMonitorRuleIDMatchingHALProtoObj(mirrorSession *tsproto.MirrorSession,
	mirrorSessID uint64, updateDropRuleIDs, deleteDropRuleIDs []uint64) ([]*halproto.DropMonitorRuleRequestMsg, []*halproto.DropMonitorRuleDeleteRequestMsg, error) {

	var updateReqMsgList []*halproto.DropMonitorRuleRequestMsg
	var deleteReqMsgList []*halproto.DropMonitorRuleDeleteRequestMsg

	for _, filter := range mirrorSession.Spec.PacketFilters {
		updateReqMsg := halproto.DropMonitorRuleRequestMsg{}
		deleteReqMsg := halproto.DropMonitorRuleDeleteRequestMsg{}
		if filter == "ALL_DROPS" {
			allDropReasons := getAllDropReasons()
			for i := range allDropReasons {
				dropRuleSpec, _ := tsa.buildDropRuleUpdateProtoObj(mirrorSession, &allDropReasons[i], updateDropRuleIDs)
				if dropRuleSpec != nil {
					updateReqMsg.Request = append(updateReqMsg.Request, dropRuleSpec)
				} else {
					deleteDropRule, _ := tsa.buildDropRuleDeleteProtoObj(mirrorSession, &allDropReasons[i], deleteDropRuleIDs)
					if deleteDropRule != nil {
						deleteReqMsg.Request = append(deleteReqMsg.Request, deleteDropRule)
					}
				}
			}
		} else if filter == "NETWORK_POLICY_DROP" {
			nwPolicyDrops := getNetWorkPolicyDropReasons()
			for i := range nwPolicyDrops {
				dropRuleSpec, _ := tsa.buildDropRuleUpdateProtoObj(mirrorSession, &nwPolicyDrops[i], updateDropRuleIDs)
				if dropRuleSpec != nil {
					updateReqMsg.Request = append(updateReqMsg.Request, dropRuleSpec)
				} else {
					deleteDropRule, _ := tsa.buildDropRuleDeleteProtoObj(mirrorSession, &nwPolicyDrops[i], deleteDropRuleIDs)
					if deleteDropRule != nil {
						deleteReqMsg.Request = append(deleteReqMsg.Request, deleteDropRule)
					}
				}
			}
		} else if filter == "FIREWALL_POLICY_DROP" {
			fwPolicyDrops := getFireWallPolicyDropReasons()
			for i := range fwPolicyDrops {
				dropRuleSpec, _ := tsa.buildDropRuleUpdateProtoObj(mirrorSession, &fwPolicyDrops[i], updateDropRuleIDs)
				if dropRuleSpec != nil {
					updateReqMsg.Request = append(updateReqMsg.Request, dropRuleSpec)
				} else {
					deleteDropRule, _ := tsa.buildDropRuleDeleteProtoObj(mirrorSession, &fwPolicyDrops[i], deleteDropRuleIDs)
					if deleteDropRule != nil {
						deleteReqMsg.Request = append(deleteReqMsg.Request, deleteDropRule)
					}
				}
			}
		}
		updateReqMsgList = append(updateReqMsgList, &updateReqMsg)
		deleteReqMsgList = append(deleteReqMsgList, &deleteReqMsg)
	}
	return updateReqMsgList, deleteReqMsgList, nil
}

func (tsa *Tagent) buildDropRuleCreateProtoObj(mirrorSession *tsproto.MirrorSession, mirrorSessID uint64, dropReason *halproto.DropReasons) (*halproto.DropMonitorRuleSpec, uint64, bool, error) {
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

func (tsa *Tagent) createHALDropMonitorRulesProtoObj(mirrorSession *tsproto.MirrorSession, mirrorSessID uint64) ([]*halproto.DropMonitorRuleRequestMsg, []uint64, error) {

	var newRuleIDs []uint64

	var ReqMsgList []*halproto.DropMonitorRuleRequestMsg
	for _, filter := range mirrorSession.Spec.PacketFilters {
		ReqMsg := halproto.DropMonitorRuleRequestMsg{}
		// Iterate over drops reasons and create one ruleSpec for each drop condition.
		// HAL expects each droprule to contain only one dropreason code.
		//create array of all drop reasons
		if filter == "ALL_DROPS" {
			allDropReasons := getAllDropReasons()
			for i := range allDropReasons {
				dropRuleSpec, ruleID, allocated, err := tsa.buildDropRuleCreateProtoObj(mirrorSession, mirrorSessID, &allDropReasons[i])
				if err == nil {
					ReqMsg.Request = append(ReqMsg.Request, dropRuleSpec)
					if allocated {
						newRuleIDs = append(newRuleIDs, ruleID)
					}
				} else {
					return nil, nil, err
				}
			}
		} else if filter == "NETWORK_POLICY_DROP" {
			nwPolicyDrops := getNetWorkPolicyDropReasons()
			for i := range nwPolicyDrops {
				dropRuleSpec, ruleID, allocated, err := tsa.buildDropRuleCreateProtoObj(mirrorSession, mirrorSessID, &nwPolicyDrops[i])
				if err == nil {
					ReqMsg.Request = append(ReqMsg.Request, dropRuleSpec)
					if allocated {
						newRuleIDs = append(newRuleIDs, ruleID)
					}
				} else {
					return nil, nil, err
				}
			}
		} else if filter == "FIREWALL_POLICY_DROP" {
			fwPolicyDrops := getFireWallPolicyDropReasons()
			for i := range fwPolicyDrops {
				dropRuleSpec, ruleID, allocated, err := tsa.buildDropRuleCreateProtoObj(mirrorSession, mirrorSessID, &fwPolicyDrops[i])
				if err == nil {
					ReqMsg.Request = append(ReqMsg.Request, dropRuleSpec)
					if allocated {
						newRuleIDs = append(newRuleIDs, ruleID)
					}
				} else {
					return nil, nil, err
				}
			}
		}
		ReqMsgList = append(ReqMsgList, &ReqMsg)
	}
	return ReqMsgList, newRuleIDs, nil
}

func (tsa *Tagent) createFlowMonitorRuleIDMatchingHALProtoObj(mirrorSession *tsproto.MirrorSession,
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
		srcIPs, destIPs, srcMACs, destMACs, appPorts, srcIPStrings, destIPStrings := expandCompositeMatchRule(mirrorSession, &rule)
		// Create protobuf requestMsgs on cross product of
		//  - srcIPs, destIPs, Apps
		//  - srcMACs, destMACs, Apps
		flowMonitorRuleSpecified := false
		ipFmRuleList, err := createIPAddrCrossProductRuleList(srcIPs, destIPs, appPorts, srcIPStrings, destIPStrings)
		if err == nil {
			flowMonitorRuleSpecified = true
		}

		macFmRuleList, err := createMACAddrCrossProductRuleList(srcMACs, destMACs, appPorts)
		if err == nil {
			flowMonitorRuleSpecified = true
		}

		//TODO: Fold in ether-type in monitor rule

		if !flowMonitorRuleSpecified {
			log.Errorf("Match Rules specified with only AppPort Selector without IP/MAC")
			return nil, nil, ErrInvalidFlowMonitorRule
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
				srcAddrObj := buildIPAddrObjProtoObj(ipFmRule.SrcIPObj)
				destAddrObj := buildIPAddrObjProtoObj(ipFmRule.DestIPObj)
				appMatchObj := buildAppMatchInfoObj(ipFmRule.AppPortObj)
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
						Protocol:   halproto.IPProtocol(ipFmRule.AppPortObj.Ipproto),
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

				appMatchObj := buildAppMatchInfoObj(macRule.AppPortObj)
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
						Protocol:      halproto.IPProtocol(macRule.AppPortObj.Ipproto),
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

func (tsa *Tagent) createHALFlowMonitorRulesProtoObj(mirrorSession *tsproto.MirrorSession, mirrorSessID uint64) ([]*halproto.FlowMonitorRuleRequestMsg, []uint64, error) {

	var ReqMsgList []*halproto.FlowMonitorRuleRequestMsg
	var newRuleIDs []uint64

	vrfID, err := getVrfID(mirrorSession)
	if err != nil {
		log.Errorf("mirror session tenant is invalid")
		return nil, nil, ErrInvalidMirrorSpec
	}

	for _, rule := range mirrorSession.Spec.MatchRules {
		srcIPs, destIPs, srcMACs, destMACs, appPorts, srcIPStrings, destIPStrings := expandCompositeMatchRule(mirrorSession, &rule)
		// Create protobuf requestMsgs on cross product of
		//  - srcIPs, destIPs, Apps
		//  - srcMACs, destMACs, Apps
		flowMonitorRuleSpecified := false
		ipFmRuleList, err := createIPAddrCrossProductRuleList(srcIPs, destIPs, appPorts, srcIPStrings, destIPStrings)
		if err == nil {
			flowMonitorRuleSpecified = true
		}

		macFmRuleList, err := createMACAddrCrossProductRuleList(srcMACs, destMACs, appPorts)
		if err == nil {
			flowMonitorRuleSpecified = true
		}

		//TODO: Fold in ether-type in monitor rule

		if !flowMonitorRuleSpecified {
			log.Errorf("Match Rules specified with only AppPort Selector without IP/MAC")
			return nil, nil, ErrInvalidFlowMonitorRule
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
			srcAddrObj := buildIPAddrObjProtoObj(ipFmRule.SrcIPObj)
			destAddrObj := buildIPAddrObjProtoObj(ipFmRule.DestIPObj)
			appMatchObj := buildAppMatchInfoObj(ipFmRule.AppPortObj)
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
					Protocol:   halproto.IPProtocol(ipFmRule.AppPortObj.Ipproto),
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

			appMatchObj := buildAppMatchInfoObj(macRule.AppPortObj)
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
					Protocol:      halproto.IPProtocol(macRule.AppPortObj.Ipproto),
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
		filterAllPkts := false
		for _, filter := range mirrorSession.Spec.PacketFilters {
			if filter == "ALL_PKTS" {
				filterAllPkts = true
				break
			}
			if filterAllPkts {
			}
		}
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
func (tsa *Tagent) createPacketCaptureSessionProtoObjs(mirrorSession *tsproto.MirrorSession) (*mirrorProtoObjs, error) {
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
	if !dropMonitor && !matchRuleSanityCheck(mirrorSession) {
		// Rule santy check failed.
		log.Errorf("Neither drop rule nor match selector rules specified in mirror spec")
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
	// Drop monitor rules
	if dropMonitor {
		dropRuleProtoObjs, newDropRuleIDs, err = tsa.createHALDropMonitorRulesProtoObj(mirrorSession, sessID)
		if err != nil {
			log.Errorf("Could not create mirror drop monitor rule object")
			return nil, err
		}
	}
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

func (tsa *Tagent) storePacketCaptureSessionInDB(pcSession *tsproto.MirrorSession, mirrorProtoObjs *mirrorProtoObjs) error {
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

func (tsa *Tagent) deleteModifyMirrorSessionRules(pcSession *tsproto.MirrorSession, purgedFlowRuleIDs, purgedDropRuleIDs []uint64) ([]uint64, []uint64, error) {
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

func (tsa *Tagent) createUpdatePacketCaptureSession(pcSession *tsproto.MirrorSession, update bool) error {
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
		log.Errorf("Packet capture session create or update request errored")
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

func (tsa *Tagent) deletePacketCaptureSession(pcSession *tsproto.MirrorSession) error {
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
func (tsa *Tagent) CreateMirrorSession(pcSession *tsproto.MirrorSession) error {
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
	if !pcSession.Spec.Enable {
		// Session created in disable mode. Just store the mirrorSession in DB
		// At later time, update mirrorsession will trigger setting up mirror
		// session in HW. Update mirrorSession object will also carry required
		// drop/monitor rules.
		key := objectKey(pcSession.ObjectMeta, pcSession.TypeMeta)
		tsa.Lock()
		tsa.DB.MirrorSessionDB[key] = pcSession
		tsa.Unlock()
		return nil
	}
	return tsa.createUpdatePacketCaptureSession(pcSession, false)
}

// UpdateMirrorSession updates mirror session
func (tsa *Tagent) UpdateMirrorSession(pcSession *tsproto.MirrorSession) error {
	log.Debugf("Processing packet capture session update... %+v", pcSession.Name)
	if pcSession.Name == "" {
		log.Errorf("mirror session name is empty")
		return ErrInvalidMirrorSpec
	}
	oldMs, err := tsa.findMirrorSession(pcSession.ObjectMeta)
	if err == nil {
		if proto.Equal(oldMs, pcSession) {
			log.Errorf("MirrorSession %+v update is same as what is already created", oldMs)
			return errors.New("Duplicate MirrorSession during update")
		}
	} else {
		log.Errorf("MirrorSession %v does not exist to update", pcSession.Name)
		return ErrInvalidMirrorSpec
	}
	if pcSession.Spec.Enable {
		return tsa.createUpdatePacketCaptureSession(pcSession, true)
	}
	return nil
}

// DeleteMirrorSession deletes packet capture session.
func (tsa *Tagent) DeleteMirrorSession(pcSession *tsproto.MirrorSession) error {
	log.Debugf("Processing packet capture session delete... %v", pcSession.Name)
	_, err := tsa.findMirrorSession(pcSession.ObjectMeta)
	if err != nil {
		log.Errorf("MirrorSession %v does not exist to delete it", pcSession.Name)
		return fmt.Errorf("mirror session %v does not exist", pcSession.Name)
	}
	return tsa.deletePacketCaptureSession(pcSession)
}

// GetMirrorSession gets a mirror session
func (tsa *Tagent) GetMirrorSession(pcSession *tsproto.MirrorSession) *tsproto.MirrorSession {
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

// ListMirrorSession lists all mirror sessions
func (tsa *Tagent) ListMirrorSession() []*tsproto.MirrorSession {
	log.Debugf("Processing packet capture session list...")
	//TODO
	return nil
}

// CreateTechSupportRequest is not implemented
func (tsa *Tagent) CreateTechSupportRequest(*tsproto.TechSupportRequest) error {
	return errors.New("not implemented")
}

// UpdateTechSupportRequest is not implemented
func (tsa *Tagent) UpdateTechSupportRequest(pcSession *tsproto.TechSupportRequest) error {
	return errors.New("not implemented")
}

// DeleteTechSupportRequest is not implemented
func (tsa *Tagent) DeleteTechSupportRequest(pcSession *tsproto.TechSupportRequest) error {
	return errors.New("not implemented")
}

// GetTechSupportRequest is not implemented
func (tsa *Tagent) GetTechSupportRequest(pcSession *tsproto.TechSupportRequest) *tsproto.TechSupportRequest {
	return nil
}

// ListTechSupportRequest is not implemented
func (tsa *Tagent) ListTechSupportRequest() []*tsproto.TechSupportRequest {
	return nil
}
