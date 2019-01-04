package state

import (
	"context"
	"encoding/json"
	"fmt"
	"net"
	"net/http"
	"os"
	"path/filepath"
	"reflect"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/venice/globals"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	agstate "github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/tpa/state/types"
	tstype "github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/nic/agent/troubleshooting/utils"
	"github.com/pensando/sw/venice/ctrler/tpm/rpcserver/protos"
	"github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

const (
	flowExportPolicyID      = "flowExportPolicyId"
	maxFlowExportCollectors = 16
	defaultDbgSock          = "/var/run/pensando/tpa.sock"
)

// PolicyState keeps the agent state
type PolicyState struct {
	// global lock
	sync.Mutex
	netAgent *agstate.Nagent
	store    emstore.Emstore
	hal      halproto.TelemetryClient
}

// internal state from db
type policyDb struct {
	state *PolicyState
	//validated matchrules for the current policy
	matchRules []tsproto.MatchRule
	// collector keys for the current policy
	collectorKeys map[types.CollectorKey]bool
	//FlowMonitorRule keys for the current policy
	flowRuleKeys map[types.FlowMonitorRuleKey]bool
	// vrf for the current policy
	vrf uint64
	//object meta
	objMeta api.ObjectMeta

	// policy from db
	tpmPolicy *types.FlowExportPolicyTable
	// collectors saved in db
	collectorTable *types.CollectorTable
	// flowmonitor saved in db
	flowMonitorTable *types.FlowMonitorTable
}

// NewTpAgent creates new telemetry policy agent state
func NewTpAgent(netAgent *agstate.Nagent, halTm halproto.TelemetryClient, dbgSock string) (*PolicyState, error) {
	state := &PolicyState{
		store:    netAgent.Store,
		netAgent: netAgent,
		hal:      halTm,
	}

	// debug
	if dbgSock == "" {
		dbgSock = defaultDbgSock
	}
	router := mux.NewRouter()
	router.HandleFunc("/debug", state.debug).Methods("GET")
	// sudo curl --unix-socket /var/run/pensando/tpa.sock http://localhost/debug
	os.Remove(dbgSock)
	os.MkdirAll(filepath.Dir(dbgSock), 0644)
	l, err := net.Listen("unix", dbgSock)
	if err != nil {

		log.Fatalf("failed to initialize debug, %s", err)
	}

	go func() {
		log.Fatal(http.Serve(l, router))
	}()

	return state, nil
}

// Close closes all policy agent resources
func (s *PolicyState) Close() {
}

func (s *PolicyState) validateMeta(p *tpmprotos.FlowExportPolicy) error {
	if len(strings.TrimSpace(p.Name)) == 0 || len(strings.TrimSpace(p.Kind)) == 0 {
		return fmt.Errorf("name/kind can't be empty")
	}

	if p.Tenant == "" {
		p.Tenant = globals.DefaultTenant
	}

	if p.Namespace == "" {
		p.Namespace = globals.DefaultNamespace
	}
	return nil
}

// count collectors in policydb
func (p *policyDb) findNumCollectors() int {
	// configured collectors
	numCollectors := len(p.collectorTable.Collector)

	// check if there are any new collectors
	for key := range p.collectorKeys {
		if _, ok := p.collectorTable.Collector[key.String()]; !ok {
			numCollectors++
		}
	}

	return numCollectors
}

func parsePortProto(src string) (halproto.IPProtocol, uint32, error) {
	s := strings.Split(src, "/")
	if len(s) != 2 {
		return halproto.IPProtocol_IPPROTO_NONE, 0, fmt.Errorf("invalid protocol/port in %s", src)
	}

	p, ok := halproto.IPProtocol_value["IPPROTO_"+strings.ToUpper(s[0])]
	if !ok {
		return halproto.IPProtocol_IPPROTO_NONE, 0, fmt.Errorf("invalid protocol in %s", src)
	}
	proto := halproto.IPProtocol(p)

	switch proto {
	case halproto.IPProtocol_IPPROTO_UDP:
	case halproto.IPProtocol_IPPROTO_TCP:

	default:
		return halproto.IPProtocol_IPPROTO_NONE, 0, fmt.Errorf("invalid protocol in %s", src)
	}

	port, err := strconv.Atoi(s[1])
	if err != nil {
		return halproto.IPProtocol_IPPROTO_NONE, 0, fmt.Errorf("invalid port in %s", src)
	}
	if uint(port) > uint(^uint16(0)) {
		return halproto.IPProtocol_IPPROTO_NONE, 0, fmt.Errorf("invalid port in %s", src)
	}

	return proto, uint32(port), nil
}

func (s *PolicyState) validatePolicy(p *tpmprotos.FlowExportPolicy) (map[types.CollectorKey]bool, error) {
	if err := s.validateMeta(p); err != nil {
		return nil, err
	}
	spec := p.Spec

	interval, err := time.ParseDuration(spec.Interval)
	if err != nil {
		return nil, fmt.Errorf("invalid interval %s", spec.Interval)
	}

	if interval < time.Second {
		return nil, fmt.Errorf("too small interval %s", spec.Interval)
	}

	if interval > 24*time.Hour {
		return nil, fmt.Errorf("too large interval %s", spec.Interval)
	}

	if halproto.ExportFormat_name[int32(halproto.ExportFormat_IPFIX)] != strings.ToUpper(spec.Format) {
		return nil, fmt.Errorf("invalid format %s", spec.Format)
	}

	if len(spec.MatchRules) == 0 {
		return nil, fmt.Errorf("no matchrule in %s", p.Name)
	}

	if err := utils.ValidateMatchRule(p.ObjectMeta, spec.MatchRules, s.netAgent.FindEndpoint); err != nil {
		return nil, fmt.Errorf("error in matchrule in %s, %s", p.Name, err)
	}

	if len(spec.Exports) == 0 {
		return nil, fmt.Errorf("exports can't be empty")
	}

	// get vrf
	vrf, err := s.getVrfID(p.GetTenant(), p.GetNamespace())
	if err != nil {
		return nil, fmt.Errorf("failed to find vrf for %s/%s", p.GetTenant(), p.GetNamespace())
	}

	collKeys := map[types.CollectorKey]bool{}
	for _, export := range spec.Exports {
		dest := export.Destination
		if dest == "" {
			return nil, fmt.Errorf("destination can't be empty")
		}

		netIP := net.ParseIP(dest)
		if netIP == nil {
			// treat it as hostname and resolve
			s, err := net.LookupHost(dest)
			if err != nil || len(s) == 0 {
				return nil, fmt.Errorf("failed to resolve name {%s}, error: %s", dest, err)
			}
			dest = s[0] // pick the first address from dns
		}

		proto, port, err := parsePortProto(export.Transport)
		if err != nil {
			return nil, err
		}

		newKey := types.CollectorKey{
			Interval:    uint32(interval.Seconds()),
			Format:      halproto.ExportFormat_IPFIX,
			Protocol:    proto,
			Destination: dest,
			Port:        port,
			VrfID:       vrf,
		}
		collKeys[newKey] = true
	}

	return collKeys, nil
}

// get vrf from tenant/namespace
func (s *PolicyState) getVrfID(tenant string, namespace string) (uint64, error) {
	nsObj, err := s.netAgent.FindNamespace(tenant, namespace)
	if err != nil {
		log.Errorf("failed to find tenant/namespace {%s/%s}", tenant, namespace)
		return uint64(0), fmt.Errorf("failed to find tenant, %s", err)
	}
	return nsObj.Status.NamespaceID, nil
}

// get vrf from tenant/namespace
func (s *PolicyState) getL2SegID(tenant, namespace, address string) (*netproto.NetworkStatus, error) {
	epList := s.netAgent.ListEndpoint()
	for _, ep := range epList {
		epAddr, _, err := net.ParseCIDR(ep.Spec.GetIPv4Address())
		if err != nil {
			log.Errorf("failed to parse endpoint address {%+v} ", ep)
			continue
		}

		if ep.ObjectMeta.Tenant == tenant && ep.ObjectMeta.Namespace == namespace &&
			epAddr.String() == address {
			netName := ep.Spec.GetNetworkName()
			netObj, err := s.netAgent.FindNetwork(api.ObjectMeta{Tenant: tenant, Namespace: namespace, Name: netName})
			if err != nil {
				return nil, fmt.Errorf("failed to find network %s of this {%s}", netName, address)
			}
			return &netObj.Status, nil
		}
	}
	return nil, fmt.Errorf("failed to find l2 segment id for %s", address)
}

func convertToHalIPAddr(src string) (*halproto.IPAddress, string, error) {
	netIP := net.ParseIP(src)
	if netIP == nil {
		// treat it as hostname and resolve
		s, err := net.LookupHost(src)
		if err != nil {
			return nil, src, fmt.Errorf("failed to resolve name {%s}, error: %s", src, err)
		}
		src = s[0] // pick the first address from dns
	}

	halIP := &halproto.IPAddress{}
	if ip, err := netutils.IPv4ToUint32(src); err == nil { // ipv4
		halIP.IpAf = halproto.IPAddressFamily_IP_AF_INET
		halIP.V4OrV6 = &halproto.IPAddress_V4Addr{
			V4Addr: ip,
		}
		return halIP, src, nil
	}

	halIP.IpAf = halproto.IPAddressFamily_IP_AF_INET6
	halIP.V4OrV6 = &halproto.IPAddress_V6Addr{
		V6Addr: netIP,
	}
	return halIP, src, nil
}

// createCollectorPolicy creates collector policy in HAL
func (p *policyDb) createCollectorPolicy(ctx context.Context) (err error) {

	// todo: move to a function
	defer func() {
		if err != nil {
			log.Infof("cleaning up collectors")
			for ckey := range p.collectorKeys {
				if cdata, ok := p.collectorTable.Collector[ckey.String()]; ok {
					delete(cdata.PolicyNames, getObjMetaKey(&p.objMeta))
					if len(cdata.PolicyNames) == 0 { // no one is using it
						log.Infof("removing collector {%+v}", ckey)
						p.deleteCollectorPolicy(ctx, cdata.CollectorID)
						p.writeCollectorTable()
					}
				}
			}
		}
	}()

	// todo: get local ip from mnic
	var halSrcAddr *halproto.IPAddress

	if ep, err := p.state.netAgent.FindLocalEndpoint(p.objMeta.Tenant, p.objMeta.Namespace); err == nil {
		epAddr := ep.Spec.GetIPv4Address()
		if epAddr == "" {
			// pick ipv6
			epAddr = ep.Spec.GetIPv6Address()
		}
		srcAddr, _, err := net.ParseCIDR(epAddr)
		if err != nil {
			log.Errorf("failed to parse local endpoint address {%+v} ", epAddr)
			return fmt.Errorf("invalid local endpoint address %s, %s", srcAddr, err)
		}

		halSrcAddr, _, err = convertToHalIPAddr(srcAddr.String())
		if err != nil {
			return fmt.Errorf("invalid source address %s, %s", srcAddr, err)
		}
	}

	for ckey := range p.collectorKeys {
		// check if collector already exists
		if cdata, ok := p.collectorTable.Collector[ckey.String()]; ok {
			log.Infof("collector %s exists", ckey.Destination)
			cdata.PolicyNames[getObjMetaKey(&p.objMeta)] = true
			continue
		}

		halDestAddr, destAddr, err := convertToHalIPAddr(ckey.Destination)
		if err != nil {
			return fmt.Errorf("invalid destination address %s, %s", ckey.Destination, err)
		}

		netObj, err := p.state.getL2SegID(p.objMeta.Tenant, p.objMeta.Namespace, destAddr)
		if err != nil {
			return fmt.Errorf("invalid l2 segment, %s", err)
		}

		collectorID, err := p.state.store.GetNextID(flowExportPolicyID)
		if err != nil {
			return fmt.Errorf("failed to allocate object id for collector policy")
		}

		log.Infof("create new collector %v(policy %v), id %d", destAddr, p.objMeta.Name, collectorID)

		var req []*halproto.CollectorSpec
		req = append(req, &halproto.CollectorSpec{
			KeyOrHandle: &halproto.CollectorKeyHandle{
				KeyOrHandle: &halproto.CollectorKeyHandle_CollectorId{
					CollectorId: collectorID,
				},
			},
			VrfKeyHandle: &halproto.VrfKeyHandle{
				KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
					VrfId: p.vrf,
				},
			},
			//  TODO: will be removed from HAL
			Encap: &halproto.EncapInfo{
				EncapType:  halproto.EncapType_ENCAP_TYPE_DOT1Q,
				EncapValue: netObj.AllocatedVlanID,
			},

			L2SegKeyHandle: &halproto.L2SegmentKeyHandle{
				KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
					SegmentId: netObj.NetworkID,
				},
			},

			//todo: set src ip from mnic
			SrcIp:          halSrcAddr,
			DestIp:         halDestAddr,
			Protocol:       ckey.Protocol,
			DestPort:       ckey.Port,
			ExportInterval: ckey.Interval,
			Format:         ckey.Format,
		})

		log.Infof("create hal collector policy, req: {%+v}", req)

		// send to hal
		resp, err := p.state.hal.CollectorCreate(ctx, &halproto.CollectorRequestMsg{Request: req})
		if err != nil {
			log.Errorf("failed to create collector policy in hal, %s", err)
			return err
		}

		if resp != nil && len(resp.Response) > 0 && resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("failed to create collector policy in hal, api returned %v", resp.Response[0].ApiStatus)
			return fmt.Errorf("api error %v", resp.Response[0].ApiStatus)
		}

		// save table in db
		p.collectorTable.Collector[ckey.String()] = &types.CollectorData{
			CollectorID: collectorID,
			PolicyNames: map[string]bool{
				getObjMetaKey(&p.objMeta): true,
			},
		}
		p.writeCollectorTable()
	}

	return nil
}

