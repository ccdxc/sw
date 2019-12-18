package state

import (
	"context"
	"encoding/json"
	"fmt"
	"net"
	"net/http"
	"os"
	"reflect"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"syscall"
	"time"

	"github.com/pensando/sw/nic/agent/netagent/datapath/constants"

	"github.com/pensando/sw/venice/globals"

	"golang.org/x/sys/unix"

	"github.com/pensando/sw/venice/utils/ipfix"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/tpm"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	agstate "github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/protos/tpmprotos"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/nic/agent/tpa/state/types"
	tstype "github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/nic/agent/troubleshooting/utils"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

const (
	flowExportPolicyID = "flowExportPolicyId"
	ipfixSrcPort       = 32007                // src port used by datapath
	tpafile            = "/tmp/.netagent-tpa" // temp file in tmpfs
)

// PolicyState keeps the agent state
type PolicyState struct {
	// global lock
	sync.Mutex
	netAgent      *agstate.Nagent
	getMgmtIPAddr func() string
	store         emstore.Emstore
	hal           halproto.TelemetryClient
	// ipfix template context to store timer info
	ipfixCtx *sync.Map
}

// ipfixTemplateContext is the context for ipfix  templates
type ipfixTemplateContext struct {
	cancel context.CancelFunc
	tick   *time.Ticker
	// txMsg num.of template messages sent
	txMsg uint64
	// txErr num. failures to send template
	txErr uint64
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
func NewTpAgent(netAgent *agstate.Nagent, getMgmtIPAddr func() string, halTm halproto.TelemetryClient) (*PolicyState, error) {
	state := &PolicyState{
		store:         netAgent.Store,
		netAgent:      netAgent,
		getMgmtIPAddr: getMgmtIPAddr,
		hal:           halTm,
		ipfixCtx:      &sync.Map{},
	}

	reboot := false // naples rebooted
	if _, err := os.Stat(tpafile); err != nil {
		if os.IsNotExist(err) {
			if fd, err := os.Create(tpafile); err != nil {
				log.Errorf("failed to create %v, %v", tpafile, err)
			} else {
				fd.Close()
			}
			reboot = true
		}
	}

	// restart? check if there are flow monitor/collectors configured in hal
	pctx := &policyDb{state: state}

	if reboot {
		log.Infof("cleanup flowexport db on restart")

		if err := state.store.Delete(&types.CollectorTable{
			TypeMeta: api.TypeMeta{Kind: "tpacollectorTable"},
		}); err != nil {
			if !strings.Contains(err.Error(), emstore.ErrTableNotFound.Error()) {
				log.Errorf("failed to cleanup collector table")
			}
		}

		if err := state.store.Delete(&types.FlowMonitorTable{
			TypeMeta: api.TypeMeta{Kind: "tpaFlowMonitorTable"},
		}); err != nil {
			if !strings.Contains(err.Error(), emstore.ErrTableNotFound.Error()) {
				log.Errorf("failed to cleanup flow monitor table")
			}
		}

		if objList, err := state.store.List(&types.FlowExportPolicyTable{
			FlowExportPolicy: &tpmprotos.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FlowExportPolicy",
				},
			},
		}); err == nil {
			for _, obj := range objList {
				log.Infof("deleting policy %v", obj.GetObjectMeta())
				if err := state.store.Delete(obj); err != nil {
					log.Errorf("failed to delete policy, %v", err)
				}
			}
		}
	}

	pctx.readCollectorTable()
	for _, cdata := range pctx.collectorTable.Collector {
		ckey := &cdata.Key
		state.SendTemplates(context.Background(), ckey)
	}

	return state, nil
}

// Close closes all policy agent resources
func (s *PolicyState) Close() {
}

