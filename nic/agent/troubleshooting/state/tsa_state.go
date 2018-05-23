package state

import (
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	config "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	halMaxMirrorSession = 8
)

// Tagent is an instance of Troubleshooting agent
type Tagent types.TsAgent

// ErrInvalidMirrorSpec error code is returned when mirror spec is invalid
var ErrInvalidMirrorSpec = errors.New("Mirror specification is incorrect")

// ErrMirrorSpecResource error code is returned when mirror create failed due to resource error
var ErrMirrorSpecResource = errors.New("Out of resource. No more mirror resource available")

// ErrInvalidFlowMonitorRule error code is returned when flow monitor rule is invalid
var ErrInvalidFlowMonitorRule = errors.New("Flow monitor rule is incorrect")

// NewTsAgent creates new troubleshooting agent
func NewTsAgent(dp types.TsDatapathAPI, mode config.AgentMode, dbPath, nodeUUID string) (*Tagent, error) {
	var tsa Tagent
	var emdb emstore.Emstore
	var err error

	if dbPath == "" {
		emdb, err = emstore.NewEmstore(emstore.MemStoreType, "TsAgentDB")
	} else {
		emdb, err = emstore.NewEmstore(emstore.BoltDBType, dbPath)
	}
	if err != nil {
		log.Errorf("Error opening the embedded db. Err: %v", err)
		return nil, err
	}

	tsa.init(emdb, nodeUUID, dp)

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
	tsa.Store.Close()
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

func (tsa *Tagent) init(emdb emstore.Emstore, nodeUUID string, dp types.TsDatapathAPI) {
	tsa.Store = emdb
	tsa.NodeUUID = nodeUUID
	tsa.Datapath = dp
	// Initialize all maps in DB
	tsa.DB.MirrorSessionNameToID = make(map[string]uint32)
	tsa.DB.PktMirrorSessions = make(map[string]*types.MirrorSessionStatus)
	tsa.DB.FlowMonitorRuleIDToFlowMonitorRule = make(map[uint64]types.FlowMonitorRule)
	tsa.DB.FlowMonitorRules = make(map[types.FlowMonitorRule]*types.MonitorRuleStatus)
	tsa.DB.DropRuleIDToDropRule = make(map[uint64]types.DropMonitorRule)
	tsa.DB.DropMonitorRules = make(map[types.DropMonitorRule]*types.MonitorRuleStatus)
	tsa.DB.AllocatedMirrorIds = 1
	tsa.DB.AllocatedDropRuleIds = 1
	tsa.DB.AllocatedFlowMonitorRuleIds = 1
}

func allocateMirrorSessionID(tsa *Tagent, msName string) uint32 {
	if _, v := tsa.DB.MirrorSessionNameToID[msName]; v {
		return tsa.DB.MirrorSessionNameToID[msName]
	}
	if tsa.DB.AllocatedMirrorIds < halMaxMirrorSession {
		tsa.DB.MirrorSessionNameToID[msName] = tsa.DB.AllocatedMirrorIds
		tsa.DB.AllocatedMirrorIds++
		return tsa.DB.MirrorSessionNameToID[msName]
	}
	return halMaxMirrorSession + 1
}

func getMirrorSessionID(tsa *Tagent, msName string) uint32 {
	if _, v := tsa.DB.MirrorSessionNameToID[msName]; v {
		return tsa.DB.MirrorSessionNameToID[msName]
	}
	return halMaxMirrorSession + 1
}

func allocateDropRuleID(tsa *Tagent, dropRule types.DropMonitorRule) uint64 {
	var ruleID uint64
	if _, v := tsa.DB.DropMonitorRules[dropRule]; v {
		ruleID = tsa.DB.DropMonitorRules[dropRule].RuleID
	} else {
		ruleID = tsa.DB.AllocatedDropRuleIds
		tsa.DB.AllocatedDropRuleIds++
		tsa.DB.DropRuleIDToDropRule[ruleID] = dropRule
	}
	return ruleID
}

func getDropRuleID(tsa *Tagent, dropRule types.DropMonitorRule) uint64 {
	if _, v := tsa.DB.DropMonitorRules[dropRule]; v {
		return tsa.DB.DropMonitorRules[dropRule].RuleID
	}
	return 0
}

func getFlowMonitorRuleID(tsa *Tagent, flowRule types.FlowMonitorRule) uint64 {
	if _, v := tsa.DB.FlowMonitorRules[flowRule]; v {
		return tsa.DB.FlowMonitorRules[flowRule].RuleID
	}
	return 0
}

func allocateFlowMonitorRuleID(tsa *Tagent, flowRule types.FlowMonitorRule) uint64 {
	var ruleID uint64
	if _, v := tsa.DB.FlowMonitorRules[flowRule]; v {
		ruleID = tsa.DB.FlowMonitorRules[flowRule].RuleID
	} else {
		ruleID = tsa.DB.AllocatedFlowMonitorRuleIds
		tsa.DB.AllocatedFlowMonitorRuleIds++
		tsa.DB.FlowMonitorRuleIDToFlowMonitorRule[ruleID] = flowRule
	}
	return ruleID
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
	// TODO: Scrub drop reason code to match Network policy drop
	NwPolicyDropReasons := []halproto.DropReasons{
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
	}
	return NwPolicyDropReasons
}

func getFireWallPolicyDropReasons() []halproto.DropReasons {
	// TODO: Scrub drop reason code to match firewall policy drop
	FwPolicyDropReasons := []halproto.DropReasons{
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
	isIpv4, isRange, isSubnet := getIPAddrDetails(ipaddr)
	ipAddr := &types.IPAddrDetails{}
	if !isRange {
		if !isSubnet {
			//TODO: ParseIP func returns bytes[]. Do they need to endian adjusted ??
			ip := net.ParseIP(ipaddr)
			ipAddr = &types.IPAddrDetails{
				IP:       ip,
				IsIpv4:   isIpv4,
				IsSubnet: isSubnet,
				//prefixLen: 0
			}
		} else {
			ip, _, _ := net.ParseCIDR(ipaddr)
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
				//TODO : Lookup endpoint name and check presence of endpoint in netagent.
			} else if len(srcSelectors.IPAddresses) > 0 {
				for _, ipAddr := range srcSelectors.IPAddresses {
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
				//TODO : Lookup endpoint name and get IP address
			} else if len(destSelectors.IPAddresses) > 0 {
				for _, ipAddr := range destSelectors.IPAddresses {
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
					if getProtocol(protoPort) == -1 {
						return false
					}
					if getPort(protoPort) == -1 {
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
func expandCompositeMatchRule(rule *tsproto.MatchRule) ([]*types.IPAddrDetails, []*types.IPAddrDetails, []uint64, []uint64, []*types.AppPortDetails) {
	srcSelectors := rule.Src
	destSelectors := rule.Dst
	appSelectors := rule.AppProtoSel
	var srcIPs []*types.IPAddrDetails
	var srcMACs []uint64
	var destIPs []*types.IPAddrDetails
	var destMACs []uint64
	var appPorts []*types.AppPortDetails
	if srcSelectors != nil {
		if len(srcSelectors.Endpoints) > 0 {
			//TODO : Lookup endpoint name and get IP address
		} else if len(srcSelectors.IPAddresses) > 0 {
			for _, ipaddr := range srcSelectors.IPAddresses {
				srcIPs = append(srcIPs, buildIPAddrDetails(ipaddr))
			}
		} else if len(srcSelectors.MACAddresses) > 0 {
			for _, macAddr := range srcSelectors.MACAddresses {
				hwMac, _ := net.ParseMAC(macAddr)
				srcMACs = append(srcMACs, binary.BigEndian.Uint64(hwMac))
			}
		}
	}
	if destSelectors != nil {
		if len(destSelectors.Endpoints) > 0 {
			//TODO : Lookup endpoint name and get IP address
		} else if len(destSelectors.IPAddresses) > 0 {
			for _, ipaddr := range destSelectors.IPAddresses {
				destIPs = append(destIPs, buildIPAddrDetails(ipaddr))
			}
		} else if len(destSelectors.MACAddresses) > 0 {
			for _, macAddr := range destSelectors.MACAddresses {
				hwMac, _ := net.ParseMAC(macAddr)
				destMACs = append(destMACs, binary.BigEndian.Uint64(hwMac))
			}
		}
	}
	if appSelectors != nil {
		if len(appSelectors.Ports) > 0 {
			// Ports specified by controller will be in the form
			// "tcp/5000"
			for _, protoPort := range appSelectors.Ports {
				protoType := getProtocol(protoPort)
				portNum := getPort(protoPort)
				appPort := &types.AppPortDetails{
					Ipproto: protoType,
					L4port:  portNum,
				}
				appPorts = append(appPorts, appPort)
			}
		} else if len(appSelectors.Apps) > 0 {
			//TODO: Handle Application selection later. "Ex: Redis"
		}
	}
	return srcIPs, destIPs, srcMACs, destMACs, appPorts
}

func createIPAddrCrossProductRuleList(srcIPs, destIPs []*types.IPAddrDetails, appPorts []*types.AppPortDetails) ([]*types.FlowMonitorIPRuleDetails, error) {

	if len(srcIPs) == 0 && len(destIPs) == 0 {
		return nil, ErrInvalidFlowMonitorRule
	}
	var flowMonitorRules []*types.FlowMonitorIPRuleDetails

	for i := 0; i < len(srcIPs); i++ {
		for j := 0; j < len(destIPs); j++ {
			for k := 0; k < len(appPorts); k++ {
				iprule := &types.FlowMonitorIPRuleDetails{
					SrcIPObj:   srcIPs[i],
					DestIPObj:  destIPs[j],
					AppPortObj: appPorts[k],
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

func buildAppMatchInfoObj(appPort *types.AppPortDetails) *halproto.RuleMatch_AppMatchInfo {

	appMatch := &halproto.RuleMatch_AppMatchInfo{}
	if appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_TCP) || appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_UDP) {
		l4port := &halproto.L4PortRange{
			PortLow:  uint32(appPort.L4port),
			PortHigh: uint32(appPort.L4port),
		}
		appMatch.App = &halproto.RuleMatch_AppMatchInfo_PortInfo{
			PortInfo: &halproto.RuleMatch_L4PortAppInfo{
				//Controller provided port# and proto is applied on only dest
				// srcPort is not populated.
				DstPortRange: []*halproto.L4PortRange{l4port},
				//SrcPortRange:
			},
		}
	}
	if appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_ICMP) {
		appMatch.App = &halproto.RuleMatch_AppMatchInfo_IcmpInfo{
			IcmpInfo: &halproto.RuleMatch_ICMPAppInfo{
				IcmpType: 0, //TODO : Get this value from ctrler
				IcmpCode: 0, //TODO : Get this value from ctrler
			},
		}
	}
	if appPort.Ipproto == int32(halproto.IPProtocol_IPPROTO_ICMPV6) {
		appMatch.App = &halproto.RuleMatch_AppMatchInfo_Icmpv6Info{
			Icmpv6Info: &halproto.RuleMatch_ICMPv6AppInfo{
				Icmpv6Type: 0, //TODO : Get this value from ctrler
				Icmpv6Code: 0, //TODO : Get this value from ctrler
			},
		}
	}

	return appMatch
}

func buildVeniceCollectorProtoObj(mirrorSession *tsproto.MirrorSession, mirrorSessID uint32) []*halproto.MirrorSessionSpec_LocalSpanIf {
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

func buildErspanCollectorProtoObj(mirrorSession *tsproto.MirrorSession, mirrorSessID uint32) []*halproto.MirrorSessionSpec_ErspanSpec {
	var mirrorCollectors []*halproto.MirrorSessionSpec_ErspanSpec
	for _, mirrorCollector := range mirrorSession.Spec.Collectors {
		if mirrorCollector.Type == "ERSPAN" {
			//mirrorCollector.ExportCfg.Transport -- Does this info from ctrler need to be used ??
			destIPDetails := buildIPAddrDetails(mirrorCollector.ExportCfg.Destination)
			mirrorDestObj := &halproto.MirrorSessionSpec_ErspanSpec{
				ErspanSpec: &halproto.ERSpanSpec{
					DestIp: buildIPAddrProtoObj(destIPDetails),
					SpanId: mirrorSessID, //For now this value is same as mirrorSessionID.
					//SrcIp: HAL can for now choose appropriate value.
					//Dscp:  HAL can for now choose appropriate value.
				},
			}
			mirrorCollectors = append(mirrorCollectors, mirrorDestObj)
		}
	}
	return mirrorCollectors
}

func buildMirrorTrafficCollectorProtoObj(mirrorSession *tsproto.MirrorSession, mirrorSessID uint32) ([]*halproto.MirrorSessionSpec_LocalSpanIf, []*halproto.MirrorSessionSpec_ErspanSpec) {
	veniceCollectors := buildVeniceCollectorProtoObj(mirrorSession, mirrorSessID)
	erspanCollectors := buildErspanCollectorProtoObj(mirrorSession, mirrorSessID)
	return veniceCollectors, erspanCollectors
}

func createHALMirrorSessionProtoObj(tsa *Tagent, mirrorSession *tsproto.MirrorSession, sessID uint32) (*halproto.MirrorSessionRequestMsg, error) {

	tsa.DB.PktMirrorSessions[mirrorSession.Name] = &types.MirrorSessionStatus{
		Created:  false,
		MirrorID: sessID,
		Handle:   0,
	}

	veniceCollectors, erspanCollectors := buildMirrorTrafficCollectorProtoObj(mirrorSession, sessID)
	if len(veniceCollectors) == 0 && len(erspanCollectors) == 0 {
		return nil, ErrInvalidMirrorSpec
	}

	// TODO: LocalSpan/Venice; for now populate only ERSPAN collector details
	if len(erspanCollectors) == 0 {
		return nil, ErrInvalidMirrorSpec
	}

	mirrorSpec := halproto.MirrorSessionSpec{
		Meta: &halproto.ObjectMeta{
			VrfId: 0, // TODO: Map Tenant-Name to Vrf
		},
		KeyOrHandle: &halproto.MirrorSessionKeyHandle{
			KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
				MirrorsessionId: uint64(sessID),
			},
		},
		Snaplen:     mirrorSession.Spec.PacketSize,
		Destination: erspanCollectors[0], // HAL accepts only one collector per mirrorSessionSpec. TODO
	}
	ReqMsg := halproto.MirrorSessionRequestMsg{
		Request: []*halproto.MirrorSessionSpec{&mirrorSpec},
	}

	return &ReqMsg, nil
}

func createHALDropMonitorRulesProtoObj(tsa *Tagent, mirrorSession *tsproto.MirrorSession) ([]*halproto.DropMonitorRuleRequestMsg, error) {

	MsKeyOrHandle := &halproto.MirrorSessionKeyHandle{
		KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionHandle{
			MirrorsessionHandle: tsa.DB.PktMirrorSessions[mirrorSession.Name].Handle,
		},
	}
	var ReqMsgList []*halproto.DropMonitorRuleRequestMsg
	for _, filter := range mirrorSession.Spec.PacketFilters {
		ReqMsg := halproto.DropMonitorRuleRequestMsg{}
		// Iterate over drops reasons and create one ruleSpec for each drop condition.
		// HAL expects each droprule to contain only one dropreason code.
		//create array of all drop reasons
		if filter == "ALL_DROPS" {
			allDropReasons := getAllDropReasons()
			for _, dropReason := range allDropReasons {
				// create drop rule using drop reason
				dropRule := types.DropMonitorRule{
					DropReasons:       dropReason,
					DropReasonAll:     false,
					MirrorSessionName: mirrorSession.Name,
				}
				// Check if the dropRule is already applied on the same mirror session
				// If so, need to reuse ruleID
				ruleID := getDropRuleID(tsa, dropRule)
				if ruleID == 0 {
					ruleID = allocateDropRuleID(tsa, dropRule)
				}
				dropRuleSpec := halproto.DropMonitorRuleSpec{
					Meta: &halproto.ObjectMeta{
						VrfId: 0, // TODO: Map Tenant-Name to Vrf
					},
					KeyOrHandle: &halproto.DropMonitorRuleKeyHandle{
						KeyOrHandle: &halproto.DropMonitorRuleKeyHandle_DropmonitorruleId{
							DropmonitorruleId: ruleID,
						},
					},
					MsKeyHandle: []*halproto.MirrorSessionKeyHandle{MsKeyOrHandle},
					Reasons:     &dropReason,
				}
				ReqMsg.Request = append(ReqMsg.Request, &dropRuleSpec)
			}
		} else if filter == "NETWORK_POLICY_DROP" {
			nwPolicyDrops := getNetWorkPolicyDropReasons()
			for _, dropReason := range nwPolicyDrops {
				dropRule := types.DropMonitorRule{
					DropReasons:       dropReason,
					DropReasonAll:     false,
					MirrorSessionName: mirrorSession.Name,
				}
				// Check if the dropRule is already applied on the same mirror session
				// If so, need to reuse ruleID
				ruleID := getDropRuleID(tsa, dropRule)
				if ruleID == 0 {
					ruleID = allocateDropRuleID(tsa, dropRule)
				}
				dropRuleSpec := halproto.DropMonitorRuleSpec{
					Meta: &halproto.ObjectMeta{
						VrfId: 0, // TODO: Map Tenant-Name to Vrf
					},
					KeyOrHandle: &halproto.DropMonitorRuleKeyHandle{
						KeyOrHandle: &halproto.DropMonitorRuleKeyHandle_DropmonitorruleId{
							DropmonitorruleId: ruleID,
						},
					},
					MsKeyHandle: []*halproto.MirrorSessionKeyHandle{MsKeyOrHandle},
					Reasons:     &dropReason,
				}
				ReqMsg.Request = append(ReqMsg.Request, &dropRuleSpec)
			}
		} else if filter == "FIREWALL_POLICY_DROP" {
			fwPolicyDrops := getFireWallPolicyDropReasons()
			for _, dropReason := range fwPolicyDrops {
				dropRule := types.DropMonitorRule{
					DropReasons:       dropReason,
					DropReasonAll:     false,
					MirrorSessionName: mirrorSession.Name,
				}
				// Check if the dropRule is already applied on the same mirror session
				// If so, need to reuse ruleID
				ruleID := getDropRuleID(tsa, dropRule)
				if ruleID == 0 {
					ruleID = allocateDropRuleID(tsa, dropRule)
				}
				dropRuleSpec := halproto.DropMonitorRuleSpec{
					Meta: &halproto.ObjectMeta{
						VrfId: 0, // TODO: Map Tenant-Name to Vrf
					},
					KeyOrHandle: &halproto.DropMonitorRuleKeyHandle{
						KeyOrHandle: &halproto.DropMonitorRuleKeyHandle_DropmonitorruleId{
							DropmonitorruleId: ruleID,
						},
					},
					MsKeyHandle: []*halproto.MirrorSessionKeyHandle{MsKeyOrHandle},
					Reasons:     &dropReason,
				}
				ReqMsg.Request = append(ReqMsg.Request, &dropRuleSpec)
			}
		}
		ReqMsgList = append(ReqMsgList, &ReqMsg)
	}
	return ReqMsgList, nil
}

func createHALFlowMonitorRulesProtoObj(tsa *Tagent, mirrorSession *tsproto.MirrorSession) ([]*halproto.FlowMonitorRuleRequestMsg, error) {

	var ReqMsgList []*halproto.FlowMonitorRuleRequestMsg

	for _, rule := range mirrorSession.Spec.MatchRules {
		srcIPs, destIPs, srcMACs, destMACs, appPorts := expandCompositeMatchRule(&rule)
		// Create protobuf requestMsgs on cross product of
		//  - srcIPs, destIPs, Apps
		//  - srcMACs, destMACs, Apps
		flowMonitorRuleSpecified := false
		ipFmRuleList, err := createIPAddrCrossProductRuleList(srcIPs, destIPs, appPorts)
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
			return nil, ErrInvalidFlowMonitorRule
		}

		ReqMsg := halproto.FlowMonitorRuleRequestMsg{}
		var srcIP [16]byte
		var destIP [16]byte
		for _, ipFmRule := range ipFmRuleList {
			copy(srcIP[:], ipFmRule.SrcIPObj.IP)
			copy(destIP[:], ipFmRule.DestIPObj.IP)
			flowRule := types.FlowMonitorRule{
				SourceIP: srcIP,
				DestIP:   destIP,
				//SourceMac:         0,
				//DestMac:           0,
				//EtherType:         0,
				Protocol:     uint32(ipFmRule.AppPortObj.Ipproto),
				SourceL4Port: uint32(ipFmRule.AppPortObj.L4port),
				DestL4Port:   uint32(ipFmRule.AppPortObj.L4port),
				//SourceGroupID:     0,
				//DestGroupID:       0,
				MirrorSessionName: mirrorSession.Name,
			}
			// Check if the FlowRule is already applied on the same mirror session
			// If so, need to reuse ruleID
			ruleID := getFlowMonitorRuleID(tsa, flowRule)
			if ruleID == 0 {
				ruleID = allocateFlowMonitorRuleID(tsa, flowRule)
			}
			srcAddrObj := buildIPAddrObjProtoObj(ipFmRule.SrcIPObj)
			destAddrObj := buildIPAddrObjProtoObj(ipFmRule.SrcIPObj)
			appMatchObj := buildAppMatchInfoObj(ipFmRule.AppPortObj)
			flowRuleSpec := halproto.FlowMonitorRuleSpec{
				Meta: &halproto.ObjectMeta{
					VrfId: 0, // TODO: Map Tenant-Name to Vrf
				},
				KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
					KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
						FlowmonitorruleId: ruleID,
					},
				},
				Match: &halproto.RuleMatch{
					SrcAddress: []*halproto.IPAddressObj{srcAddrObj},
					DstAddress: []*halproto.IPAddressObj{destAddrObj},
					Protocol:   halproto.IPProtocol(ipFmRule.AppPortObj.Ipproto),
					AppMatch:   []*halproto.RuleMatch_AppMatchInfo{appMatchObj},
				},
				Action: &halproto.MonitorAction{},
			}
			ReqMsg.Request = append(ReqMsg.Request, &flowRuleSpec)
		}

		for _, macRule := range macFmRuleList {
			flowRule := types.FlowMonitorRule{
				SourceIP:  [16]byte{0},
				DestIP:    [16]byte{0},
				SourceMac: macRule.SrcMAC,
				DestMac:   macRule.DestMAC,
				//EtherType: 0,
				// ??? : Does it make sense to use proto, l4ports when src/dest MAC based matching is used ?
				Protocol:     uint32(macRule.AppPortObj.Ipproto),
				SourceL4Port: uint32(macRule.AppPortObj.L4port),
				DestL4Port:   uint32(macRule.AppPortObj.L4port),
				//SourceGroupID:     0,
				//DestGroupID:       0,
				MirrorSessionName: mirrorSession.Name,
			}
			// Check if the FlowRule is already applied on the same mirror session
			// If so, need to reuse ruleID
			ruleID := getFlowMonitorRuleID(tsa, flowRule)
			if ruleID == 0 {
				ruleID = allocateFlowMonitorRuleID(tsa, flowRule)
			}
			appMatchObj := buildAppMatchInfoObj(macRule.AppPortObj)
			flowRuleSpec := halproto.FlowMonitorRuleSpec{
				Meta: &halproto.ObjectMeta{
					VrfId: 0, // TODO: Map Tenant-Name to Vrf
				},
				KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
					KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
						FlowmonitorruleId: ruleID,
					},
				},
				Match: &halproto.RuleMatch{
					SrcMacAddress: []uint64{macRule.SrcMAC},
					DstMacAddress: []uint64{macRule.DestMAC},
					Protocol:      halproto.IPProtocol(macRule.AppPortObj.Ipproto),
					AppMatch:      []*halproto.RuleMatch_AppMatchInfo{appMatchObj},
				},
				Action: &halproto.MonitorAction{},
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
	return ReqMsgList, nil
}

// CreatePacketCaptureSessionProtoObjects creates all proto objects needed to create mirror session.
func createPacketCaptureSessionProtoObjs(tsa *Tagent, mirrorSession *tsproto.MirrorSession) (*halproto.MirrorSessionRequestMsg, []*halproto.FlowMonitorRuleRequestMsg, []*halproto.DropMonitorRuleRequestMsg, error) {
	var flowRuleProtoObjs []*halproto.FlowMonitorRuleRequestMsg
	var dropRuleProtoObjs []*halproto.DropMonitorRuleRequestMsg
	var mirrorSessionProtoObjs *halproto.MirrorSessionRequestMsg

	var err error

	if mirrorSession.Name == "" {
		return nil, nil, nil, ErrInvalidMirrorSpec
	}

	// Check if flow rules or drop rules needs to be processed.
	dropMonitor := true
	if len(mirrorSession.Spec.MatchRules) > 0 {
		dropMonitor = false
	}
	if !dropMonitor && !matchRuleSanityCheck(mirrorSession) {
		// Rule santy check failed.
		return nil, nil, nil, ErrInvalidMirrorSpec
	}

	createMirrorSession := true
	sessID := getMirrorSessionID(tsa, mirrorSession.Name)
	if sessID < halMaxMirrorSession {
		if tsa.DB.PktMirrorSessions[mirrorSession.Name].Created &&
			tsa.DB.PktMirrorSessions[mirrorSession.Name].Handle != 0 {
			// mirror create already posted to HAL.
			createMirrorSession = false
		}
	} else {
		sessID = allocateMirrorSessionID(tsa, mirrorSession.Name)
		if sessID > halMaxMirrorSession {
			return nil, nil, nil, ErrMirrorSpecResource
		}
	}

	//Mirror Session  proto objs
	if createMirrorSession {
		mirrorSessionProtoObjs, err = createHALMirrorSessionProtoObj(tsa, mirrorSession, sessID)
		if err != nil {
			return nil, nil, nil, err
		}
	}

	// FlowMonitor rules
	flowRuleProtoObjs, err = createHALFlowMonitorRulesProtoObj(tsa, mirrorSession)
	if err != nil {
		if err != nil {
			return nil, nil, nil, err
		}
	}
	// Drop monitor rules
	if dropMonitor {
		dropRuleProtoObjs, err = createHALDropMonitorRulesProtoObj(tsa, mirrorSession)
		if err != nil {
			return nil, nil, nil, err
		}
	}
	return mirrorSessionProtoObjs, flowRuleProtoObjs, dropRuleProtoObjs, err
}

// CreatePacketCaptureSession creates mirror session to enable packet capture
func (tsa *Tagent) CreatePacketCaptureSession(pcSession *tsproto.MirrorSession) error {
	log.Infof("Processing create packet capture session")
	mirrorProtoObj, flowRuleProtoObj, dropRuleProtoObj, err := createPacketCaptureSessionProtoObjs(tsa, pcSession)
	if err == nil {
		return tsa.Datapath.CreatePacketCaptureSession(pcSession.Name, &tsa.DB, mirrorProtoObj, flowRuleProtoObj, dropRuleProtoObj)
	}
	return err
}

// ListPacketCaptureSession lists all mirror sessions
func (tsa *Tagent) ListPacketCaptureSession() []*tsproto.MirrorSession {
	log.Infof("Processing list packet capture session")
	return nil
}

// UpdatePacketCaptureSession updates mirror session
func (tsa *Tagent) UpdatePacketCaptureSession(pcSession *tsproto.MirrorSession) error {
	log.Infof("Processing update packet capture session")
	return tsa.Datapath.UpdatePacketCaptureSession(pcSession)
}

// DeletePacketCaptureSession deletes packet capture session.
func (tsa *Tagent) DeletePacketCaptureSession(pcSession *tsproto.MirrorSession) error {
	log.Infof("Processing delete packet capture session")
	return tsa.Datapath.DeletePacketCaptureSession(pcSession)
}