// deleteCollectorPolicy deletes collector policy in HAL
func (p *policyDb) deleteCollectorPolicy(ctx context.Context, key uint64) error {
	log.Infof("delete collector id: %+v", key)

	// ignore invalid key
	if key == 0 {
		return nil
	}

	var req []*halproto.CollectorDeleteRequest

	req = append(req, &halproto.CollectorDeleteRequest{
		KeyOrHandle: &halproto.CollectorKeyHandle{
			KeyOrHandle: &halproto.CollectorKeyHandle_CollectorId{
				CollectorId: key,
			},
		}})

	// send to hal
	log.Infof("delete hal collector policy {%+v}", req)
	resp, err := p.state.hal.CollectorDelete(ctx, &halproto.CollectorDeleteRequestMsg{Request: req})
	if err != nil {
		log.Errorf("failed to delete hal collector policy, %s", err)
		return err
	}

	// log error and return success
	if resp != nil && len(resp.Response) > 0 && resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to delete hal collector policy, api returned %v", resp.Response[0].ApiStatus)
		return fmt.Errorf("api error  %v", resp.Response[0].ApiStatus)
	}

	return nil
}

func (p *policyDb) deleteFlowMonitorRule(ctx context.Context, ruleID uint64) error {
	log.Infof("delete flow monitor rule: %+v", ruleID)

	// ignore invalid key
	if ruleID == 0 {
		return nil
	}

	var req []*halproto.FlowMonitorRuleDeleteRequest
	req = append(req, &halproto.FlowMonitorRuleDeleteRequest{
		KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
			KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
				FlowmonitorruleId: ruleID,
			},
		},
		VrfKeyHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: p.vrf,
			},
		},
	})

	log.Infof("delete hal flow monitor, req: {%+v}", req)
	resp, err := p.state.hal.FlowMonitorRuleDelete(ctx, &halproto.FlowMonitorRuleDeleteRequestMsg{
		Request: req,
	})

	if err != nil {
		log.Errorf("failed to delete hal flow monitor rule policy, %s", err)
		return err
	}

	if resp != nil && len(resp.Response) > 0 && resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to delete hal flow monitor rule policy, api returned %v", resp.Response[0].ApiStatus)
		return fmt.Errorf("api error %v", resp.Response[0].ApiStatus)
	}

	return nil
}