func (s *PolicyState) validateMeta(p *tpmprotos.FlowExportPolicy) error {
	if strings.TrimSpace(p.Name) == "" || strings.TrimSpace(p.Kind) == "" {
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
		return halproto.IPProtocol_IPPROTO_NONE, 0, fmt.Errorf("transport should be in protocol/port format")
	}

	p, ok := halproto.IPProtocol_value["IPPROTO_"+strings.ToUpper(s[0])]
	if !ok {
		return halproto.IPProtocol_IPPROTO_NONE, 0, fmt.Errorf("invalid protocol in %s", src)
	}
	proto := halproto.IPProtocol(p)

	switch proto {
	case halproto.IPProtocol_IPPROTO_UDP:

	default:
		return halproto.IPProtocol_IPPROTO_NONE, 0, fmt.Errorf("invalid protocol in %s\n Only UDP is accepted", src)
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

func validateFlowExportInterval(s string) (time.Duration, error) {

	interval, err := time.ParseDuration(s)
	if err != nil {
		return interval, fmt.Errorf("invalid interval %s", s)
	}

	if interval < time.Second {
		return interval, fmt.Errorf("too small interval %s", s)
	}

	if interval > 24*time.Hour {
		return interval, fmt.Errorf("too large interval %s", s)
	}

	return interval, nil
}

func validateTemplateInterval(s string) (time.Duration, error) {

	interval, err := time.ParseDuration(s)
	if err != nil {
		return interval, fmt.Errorf("invalid template interval %s", s)
	}

	if interval < time.Minute {
		return interval, fmt.Errorf("too small template interval %s", s)
	}

	if interval > 30*time.Minute {
		return interval, fmt.Errorf("too large template interval %s", s)
	}

	return interval, nil
}

func validateFlowExportFormat(s string) error {
	if halproto.ExportFormat_name[int32(halproto.ExportFormat_IPFIX)] != strings.ToUpper(s) {
		return fmt.Errorf("invalid format %s", s)
	}
	return nil
}

// ValidateFlowExportPolicy validates policy, called from api-server
func ValidateFlowExportPolicy(p *monitoring.FlowExportPolicy) error {
	spec := p.Spec
	if _, err := validateFlowExportInterval(spec.Interval); err != nil {
		return err
	}

	if _, err := validateTemplateInterval(spec.TemplateInterval); err != nil {
		return err
	}

	if err := validateFlowExportFormat(spec.Format); err != nil {
		return err
	}

	if len(spec.MatchRules) != 0 {
		data, err := json.Marshal(spec.MatchRules)
		if err != nil {
			return fmt.Errorf("failed to marshal, %v", err)
		}

		matchrules := []tsproto.MatchRule{}
		if err := json.Unmarshal(data, &matchrules); err != nil {
			return fmt.Errorf("failed to unmarshal, %v", err)
		}

		if err := utils.ValidateMatchRules(p.ObjectMeta, matchrules,
			func(meta api.ObjectMeta) (*netproto.Endpoint, error) { return nil, nil }); err != nil {
			return fmt.Errorf("error in match-rule, %v", err)
		}
	}

	if len(spec.Exports) == 0 {
		return fmt.Errorf("no targets configured")
	}

	if len(spec.Exports) > tpm.MaxNumCollectorsPerPolicy {
		return fmt.Errorf("cannot configure more than %d targets", tpm.MaxNumCollectorsPerPolicy)
	}

	feTargets := map[string]bool{}
	for _, export := range spec.Exports {
		if key, err := json.Marshal(export); err == nil {
			ks := string(key)
			if _, ok := feTargets[ks]; ok {
				return fmt.Errorf("found duplicate target %v %v", export.Destination, export.Transport)
			}
			feTargets[ks] = true
		}

		dest := export.Destination
		if dest == "" {
			return fmt.Errorf("destination is empty")
		}

		netIP, _, err := net.ParseCIDR(dest)
		if err == nil {
			dest = netIP.String()
		} else {
			netIP = net.ParseIP(dest)
		}

		if netIP == nil {
			// treat it as hostname and resolve
			s, err := net.LookupHost(dest)
			if err != nil || len(s) == 0 {
				return fmt.Errorf("failed to resolve name {%s}, error: %s", dest, err)
			}
		}

		if _, _, err := parsePortProto(export.Transport); err != nil {
			return err
		}
	}

	return nil
}

func (s *PolicyState) validatePolicy(p *tpmprotos.FlowExportPolicy) (map[types.CollectorKey]bool, error) {
	if err := s.validateMeta(p); err != nil {
		return nil, err
	}
	spec := p.Spec

	interval, err := validateFlowExportInterval(spec.Interval)
	if err != nil {
		interval = 10 * time.Second
	}

	templInterval, err := validateTemplateInterval(spec.TemplateInterval)
	if err != nil {
		templInterval = 5 * time.Minute
	}

	if err := validateFlowExportFormat(spec.Format); err != nil {
		return nil, err
	}

	if len(spec.MatchRules) != 0 {

		if err := utils.ValidateMatchRules(p.ObjectMeta, spec.MatchRules, s.netAgent.FindEndpoint); err != nil {
			return nil, fmt.Errorf("error in match-rule, %v", err)
		}
	}

	if len(spec.Exports) == 0 {
		return nil, fmt.Errorf("no targets configured")
	}

	if len(spec.Exports) > tpm.MaxNumCollectorsPerPolicy {
		return nil, fmt.Errorf("cannot configure more than %d targets", tpm.MaxNumCollectorsPerPolicy)
	}

	// get vrf
	vrf, err := s.getVrfID(p.GetTenant(), p.GetNamespace(), p.Spec.VrfName)
	if err != nil {
		return nil, fmt.Errorf("failed to find vrf for %s/%s", p.GetTenant(), p.GetNamespace())
	}

	collKeys := map[types.CollectorKey]bool{}
	for _, export := range spec.Exports {
		dest := export.Destination
		if dest == "" {
			return nil, fmt.Errorf("destination can't be empty")
		}

		netIP, _, err := net.ParseCIDR(dest)
		if err == nil {
			dest = netIP.String()
		} else {
			netIP = net.ParseIP(dest)
		}

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
			Interval:         uint32(interval.Seconds()),
			TemplateInterval: uint32(templInterval.Seconds()),
			Format:           halproto.ExportFormat_IPFIX,
			Protocol:         proto,
			Destination:      dest,
			Port:             port,
			VrfID:            vrf,
		}
		collKeys[newKey] = true
	}

	return collKeys, nil
}

// get vrf from vrf name
func (s *PolicyState) getVrfID(tenant, namespace, vrfName string) (uint64, error) {
	vrfObj, err := s.netAgent.ValidateVrf(tenant, namespace, vrfName)
	if err != nil {
		log.Errorf("failed to find vrf %s. Err: %v", vrfName, err)
		return uint64(0), fmt.Errorf("failed to find vrf %s. Err: %v", vrfName, err)
	}
	return vrfObj.Status.VrfID, nil
}

// get vrf from tenant/namespace
func (s *PolicyState) getL2SegID(tenant, namespace, address string) (*netproto.NetworkStatus, error) {
	epList := s.netAgent.ListEndpoint()
	for _, ep := range epList {
		for _, addr := range ep.Spec.IPv4Addresses {
			epAddr, _, err := net.ParseCIDR(addr)
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
	}
	return nil, fmt.Errorf("failed to find l2 segment id for %s", address)
}

func convertToHalIPAddr(src string) (*halproto.IPAddress, string, error) {
	netIP, _, err := net.ParseCIDR(src)
	if err == nil {
		src = netIP.String()
	} else {
		netIP = net.ParseIP(src)
	}

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
					if val, ok := p.state.ipfixCtx.Load(ckey.String()); ok {
						if tick, ok := val.(*time.Ticker); ok {
							tick.Stop()
						}
					}
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

	srcIPAddr := p.state.getMgmtIPAddr()
	if srcIPAddr == "" {
		// get local endpoint
		if ep, err := p.state.netAgent.FindLocalEndpoint(p.objMeta.Tenant, p.objMeta.Namespace); err == nil {
			for _, addr := range ep.Spec.IPv4Addresses {
				srcAddr, _, err := net.ParseCIDR(addr)
				if err != nil {
					log.Errorf("failed to parse local endpoint address {%+v} ", addr)
					return fmt.Errorf("invalid local endpoint address %s, %s", srcAddr, err)
				}
				srcIPAddr = srcAddr.String()
				// Use the first IP Address
				break
			}
		}
	}

	halSrcAddr, _, err := convertToHalIPAddr(srcIPAddr)
	if err != nil {
		return fmt.Errorf("invalid source address %s, %s", srcIPAddr, err)
	}

	for k := range p.collectorKeys {
		ckey := k
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

		if err := p.state.SendTemplates(ctx, &ckey); err != nil {
			return fmt.Errorf("failed to send template %v", err)
		}

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
				EncapValue: constants.UntaggedCollVLAN,
			},

			L2SegKeyHandle: &halproto.L2SegmentKeyHandle{
				KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
					SegmentId: netObj.NetworkID,
				},
			},

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
			Key:         ckey,
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

func (p *policyDb) createHalFlowMonitorRule(ctx context.Context, ruleKey types.FlowMonitorRuleKey, linkPolicy bool) error {
	// create rule id
	var ruleID uint64
	var err error

	log.Infof("create rule %+v ", ruleKey)
	ruleID, err = p.state.store.GetNextID(tstype.FlowMonitorRuleIDType)
	if err != nil {
		log.Errorf("failed to allocate object id for flow monitor %s", err)
		return fmt.Errorf("failed to allocate object id for flow monitor %s", err)
	}

	//todo: delete only if there is any change in collector config
	flowRuleData, ok := p.flowMonitorTable.FlowRules[ruleKey.String()]
	if ok {
		log.Infof("rule %+v already exists, deleting ", ruleKey)
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

	if linkPolicy {
		// link new policy
		if _, ok := flowRuleData.Collectors[getObjMetaKey(&p.objMeta)]; !ok {
			flowRuleData.Collectors[getObjMetaKey(&p.objMeta)] = map[string]bool{}
		}
		flowRuleData.PolicyNames[getObjMetaKey(&p.objMeta)] = true
	}

	p.flowRuleKeys[ruleKey] = true

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
	newKeys := map[string]bool{}
	for ckey := range p.collectorKeys {
		if cdata, ok := p.collectorTable.Collector[ckey.String()]; ok {
			collectorKeys = append(collectorKeys, &halproto.CollectorKeyHandle{
				KeyOrHandle: &halproto.CollectorKeyHandle_CollectorId{
					CollectorId: cdata.CollectorID,
				}})
			newKeys[ckey.String()] = true
		}
	}

	// now pick up existing collectors
	for _, cmap := range flowRuleData.Collectors {
		for ckey := range cmap {
			// skip already added collectors
			if ok := newKeys[ckey]; ok {
				continue
			}
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
			Protocol: halproto.IPProtocol(appPortObj.Ipproto),
		},
		CollectorKeyHandle: collectorKeys,

		Action: &halproto.MonitorAction{
			Action: []halproto.RuleAction{halproto.RuleAction_COLLECT_FLOW_STATS},
		},
	}

	if ruleKey.SourceIP != "any" {
		flowRuleSpec.Match.SrcAddress = []*halproto.IPAddressObj{srcAddrObj}
	}

	if ruleKey.DestIP != "any" {
		flowRuleSpec.Match.DstAddress = []*halproto.IPAddressObj{destAddrObj}
	}

	if ruleKey.DestL4Port != 0 || ruleKey.Protocol != 0 {
		flowRuleSpec.Match.AppMatch = appMatchObj
	}

	// add valid mac address
	if ruleKey.SourceMac != 0 {
		flowRuleSpec.Match.SrcMacAddress = []uint64{ruleKey.SourceMac}
	}

	if ruleKey.DestMac != 0 {
		flowRuleSpec.Match.DstMacAddress = []uint64{ruleKey.DestMac}
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
	if linkPolicy {
		if policyData, ok := flowRuleData.Collectors[getObjMetaKey(&p.objMeta)]; ok {
			for ckey := range p.collectorKeys {
				policyData[ckey.String()] = true
			}
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

		log.Infof("ip flow rule %+v", ruleKey)

		if err := p.createHalFlowMonitorRule(ctx, ruleKey, true); err != nil {
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
		log.Infof("mac flow rule %+v", ruleKey)

		if err := p.createHalFlowMonitorRule(ctx, ruleKey, true); err != nil {
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

	log.Debugf("num rules %d, %+v", len(p.matchRules), p.matchRules)
	// process match-rule
	for _, rule := range p.matchRules {
		srcIPs, destIPs, srcMACs, destMACs, appPorts, srcIPStrings, destIPStrings := utils.ExpandCompositeMatchRule(p.collectorTable.ObjectMeta, &rule, p.state.netAgent.FindEndpoint)
		// Create protobuf requestMsgs on cross product of
		//  - srcIPs, destIPs, Apps
		//  - srcMACs, destMACs, Apps

		ipFmRuleList, _ := utils.CreateIPAddrCrossProductRuleList(srcIPs, destIPs, appPorts, srcIPStrings, destIPStrings)
		macFmRuleList, _ := utils.CreateMACAddrCrossProductRuleList(srcMACs, destMACs, appPorts)

		log.Debugf("num ip-rules %v, mac-rules %v", len(ipFmRuleList), len(macFmRuleList))
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

func (p *policyDb) readFlowMonitorTable() *types.FlowMonitorTable {
	fmObj := &types.FlowMonitorTable{
		TypeMeta:  api.TypeMeta{Kind: "tpaFlowMonitorTable"},
		FlowRules: map[string]*types.FlowMonitorData{},
	}

	p.flowMonitorTable = fmObj

	obj, err := p.state.store.Read(fmObj)
	if err != nil {
		if !strings.Contains(err.Error(), emstore.ErrTableNotFound.Error()) {
			log.Warnf("failed to read FlowMonitor table, %s", err)
		}
		return fmObj
	}

	dbObj, ok := obj.(*types.FlowMonitorTable)
	if !ok {
		log.Errorf("invalid flow monitor object in db for %+v", fmObj.ObjectMeta)
		return fmObj
	}

	p.flowMonitorTable = dbObj

	return dbObj
}

func (p *policyDb) writeFlowMonitorTable() (err error) {
	if err := p.state.store.Write(p.flowMonitorTable); err != nil {
		return fmt.Errorf("failed to write  flowmonitor object in db, %s", err)
	}
	return nil
}

func (p *policyDb) readCollectorTable() error {
	collObj := &types.CollectorTable{
		TypeMeta:  api.TypeMeta{Kind: "tpacollectorTable"},
		Collector: map[string]*types.CollectorData{},
	}

	p.collectorTable = collObj

	obj, err := p.state.store.Read(collObj)
	if err != nil {
		if !strings.Contains(err.Error(), emstore.ErrTableNotFound.Error()) {
			log.Warnf("failed to read collector table, %s", err)
		}
		return nil
	}

	dbObj, ok := obj.(*types.CollectorTable)
	if !ok {
		log.Errorf("invalid collector object in db for %+v", collObj.ObjectMeta)
		return nil
	}

	p.collectorTable = dbObj
	return nil
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
		return nil, fmt.Errorf("failed to read policy, error: %s", err)
	}

	flowObj, ok := obj.(*types.FlowExportPolicyTable)
	if !ok {
		return nil, fmt.Errorf("invalid flow export object type %T", obj)
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
	}

	// todo: check no match-rule ? match all for ipfix
	if p.Spec.MatchRules == nil {
		policyCtx.matchRules = []tsproto.MatchRule{
			{
				Src:         &tsproto.MatchSelector{IPAddresses: []string{"any"}},
				Dst:         &tsproto.MatchSelector{IPAddresses: []string{"any"}},
				AppProtoSel: &tsproto.AppProtoSelector{Ports: []string{"any"}},
			},
		}

	} else {
		for _, r := range p.Spec.MatchRules {
			rule := tsproto.MatchRule{
				Src:         &tsproto.MatchSelector{IPAddresses: []string{"any"}},
				Dst:         &tsproto.MatchSelector{IPAddresses: []string{"any"}},
				AppProtoSel: &tsproto.AppProtoSelector{Ports: []string{"any"}},
			}

			if r.Src != nil {
				rule.Src = r.Src
			}
			if r.Dst != nil {
				rule.Dst = r.Dst
			}
			if r.AppProtoSel != nil {
				rule.AppProtoSel = r.AppProtoSel
			}
			policyCtx.matchRules = append(policyCtx.matchRules, rule)
		}
	}

	polObj, err := policyCtx.readFlowExportPolicyTable(p)
	if err == nil {
		policyCtx.tpmPolicy = polObj
	}

	// read collector object
	policyCtx.readCollectorTable()

	// read flowmonitor object
	policyCtx.readFlowMonitorTable()

	// get vrf
	vrf, err := s.getVrfID(p.Tenant, p.Namespace, p.Spec.VrfName)
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

	if objList, err := s.store.List(&types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "FlowExportPolicy",
			},
		},
	}); err == nil {
		if len(objList) >= tpm.MaxNumExportPolicy {
			return fmt.Errorf("can't configure more than %v FlowExportPolicy", tpm.MaxNumExportPolicy)
		}
	}

	policyCtx.collectorKeys = collKeys

	// Create lateral objects here
	for _, c := range p.Spec.Exports {
		mgmtIP := s.getMgmtIPAddr()
		if err := s.netAgent.CreateLateralNetAgentObjects(p.GetKey(), mgmtIP, c.Destination, false); err != nil {
			log.Errorf("Failed to create lateral objects in netagent. Err: %v", err)
			return err
		}
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
	pctx, err := s.createPolicyContext(p)
	if err != nil {
		return fmt.Errorf("failed to create policy context, %s", err)
	}

	if pctx.tpmPolicy == nil {
		return fmt.Errorf("policy %v doesn't exist", p.Name)
	}

	if reflect.DeepEqual(pctx.tpmPolicy.Spec, p.Spec) {
		log.Infof("no change in policy %v", p.Name)
		return nil
	}

	if err := s.DeleteFlowExportPolicy(ctx, p); err != nil {
		return fmt.Errorf("policy %s doesn't exist", p.Name)
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

			log.Infof("delete collectors from %v", p.objMeta.Name)
			delete(fdata.Collectors, getObjMetaKey(&p.objMeta))

			// update new collectors to hal, delete & add
			delete(fdata.PolicyNames, getObjMetaKey(&p.objMeta))
			if len(fdata.PolicyNames) != 0 {
				// this delete/add if the flowrule exists
				log.Infof("updating rule %+v", fdata.RuleKey)
				p.createHalFlowMonitorRule(ctx, fdata.RuleKey, false)
			} else {
				log.Infof("delete rule %+v", fdata.RuleID)
				p.deleteFlowMonitorRule(ctx, fdata.RuleID)
				delete(p.flowMonitorTable.FlowRules, fkey)
			}
		}
	}

	for ckey := range collectorKeys {
		if cdata, ok := p.collectorTable.Collector[ckey]; ok {
			delete(cdata.PolicyNames, getObjMetaKey(&p.objMeta))
			if len(cdata.PolicyNames) == 0 {
				log.Infof("delete collector %+v", ckey)
				if val, ok := p.state.ipfixCtx.Load(ckey); ok {
					if c, ok := val.(*ipfixTemplateContext); ok {
						c.tick.Stop()
						c.cancel()
					}
				}
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

	for _, c := range polCtx.tpmPolicy.Spec.Exports {
		mgmtIP := s.getMgmtIPAddr()
		if err := s.netAgent.DeleteLateralNetAgentObjects(p.GetKey(), mgmtIP, c.Destination, false); err != nil {
			log.Errorf("Failed to delete lateral objects in netagent. Err: %v", err)
		}
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
	TxMsg        uint64
	TxErr        uint64
	Timer        bool
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

// Debug is the tpa debug entry point from REST
func (s *PolicyState) Debug(r *http.Request) (interface{}, error) {
	dbgInfo := &debugInfo{
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
			readObj, err := s.store.Read(&types.FlowExportPolicyTable{
				FlowExportPolicy: &tpmprotos.FlowExportPolicy{
					TypeMeta: api.TypeMeta{
						Kind: "FlowExportPolicy",
					},
					ObjectMeta: *obj.GetObjectMeta(),
				},
			})

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
				var txErr uint64
				var txMsg uint64
				var timerState bool

				for f := range v.PolicyNames {
					names = append(names, f)
				}

				if v, ok := s.ipfixCtx.Load(k); ok {
					if tv, ok := v.(*ipfixTemplateContext); ok {
						txErr = tv.txErr
						txMsg = tv.txMsg
						timerState = tv.tick != nil
					}
				}

				dbgInfo.CollectorTable = append(dbgInfo.CollectorTable, debugCollector{
					CollectorKey: types.ParseCollectorKey(k),
					CollectorID:  v.CollectorID,
					StrKey:       k,
					PolicyNames:  names,
					TxMsg:        txMsg,
					TxErr:        txErr,
					Timer:        timerState,
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

	return dbgInfo, nil
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
	s.Lock()
	defer s.Unlock()

	objList, err := s.store.List(&types.FlowExportPolicyTable{
		FlowExportPolicy: &tpmprotos.FlowExportPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "FlowExportPolicy",
			},
		},
	})

	flowExpList := []*tpmprotos.FlowExportPolicy{}

	if err != nil {
		if !strings.Contains(err.Error(), emstore.ErrTableNotFound.Error()) {
			log.Errorf("failed to list flow export policy, err:%s", err)
		}
		return flowExpList, nil
	}

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

// CreateFwlogPolicy is the POST entry point
func (s *PolicyState) CreateFwlogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error {
	return nil
}

// GetFwlogPolicy is the GET entry point
func (s *PolicyState) GetFwlogPolicy(tx context.Context, p *tpmprotos.FwlogPolicy) (*tpmprotos.FwlogPolicy, error) {
	return &tpmprotos.FwlogPolicy{}, nil
}

// ListFwlogPolicy is the LIST entry point
func (s *PolicyState) ListFwlogPolicy(tx context.Context) ([]*tpmprotos.FwlogPolicy, error) {
	return []*tpmprotos.FwlogPolicy{}, nil
}

// UpdateFwlogPolicy is the PUT entry point
func (s *PolicyState) UpdateFwlogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error {
	return nil
}

// DeleteFwlogPolicy is the DEL entry points
func (s *PolicyState) DeleteFwlogPolicy(ctx context.Context, p *tpmprotos.FwlogPolicy) error {
	return nil
}

// SendTemplates sends ipfix templates periodically
func (s *PolicyState) SendTemplates(ctx context.Context, ckey *types.CollectorKey) error {
	dest := ckey.Destination
	dport := int(ckey.Port)
	interval := time.Duration(ckey.TemplateInterval) * time.Second

	tick := time.NewTicker(interval)

	tmplt, err := ipfix.CreateTemplateMsg()
	if err != nil {
		log.Errorf("failed to generate template, %v", err)
		return err
	}

	nc := net.ListenConfig{Control: func(network, address string, c syscall.RawConn) error {
		var sockErr error
		if err := c.Control(func(fd uintptr) {
			sockErr = unix.SetsockoptInt(int(fd), unix.SOL_SOCKET, unix.SO_REUSEPORT, 1)
		}); err != nil {
			return err
		}
		return sockErr
	},
	}

	conn, err := nc.ListenPacket(ctx, "udp", fmt.Sprintf("%v:%v", s.getMgmtIPAddr(), ipfixSrcPort))
	if err != nil {
		log.Errorf("failed to bind %v, %v", fmt.Sprintf("%v:%v", s.getMgmtIPAddr(), ipfixSrcPort), err)
		return err
	}

	tctx, cancel := context.WithCancel(context.Background())
	templateCtx := &ipfixTemplateContext{
		cancel: cancel,
		tick:   tick,
	}

	s.ipfixCtx.Store(ckey.String(), templateCtx)

	go func() {
		defer conn.Close()
		// send first template as soon as we start
		if _, err := conn.WriteTo(tmplt, &net.UDPAddr{IP: net.ParseIP(dest), Port: dport}); err != nil {
			log.Errorf("failed to send to %v:%v, %v", dest, dport, err)
			atomic.AddUint64(&templateCtx.txErr, 1)
		} else {
			atomic.AddUint64(&templateCtx.txMsg, 1)
		}

		for {
			select {
			case <-tick.C:
				if _, err := conn.WriteTo(tmplt, &net.UDPAddr{IP: net.ParseIP(dest), Port: dport}); err != nil {
					log.Errorf("failed to send to %v", err)
					atomic.AddUint64(&templateCtx.txErr, 1)
				} else {
					atomic.AddUint64(&templateCtx.txMsg, 1)
				}
			case <-tctx.Done():
				log.Infof("timer stopped, stopping templates to %v:%v", dest, dport)
				return
			}
		}
	}()

	return nil
}