func (p *policyDb) createHalFlowMonitorRule(ctx context.Context, ruleKey types.FlowMonitorRuleKey) error {
	// create rule id
	var ruleID uint64
	var err error

	log.Infof("create new rule %+v ", ruleKey)
	ruleID, err = p.state.store.GetNextID(tstype.FlowMonitorRuleIDType)
	if err != nil {
		log.Errorf("failed to allocate object id for flow monitor %s", err)
		return fmt.Errorf("failed to allocate object id for flow monitor %s", err)
	}

	//todo: delete only if there is any change in collector config
	flowRuleData, ok := p.flowMonitorTable.FlowRules[ruleKey.String()]
	if ok {
		log.Infof("rule already exists, %+v ", ruleKey)
		// delete and set new id
		p.deleteFlowMonitorRule(ctx, flowRuleData.RuleID)
		flowRuleData.RuleID = ruleID
	} else {
		flowRuleData = &types.FlowMonitorData{
			RuleID:      ruleID,
			RuleKey:     ruleKey,
			Collectors:  map[string]map[string]bool{},
			PolicyNames: map[string]bool{},
		}
		p.flowMonitorTable.FlowRules[ruleKey.String()] = flowRuleData
		flowRuleData.Collectors[getObjMetaKey(&p.objMeta)] = map[string]bool{}
	}
	p.flowRuleKeys[ruleKey] = true
	flowRuleData.PolicyNames[getObjMetaKey(&p.objMeta)] = true

	srcIPObj := utils.BuildIPAddrDetails(ruleKey.SourceIP)
	destIPObj := utils.BuildIPAddrDetails(ruleKey.DestIP)
	appPortObj := &tstype.AppPortDetails{
		Ipproto: int32(ruleKey.Protocol),
		L4port:  int32(ruleKey.DestL4Port),
	}

	srcAddrObj := utils.BuildIPAddrObjProtoObj(srcIPObj)
	destAddrObj := utils.BuildIPAddrObjProtoObj(destIPObj)
	appMatchObj := utils.BuildAppMatchInfoObj(appPortObj)

	// create collector keys for hal
	collectorKeys := []*halproto.CollectorKeyHandle{}

	// add new collectors
	for ckey := range p.collectorKeys {
		if cdata, ok := p.collectorTable.Collector[ckey.String()]; ok {
			collectorKeys = append(collectorKeys, &halproto.CollectorKeyHandle{
				KeyOrHandle: &halproto.CollectorKeyHandle_CollectorId{
					CollectorId: cdata.CollectorID,
				}})
		}
	}

	// now pick up existing collectors
	for _, cmap := range flowRuleData.Collectors {
		for ckey := range cmap {
			if cdata, ok := p.collectorTable.Collector[ckey]; ok {
				collectorKeys = append(collectorKeys, &halproto.CollectorKeyHandle{
					KeyOrHandle: &halproto.CollectorKeyHandle_CollectorId{
						CollectorId: cdata.CollectorID,
					}})
			}
		}
	}

	flowRuleSpec := halproto.FlowMonitorRuleSpec{
		KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
			KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
				FlowmonitorruleId: ruleID,
			},
		},
		VrfKeyHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: p.vrf,
			},
		},

		Match: &halproto.RuleMatch{
			SrcMacAddress: []uint64{ruleKey.SourceMac},
			DstMacAddress: []uint64{ruleKey.DestMac},
			SrcAddress:    []*halproto.IPAddressObj{srcAddrObj},
			DstAddress:    []*halproto.IPAddressObj{destAddrObj},
			Protocol:      halproto.IPProtocol(appPortObj.Ipproto),
			AppMatch:      appMatchObj,
		},
		CollectorKeyHandle: collectorKeys,

		Action: &halproto.MonitorAction{
			Action: []halproto.RuleAction{halproto.RuleAction_COLLECT_FLOW_STATS},
		},
	}
	reqMsg := &halproto.FlowMonitorRuleRequestMsg{Request: []*halproto.FlowMonitorRuleSpec{&flowRuleSpec}}

	// program hal
	log.Infof("create hal flow monitor, req: {%+v}", reqMsg)
	resp, err := p.state.hal.FlowMonitorRuleCreate(ctx, reqMsg)
	if err != nil {
		log.Errorf("failed to create hal flow monitor rule policy, %s", err)
		return err
	}

	if resp != nil && len(resp.Response) > 0 && resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to create hal flow monitor rule policy, api returned %v", resp.Response[0].ApiStatus)
		return fmt.Errorf("api error %v", resp.Response[0].ApiStatus)
	}

	// update collectors in flowrule
	if policyData, ok := flowRuleData.Collectors[getObjMetaKey(&p.objMeta)]; ok {
		for ckey := range p.collectorKeys {
			policyData[ckey.String()] = true
		}
	}

	// save
	p.writeCollectorTable()
	p.writeFlowMonitorTable()
	return nil

}

func (p *policyDb) createIPFlowMonitorRule(ctx context.Context, ipFmRuleList []*tstype.FlowMonitorIPRuleDetails) error {
	for _, ipFmRule := range ipFmRuleList {
		ruleKey := types.FlowMonitorRuleKey{
			SourceIP:     ipFmRule.SrcIPString,
			DestIP:       ipFmRule.DestIPString,
			Protocol:     uint32(ipFmRule.AppPortObj.Ipproto),
			SourceL4Port: uint32(ipFmRule.AppPortObj.L4port), // not sent to hal
			DestL4Port:   uint32(ipFmRule.AppPortObj.L4port),
			VrfID:        p.vrf,
		}

		if err := p.createHalFlowMonitorRule(ctx, ruleKey); err != nil {
			log.Errorf("failed to create ip rule %+v in hal, %s", ruleKey, err)
			return err
		}
	}
	return nil
}

func (p *policyDb) createMacFlowMonitorRule(ctx context.Context, macFmRuleList []*tstype.FlowMonitorMACRuleDetails) error {
	for _, macRule := range macFmRuleList {
		ruleKey := types.FlowMonitorRuleKey{
			SourceIP:  "",
			DestIP:    "",
			SourceMac: macRule.SrcMAC,
			DestMac:   macRule.DestMAC,
			//EtherType: 0,
			// ??? : Does it make sense to use proto, l4ports when src/dest MAC based matching is used ?
			Protocol:     uint32(macRule.AppPortObj.Ipproto),
			SourceL4Port: uint32(macRule.AppPortObj.L4port), //  not sent to hal
			DestL4Port:   uint32(macRule.AppPortObj.L4port),
			VrfID:        p.vrf,
		}

		if err := p.createHalFlowMonitorRule(ctx, ruleKey); err != nil {
			log.Errorf("failed to create ip rule %+v in hal, %s", ruleKey, err)
			return err
		}
	}
	return nil
}

func (p *policyDb) createFlowMonitorRule(ctx context.Context) (err error) {

	// configure collector policy
	if err := p.createCollectorPolicy(ctx); err != nil {
		log.Errorf("failed to create collector policy, error: %s", err)
		return fmt.Errorf("failed to create collector policy, %s", err)
	}

	defer func() {
		if err != nil {
			p.cleanupTables(ctx, false)
			p.writeCollectorTable()
			p.writeFlowMonitorTable()
		}
	}()

	p.flowRuleKeys = map[types.FlowMonitorRuleKey]bool{}
	// process match-rule
	for _, rule := range p.matchRules {
		srcIPs, destIPs, srcMACs, destMACs, appPorts, srcIPStrings, destIPStrings := utils.ExpandCompositeMatchRule(p.collectorTable.ObjectMeta, &rule, p.state.netAgent.FindEndpoint)
		// Create protobuf requestMsgs on cross product of
		//  - srcIPs, destIPs, Apps
		//  - srcMACs, destMACs, Apps
		ipFmRuleList, _ := utils.CreateIPAddrCrossProductRuleList(srcIPs, destIPs, appPorts, srcIPStrings, destIPStrings)
		macFmRuleList, _ := utils.CreateMACAddrCrossProductRuleList(srcMACs, destMACs, appPorts)

		//TODO: Fold in ether-type in monitor rule
		if len(ipFmRuleList) == 0 && len(macFmRuleList) == 0 {
			log.Errorf("Match Rules specified with only AppPort Selector without IP/MAC")
			return utils.ErrInvalidFlowMonitorRule
		}

		if err := p.createIPFlowMonitorRule(ctx, ipFmRuleList); err != nil {
			log.Errorf("failed to create ip flow monitor rule %s", err)
			return fmt.Errorf("failed to create ip flow monitor rule %s", err)
		}

		if err := p.createMacFlowMonitorRule(ctx, macFmRuleList); err != nil {
			log.Errorf("failed to create mac flow monitor rule %s", err)
			return fmt.Errorf("failed to create mac flow monitor rule %s", err)
		}
	}

	return nil
}

func (p *policyDb) readFlowMonitorTable() (*types.FlowMonitorTable, error) {
	fmObj := &types.FlowMonitorTable{
		TypeMeta:  api.TypeMeta{Kind: "tpaFlowMonitorTable"},
		FlowRules: map[string]*types.FlowMonitorData{},
	}

	obj, err := p.state.store.Read(fmObj)
	if err != nil {
		log.Warnf("failed to read FlowMonitor table, %s", err)
		return fmObj, nil
	}

	dbObj, ok := obj.(*types.FlowMonitorTable)
	if ok != true {
		log.Errorf("invalid flow monitor object in db for %+v", fmObj.ObjectMeta)
		return fmObj, nil
	}

	return dbObj, nil
}

func (p *policyDb) writeFlowMonitorTable() (err error) {
	if err := p.state.store.Write(p.flowMonitorTable); err != nil {
		return fmt.Errorf("failed to write  flowmonitor object in db, %s", err)
	}
	return nil
}

func (p *policyDb) readCollectorTable() (*types.CollectorTable, error) {
	collObj := &types.CollectorTable{
		TypeMeta:  api.TypeMeta{Kind: "tpacollectorTable"},
		Collector: map[string]*types.CollectorData{},
	}

	obj, err := p.state.store.Read(collObj)
	if err != nil {
		log.Warnf("failed to read collector table, %s", err)
		return collObj, nil
	}

	dbObj, ok := obj.(*types.CollectorTable)
	if ok != true {
		log.Errorf("invalid collector object in db for %+v", collObj.ObjectMeta)
		return collObj, nil
	}
	return dbObj, nil
}

func (p *policyDb) writeCollectorTable() (err error) {
	if err := p.state.store.Write(p.collectorTable); err != nil {
		return fmt.Errorf("failed to write  collector object in db, %s", err)
	}
	return nil
}

func (p *policyDb) readFlowExportPolicyTable(tpmPolicy *tpmprotos.FlowExportPolicy) (*types.FlowExportPolicyTable, error) {
	obj, err := p.state.store.Read(&types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta:   tpmPolicy.TypeMeta,
			ObjectMeta: tpmPolicy.ObjectMeta,
		}})
	if err != nil {
		return nil, fmt.Errorf("failed to read collectors, %s", err)
	}

	flowObj, ok := obj.(*types.FlowExportPolicyTable)
	if ok != true {
		return nil, fmt.Errorf("invalid collector object in db")
	}

	return flowObj, nil
}

func (p *policyDb) writeFlowExportPolicyTable(fp *tpmprotos.FlowExportPolicy) (err error) {

	ckey := map[string]bool{}
	for k, v := range p.collectorKeys {
		ckey[k.String()] = v
	}

	fkey := map[string]bool{}
	for k, v := range p.flowRuleKeys {
		fkey[k.String()] = v
	}

	polObj := &types.FlowExportPolicyTable{
		Vrf:              p.vrf,
		CollectorKeys:    ckey,
		FlowRuleKeys:     fkey,
		FlowExportPolicy: fp,
	}

	if err := p.state.store.Write(polObj); err != nil {
		return fmt.Errorf("failed to write flow monitor object in db, %s", err)
	}

	return nil
}

func (s *PolicyState) createPolicyContext(p *tpmprotos.FlowExportPolicy) (*policyDb, error) {
	policyCtx := &policyDb{
		objMeta:       p.ObjectMeta,
		state:         s,
		flowRuleKeys:  map[types.FlowMonitorRuleKey]bool{},
		collectorKeys: map[types.CollectorKey]bool{},
		matchRules:    p.Spec.MatchRules,
	}

	polObj, err := policyCtx.readFlowExportPolicyTable(p)
	if err != nil {
		log.Warnf("flow export policy %s doesn't exist ", p.Name)

	} else {
		policyCtx.tpmPolicy = polObj
	}

	// read collector object
	policyCtx.collectorTable, _ = policyCtx.readCollectorTable()

	// read flowmonitor object
	policyCtx.flowMonitorTable, _ = policyCtx.readFlowMonitorTable()

	// get vrf
	vrf, err := s.getVrfID(p.GetTenant(), p.GetNamespace())
	if err != nil {
		return nil, fmt.Errorf("failed to find tenant/namespace, %s/%s", p.GetTenant(), p.GetNamespace())
	}
	policyCtx.vrf = vrf

	return policyCtx, nil
}

// CreateFlowExportPolicy is the POST() entry point
func (s *PolicyState) CreateFlowExportPolicy(ctx context.Context, p *tpmprotos.FlowExportPolicy) error {
	log.Infof("POST: %+v", p)

	collKeys, err := s.validatePolicy(p)
	if err != nil {
		log.Errorf("invalid policy, %s", err)
		return err
	}

	s.Lock()
	defer s.Unlock()

	policyCtx, err := s.createPolicyContext(p)
	if err != nil {
		log.Errorf("failed to create policy context, %s", err)
		return err
	}

	if policyCtx.tpmPolicy != nil {
		return fmt.Errorf("policy %s already exists", p.Name)
	}

	policyCtx.collectorKeys = collKeys

	numCollector := policyCtx.findNumCollectors()
	log.Infof("total num. of collectors %d", numCollector)

	if numCollector > maxFlowExportCollectors {
		return fmt.Errorf("exceeds(%d>%d) maximum collector configs", numCollector, maxFlowExportCollectors)
	}

	if err := policyCtx.createFlowMonitorRule(ctx); err != nil {
		return fmt.Errorf("failed to create policy, err:%s", err)
	}

	// all success? save context
	return policyCtx.writeFlowExportPolicyTable(p)
}

// UpdateFlowExportPolicy is the PUT entry point
func (s *PolicyState) UpdateFlowExportPolicy(ctx context.Context, p *tpmprotos.FlowExportPolicy) error {
	log.Infof("PUT: %+v", p)
	if err := s.DeleteFlowExportPolicy(ctx, p); err != nil {
		return fmt.Errorf("failed to delete policy %s, error %s", p.Name, err)
	}

	return s.CreateFlowExportPolicy(ctx, p)
}

func (p *policyDb) cleanupTables(ctx context.Context, del bool) {
	// caller must write to db
	flowRuleKeys := map[string]bool{}
	collectorKeys := map[string]bool{}

	// read it from policy for delete
	if del {
		for fkey := range p.tpmPolicy.FlowRuleKeys {
			flowRuleKeys[fkey] = true
		}
		for ckey := range p.tpmPolicy.CollectorKeys {
			collectorKeys[ckey] = true
		}

	} else {
		// read it from context for cleanup
		for fkey := range p.flowRuleKeys {
			flowRuleKeys[fkey.String()] = true
		}
		for ckey := range p.collectorKeys {
			collectorKeys[ckey.String()] = true
		}
	}

	for fkey := range flowRuleKeys {
		if fdata, ok := p.flowMonitorTable.FlowRules[fkey]; ok {
			for ckey := range collectorKeys {
				delete(fdata.Collectors[getObjMetaKey(&p.objMeta)], ckey)
			}
			// update new collectors to hal, delete & add
			delete(fdata.PolicyNames, getObjMetaKey(&p.objMeta))
			if len(fdata.PolicyNames) != 0 {
				// this delete/add if the flowrule exists
				log.Infof("updating rule %+v", fdata.RuleKey)
				p.createHalFlowMonitorRule(ctx, fdata.RuleKey)
			} else {
				log.Infof("rule id %d is not used, delete", fdata.RuleID)
				p.deleteFlowMonitorRule(ctx, fdata.RuleID)
				delete(p.flowMonitorTable.FlowRules, fkey)
			}
		}
	}

	for ckey := range collectorKeys {
		if cdata, ok := p.collectorTable.Collector[ckey]; ok {
			delete(cdata.PolicyNames, getObjMetaKey(&p.objMeta))
			if len(cdata.PolicyNames) == 0 {
				p.deleteCollectorPolicy(ctx, cdata.CollectorID)
				delete(p.collectorTable.Collector, ckey)
			}
		}
	}
}

// DeleteFlowExportPolicy is the DELETE entry point
func (s *PolicyState) DeleteFlowExportPolicy(ctx context.Context, p *tpmprotos.FlowExportPolicy) error {
	log.Infof("DELETE :%+v", p)

	if err := s.validateMeta(p); err != nil {
		return err
	}

	s.Lock()
	defer s.Unlock()

	polCtx, err := s.createPolicyContext(p)
	if err != nil {
		return fmt.Errorf("failed to create context, %s", err)
	}

	if polCtx.tpmPolicy == nil {
		return fmt.Errorf("policy %s doesn't exist", p.Name)
	}

	polCtx.cleanupTables(ctx, true)
	polCtx.writeCollectorTable()
	polCtx.writeFlowMonitorTable()

	err = s.store.Delete(&types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta:   p.TypeMeta,
			ObjectMeta: p.ObjectMeta,
		}})
	return err
}

type debugPolicy struct {
	FlowExportPolicy *tpmprotos.FlowExportPolicy
	Collectors       []string
	FlowRules        []string
}

type debugCollector struct {
	StrKey       string
	CollectorKey types.CollectorKey
	CollectorID  uint64
	PolicyNames  []string
}

type debugFlowRules struct {
	StrKey      string
	RuleKey     types.FlowMonitorRuleKey
	RuleID      uint64
	Collector   map[string][]string
	PolicyNames []string
}

type debugInfo struct {
	Policy         map[string]debugPolicy
	CollectorTable []debugCollector
	FlowRuleTable  []debugFlowRules
}

func (s *PolicyState) debug(w http.ResponseWriter, r *http.Request) {
	dbgInfo := debugInfo{
		Policy:         map[string]debugPolicy{},
		CollectorTable: []debugCollector{},
		FlowRuleTable:  []debugFlowRules{},
	}

	s.Lock()
	defer s.Unlock()

	// flow export policy
	if objList, err := s.store.List(&types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "FlowExportPolicy",
			},
		},
	}); err == nil {
		for _, obj := range objList {
			readObj, err := s.store.Read(obj)
			if err != nil || readObj == nil {
				continue
			}
			flowExp, ok := readObj.(*types.FlowExportPolicyTable)
			if !ok {
				continue
			}

			ckey := []string{}
			for k := range flowExp.CollectorKeys {
				ckey = append(ckey, k)
			}

			fkey := []string{}
			for k := range flowExp.FlowRuleKeys {
				fkey = append(fkey, k)
			}

			dbgInfo.Policy[fmt.Sprintf("%s/%s/%s", flowExp.Tenant, flowExp.Namespace, flowExp.Name)] = debugPolicy{
				FlowExportPolicy: flowExp.FlowExportPolicy,
				Collectors:       ckey,
				FlowRules:        fkey,
			}
		}
	}

	// collectors
	if readObj, err := s.store.Read(&types.CollectorTable{
		TypeMeta: api.TypeMeta{Kind: "tpacollectorTable"},
	}); err == nil {
		if cobj, ok := readObj.(*types.CollectorTable); ok {
			for k, v := range cobj.Collector {
				names := []string{}
				for f := range v.PolicyNames {
					names = append(names, f)
				}

				dbgInfo.CollectorTable = append(dbgInfo.CollectorTable, debugCollector{
					CollectorKey: types.ParseCollectorKey(k),
					CollectorID:  v.CollectorID,
					StrKey:       k,
					PolicyNames:  names,
				})
			}

		}
	}

	// flow rules
	if readObj, err := s.store.Read(&types.FlowMonitorTable{
		TypeMeta: api.TypeMeta{Kind: "tpaFlowMonitorTable"},
	}); err == nil {

		fobj, ok := readObj.(*types.FlowMonitorTable)
		if ok {

			cmap := map[string][]string{}

			for k, v := range fobj.FlowRules {
				for pname, value := range v.Collectors {
					cr := []string{}
					for k := range value {
						cr = append(cr, k)
					}
					cmap[pname] = cr

				}

				// policy names
				pname := []string{}
				for k := range v.PolicyNames {
					pname = append(pname, k)
				}

				dbgInfo.FlowRuleTable = append(dbgInfo.FlowRuleTable, debugFlowRules{
					RuleKey:     types.ParseFlowMonitorRuleKey(k),
					RuleID:      v.RuleID,
					PolicyNames: pname,
					StrKey:      k,
					Collector:   cmap,
				})
			}
		}
	}

	json.NewEncoder(w).Encode(dbgInfo)
}

// GetFlowExportPolicy is the GET entry point
func (s *PolicyState) GetFlowExportPolicy(tx context.Context, p *tpmprotos.FlowExportPolicy) (*tpmprotos.FlowExportPolicy, error) {
	log.Infof("GET: %+v", p)

	if err := s.validateMeta(p); err != nil {
		return nil, err
	}

	s.Lock()
	defer s.Unlock()

	polObj, err := s.store.Read(&types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta:   p.TypeMeta,
			ObjectMeta: p.ObjectMeta,
		}})
	if err != nil || polObj == nil {
		log.Errorf("failed to find flow export policy %s, err:%s", p.Name, err)
		return nil, fmt.Errorf("failed to find flow export policy %s", p.Name)
	}

	flowExp, ok := polObj.(*types.FlowExportPolicyTable)
	if !ok {
		log.Errorf("invalid data in flow export db for %s, %+v", p.Name, flowExp)
		return nil, fmt.Errorf("invalid flow export policy")
	}

	return flowExp.FlowExportPolicy, nil
}

// ListFlowExportPolicy is the LIST all entry point
func (s *PolicyState) ListFlowExportPolicy(tx context.Context) ([]*tpmprotos.FlowExportPolicy, error) {
	log.Infof("LIST:")

	s.Lock()
	defer s.Unlock()

	objList, err := s.store.List(&types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "FlowExportPolicy",
			},
		},
	})

	if err != nil {
		log.Errorf("failed to list flow export policy, err:%s", err)
		return nil, fmt.Errorf("failed to list flow export policy")
	}

	flowExpList := []*tpmprotos.FlowExportPolicy{}
	for _, obj := range objList {
		readObj, err := s.store.Read(obj)
		if err != nil || readObj == nil {
			log.Errorf("failed to read flow export policy, err:%s", err)
			return nil, fmt.Errorf("failed to read flow export policy")
		}
		flowExp, ok := readObj.(*types.FlowExportPolicyTable)
		if !ok {
			log.Errorf("invalid type %v in  export", reflect.TypeOf(readObj))
			return nil, fmt.Errorf("failed to read data from flow export policy")
		}
		flowExpList = append(flowExpList, flowExp.FlowExportPolicy)
	}

	return flowExpList, nil
}

// getObjMetaKey creates a key string from objmeta
func getObjMetaKey(m *api.ObjectMeta) string {
	return fmt.Sprintf("%s:%s:%s", m.Tenant, m.Namespace, m.Name)
}

// dummy functions, these polcies are handled in tmagent

// CreateFwLogPolicy is the POST entry point
func (s *PolicyState) CreateFwLogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error {
	return nil
}

// GetFwLogPolicy is the GET entry point
func (s *PolicyState) GetFwLogPolicy(tx context.Context, p *tpmprotos.FwlogPolicy) (*tpmprotos.FwlogPolicy, error) {
	return &tpmprotos.FwlogPolicy{}, nil
}

// ListFwLogPolicy is the LIST entry point
func (s *PolicyState) ListFwLogPolicy(tx context.Context) ([]*tpmprotos.FwlogPolicy, error) {
	return []*tpmprotos.FwlogPolicy{}, nil
}

// UpdateFwLogPolicy is the PUT entry point
func (s *PolicyState) UpdateFwLogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error {
	return nil
}

// DeleteFwLogPolicy is the DEL entry points
func (s *PolicyState) DeleteFwLogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error {
	return nil
}
