package state

import (
	"context"
	"fmt"
	"net"
	"reflect"
	"strconv"
	"strings"
	"sync"

	"time"

	"github.com/pensando/sw/api"
	apiproto "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	agstate "github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/tpa/protos"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/ctrler/tpm/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

const (
	emDbPath                = "/tmp/naples-tpagent.db" // deleted on restart
	flowExportPolicyID      = "flowExportPolicyId"
	maxFlowExportCollectors = 16
)

// PolicyState keeps the policy agent state
type PolicyState struct {
	sync.Mutex // global lock
	netAgent   *agstate.Nagent
	store      emstore.Emstore
	hal        halproto.TelemetryClient
}

// networkMeta is the meta data passed to functions
type networkMeta struct {
	vrf       uint64
	tenant    string
	namespace string
}

// NewTpAgent creates new telemetry policy agent state
func NewTpAgent(dbPath string, netAgent *agstate.Nagent, halTm halproto.TelemetryClient) (*PolicyState, error) {
	if dbPath == "" {
		dbPath = emDbPath
	}
	mstore, err := emstore.NewEmstore(emstore.BoltDBType, dbPath)
	if err != nil {
		log.Errorf("failed to open db: %s, err: %v", dbPath, err)
		return nil, err
	}

	state := &PolicyState{
		store:    mstore,
		netAgent: netAgent,
		hal:      halTm,
	}

	return state, nil
}

// Close closes all policy agent resources
func (s *PolicyState) Close() {
	s.store.Close()
}

func (s *PolicyState) validateMeta(p *tpmprotos.FlowExportPolicy) error {
	if p.Name == "" || p.Kind == "" {
		return fmt.Errorf("name/kind can't be empty")
	}
	return nil
}

func (s *PolicyState) findNumExports(p *api.ObjectMeta) (int, error) {
	numExports := 0

	// list all, to check the total number of collector policies
	pl, err := s.store.List(&tpaprotos.FlowExportPolicyObj{
		TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{},
	})
	if err == nil {
		for _, obj := range pl {
			//TODO:  debug emstore.list()
			v, err := s.store.Read(&tpaprotos.FlowExportPolicyObj{
				TypeMeta:   api.TypeMeta{Kind: obj.GetObjectKind()},
				ObjectMeta: *obj.GetObjectMeta()})
			if err != nil || v == nil {
				return numExports, fmt.Errorf("failed to read policy %s, %s", obj.GetObjectMeta(), err)
			}
			if flowExp, ok := v.(*tpaprotos.FlowExportPolicyObj); ok {
				if p != nil && flowExp.GetName() == p.GetName() &&
					flowExp.GetTenant() == p.GetTenant() &&
					flowExp.GetNamespace() == p.GetNamespace() {
					continue
				}

				for _, tgt := range flowExp.P.Spec.Targets {
					numExports += len(tgt.Exports)
				}
			}
		}
	}

	return numExports, nil
}

func (s *PolicyState) validatePolicy(p *tpmprotos.FlowExportPolicy) (int, error) {
	numExports := 0

	if err := s.validateMeta(p); err != nil {
		return numExports, err
	}

	if len(p.Spec.Targets) == 0 {
		return numExports, fmt.Errorf("export targets can't be empty")
	}

	for _, t := range p.Spec.Targets {
		if len(t.Exports) == 0 {
			return 0, fmt.Errorf("exports can't be empty")
		}

		for _, export := range t.Exports {
			numExports++

			if export.Destination == "" {
				return numExports, fmt.Errorf("destination can't be empty")
			}

			if _, _, err := parsePortProto(export.Transport); err != nil {
				return numExports, err
			}
		}
	}

	return numExports, nil
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
	if uint(port) > uint((^uint16(0))) {
		return halproto.IPProtocol_IPPROTO_NONE, 0, fmt.Errorf("invalid port in %s", src)
	}

	return proto, uint32(port), nil
}

// createCollectorPolicy() create collector policy in HAL and return the key
func (s *PolicyState) createCollectorPolicy(ctx context.Context, netMeta *networkMeta, fmtStr string, interval uint32, export *apiproto.ExportConfig) (uint64, error) {

	objID, err := s.store.GetNextID(flowExportPolicyID)
	if err != nil {
		return uint64(0), fmt.Errorf("failed to allocate object id for collector policy")
	}

	halDestAddr, destAddr, err := convertToHalIPAddr(export.Destination)
	if err != nil {
		return uint64(0), fmt.Errorf("invalid destination address %s, %s", export.Destination, err)
	}

	proto, port, err := parsePortProto(export.Transport)
	if err != nil {
		return uint64(0), fmt.Errorf("failed to parse %s, %s", export.Transport, err)
	}

	// only IPFIX is supported
	format, ok := halproto.ExportFormat_value[fmtStr]
	if !ok || halproto.ExportFormat(format) != halproto.ExportFormat_IPFIX {
		return uint64(0), fmt.Errorf("invalid format %s", fmtStr)
	}

	netObj, err := s.getL2SegID(netMeta.tenant, netMeta.namespace, destAddr)
	if err != nil {
		return uint64(0), fmt.Errorf("invalid l2 segment, %s", err)
	}

	var req []*halproto.CollectorSpec
	req = append(req, &halproto.CollectorSpec{
		Meta: &halproto.ObjectMeta{VrfId: netMeta.vrf},
		KeyOrHandle: &halproto.CollectorKeyHandle{
			KeyOrHandle: &halproto.CollectorKeyHandle_CollectorId{
				CollectorId: objID,
			},
		},
		VrfKeyHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: netMeta.vrf,
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
		DestIp:         halDestAddr,
		Protocol:       proto,
		DestPort:       port,
		ExportInterval: interval,
		Format:         halproto.ExportFormat(format),
	})

	log.Infof("create hal collector policy, req: {%+v}", req)

	// send to hal
	resp, err := s.hal.CollectorCreate(ctx, &halproto.CollectorRequestMsg{Request: req})
	if err != nil {
		log.Errorf("failed to create collector policy in hal, %s", err)
		return uint64(0), err
	}

	if resp != nil && len(resp.Response) > 0 && resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to create collector policy in hal, api returned %v", resp.Response[0].ApiStatus)
		return uint64(0), fmt.Errorf("api error %v", resp.Response[0].ApiStatus)
	}

	return objID, nil
}

// deleteCollectorPolicy() delete collector policy in HAL from the key
func (s *PolicyState) deleteCollectorPolicy(ctx context.Context, netMeta *networkMeta, key uint64) error {
	var req []*halproto.CollectorDeleteRequest

	if key == 0 {
		return nil
	}

	req = append(req, &halproto.CollectorDeleteRequest{
		Meta: &halproto.ObjectMeta{VrfId: netMeta.vrf},
		KeyOrHandle: &halproto.CollectorKeyHandle{
			KeyOrHandle: &halproto.CollectorKeyHandle_CollectorId{
				CollectorId: key,
			},
		}})

	// send to hal
	log.Infof("delete hal collector policy {%+v}", req)
	resp, err := s.hal.CollectorDelete(ctx, &halproto.CollectorDeleteRequestMsg{Request: req})
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

func (s *PolicyState) deleteFlowMonitorRule(ctx context.Context, netMeta *networkMeta, flowMon *tpaprotos.FlowMonitorObj) error {
	log.Infof("delete flow monitor rule: %+v", flowMon)

	for _, c := range flowMon.CollectorKey {
		s.deleteCollectorPolicy(ctx, netMeta, c)
	}

	if flowMon.Key == 0 {
		return nil
	}

	var req []*halproto.FlowMonitorRuleDeleteRequest
	req = append(req, &halproto.FlowMonitorRuleDeleteRequest{
		KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
			KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
				FlowmonitorruleId: flowMon.Key,
			},
		},
		VrfKeyHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: netMeta.vrf,
			},
		},
	})

	log.Infof("delete hal flow monitor, req: {%+v}", flowMon)
	resp, err := s.hal.FlowMonitorRuleDelete(ctx, &halproto.FlowMonitorRuleDeleteRequestMsg{
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

func (s *PolicyState) createFlowMonitorRule(ctx context.Context, meta *networkMeta, dbFlowExp *tpaprotos.FlowExportPolicyObj, target *monitoring.FlowExportTarget) error {
	flowMon := &tpaprotos.FlowMonitorObj{}
	dbFlowExp.FlowMonObj = append(dbFlowExp.FlowMonObj, flowMon)

	interval, err := time.ParseDuration(target.Interval)
	if err != nil {
		return fmt.Errorf("invalid interval %s", target.Interval)
	}

	var collectorKeys []*halproto.CollectorKeyHandle

	for i := 0; i < len(target.Exports); i++ {
		// configure collector policy
		collectorKey, err := s.createCollectorPolicy(ctx, meta, strings.ToUpper(target.Format), uint32(interval), &target.Exports[i])
		if err != nil {
			log.Errorf("failed to create collector policy for %s, %+v, error:%s ", dbFlowExp.P.Name, target.Exports[i], err)
			return fmt.Errorf("failed to create policy, %s", err)
		}
		// save HAL ids
		flowMon.CollectorKey = append(flowMon.CollectorKey, collectorKey)
		collectorKeys = append(collectorKeys, &halproto.CollectorKeyHandle{
			KeyOrHandle: &halproto.CollectorKeyHandle_CollectorId{
				CollectorId: collectorKey,
			},
		})
	}

	objID, err := s.store.GetNextID(flowExportPolicyID)
	if err != nil {
		log.Errorf("failed to allocate object id for export control %s", dbFlowExp.P.Name)
		return fmt.Errorf("failed to allocate unique object id ")
	}
	flowMon.Key = objID

	// create flow monitor rule
	var req []*halproto.FlowMonitorRuleSpec
	req = append(req, &halproto.FlowMonitorRuleSpec{
		VrfKeyHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: meta.vrf,
			},
		},
		KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
			KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
				FlowmonitorruleId: objID,
			},
		},
		CollectorKeyHandle: collectorKeys,
		// match all, 0.0.0.0/0
		Match: &halproto.RuleMatch{
			SrcAddress: []*halproto.IPAddressObj{
				{
					Formats: &halproto.IPAddressObj_Address{
						Address: &halproto.Address{
							Address: &halproto.Address_Prefix{
								Prefix: &halproto.IPSubnet{
									Subnet: &halproto.IPSubnet_Ipv4Subnet{
										Ipv4Subnet: &halproto.IPPrefix{
											Address: &halproto.IPAddress{
												IpAf:   halproto.IPAddressFamily_IP_AF_INET,
												V4OrV6: &halproto.IPAddress_V4Addr{},
											},
										},
									},
								},
							},
						},
					},
				},
			},
			AppMatch: &halproto.RuleMatch_AppMatch{
				App: &halproto.RuleMatch_AppMatch_PortInfo{
					PortInfo: &halproto.RuleMatch_L4PortAppInfo{
						DstPortRange: []*halproto.L4PortRange{
							{
								PortLow:  0,
								PortHigh: 65535, // all ports
							},
						},
					},
				},
			},
		},
		Action: &halproto.MonitorAction{
			Action: []halproto.RuleAction{
				halproto.RuleAction_COLLECT_FLOW_STATS,
			},
		},
	})

	log.Infof("create hal flow monitor, req: {%+v}", req)
	resp, err := s.hal.FlowMonitorRuleCreate(ctx, &halproto.FlowMonitorRuleRequestMsg{Request: req})
	if err != nil {
		log.Errorf("failed to create hal flow monitor rule policy, %s", err)
		return err
	}

	if resp != nil && len(resp.Response) > 0 && resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		log.Errorf("failed to create hal flow monitor rule policy, api returned %v", resp.Response[0].ApiStatus)
		return fmt.Errorf("api error %v", resp.Response[0].ApiStatus)
	}

	return nil
}

// CreateFlowExportPolicy is the POST() entry point
func (s *PolicyState) CreateFlowExportPolicy(ctx context.Context, p *tpmprotos.FlowExportPolicy) (err error) {
	log.Infof("POST: %+v", p)

	newExports, err := s.validatePolicy(p)
	if err != nil {
		log.Errorf("invalid policy, %s", err)
		return err
	}

	s.Lock()
	defer s.Unlock()

	numExports, err := s.findNumExports(&api.ObjectMeta{})
	if err != nil {
		return err
	}

	if numExports+newExports > maxFlowExportCollectors {
		return fmt.Errorf("exceeds maximum(%d) export configs", maxFlowExportCollectors)
	}

	db, err := s.store.Read(&tpaprotos.FlowExportPolicyObj{TypeMeta: p.TypeMeta, ObjectMeta: p.ObjectMeta})
	if err == nil && db != nil {
		log.Infof("flow export policy %s already exists", p.Name)
		// todo: update ?
		return nil
	}

	vrf, err := s.getVrfID(p.GetTenant(), p.GetNamespace())
	if err != nil {
		return fmt.Errorf("failed to find tenant/namespace, %s/%s", p.GetTenant(), p.GetNamespace())
	}

	netMeta := &networkMeta{
		vrf:       vrf,
		tenant:    p.GetTenant(),
		namespace: p.GetNamespace(),
	}

	// hal tracking object to save in db
	dbFlowExp := &tpaprotos.FlowExportPolicyObj{}
	dbFlowExp.TypeMeta = p.TypeMeta
	dbFlowExp.ObjectMeta = p.ObjectMeta
	dbFlowExp.P = p
	dbFlowExp.FlowMonObj = []*tpaprotos.FlowMonitorObj{}

	// clean up on error
	defer func() {
		if err != nil {
			for _, k := range dbFlowExp.FlowMonObj {
				s.deleteFlowMonitorRule(ctx, netMeta, k)
			}
		}
	}()

	for i := 0; i < len(p.Spec.Targets); i++ {
		if err := s.createFlowMonitorRule(ctx, netMeta, dbFlowExp, &p.Spec.Targets[i]); err != nil {
			return fmt.Errorf("failed to create policy, err:%s", err)
		}
	}

	// all success? save it in db
	if err = s.store.Write(dbFlowExp); err != nil {
		log.Errorf("failed to write flow export policy in db, err:%s", err)
		return fmt.Errorf("failed to write flow export policy in db")
	}

	return nil
}

// UpdateFlowExportPolicy is the PUT entry point
func (s *PolicyState) UpdateFlowExportPolicy(ctx context.Context, p *tpmprotos.FlowExportPolicy) error {
	log.Infof("PUT: %+v", p)
	//todo: update or delete/add ?
	return nil
}

// DeleteFlowExportPolicy is the DELETE entry point
func (s *PolicyState) DeleteFlowExportPolicy(ctx context.Context, p *tpmprotos.FlowExportPolicy) error {
	log.Infof("DELETE :%+v", p)

	if err := s.validateMeta(p); err != nil {
		return err
	}

	req := &tpaprotos.FlowExportPolicyObj{}
	req.TypeMeta = p.TypeMeta
	req.ObjectMeta = p.ObjectMeta

	vrf, err := s.getVrfID(p.ObjectMeta.Tenant, p.ObjectMeta.Namespace)
	if err != nil {
		return fmt.Errorf("failed to find tenant/namespace %s/%s", p.GetTenant(), p.GetNamespace())
	}

	netMeta := &networkMeta{
		vrf:       vrf,
		tenant:    p.Tenant,
		namespace: p.Namespace,
	}

	s.Lock()
	defer s.Unlock()

	dbr, err := s.store.Read(req)
	if err != nil || dbr == nil {
		log.Errorf("failed to find flow export policy %s, err:%s", p.Name, err)
		return fmt.Errorf("failed to find flow export policy %s", p.Name)
	}

	flowExp, ok := dbr.(*tpaprotos.FlowExportPolicyObj)
	if !ok {
		log.Errorf("invalid data in flow export db for %s, %+v", p.Name, flowExp)
		return fmt.Errorf("invalid flow export policy")
	}
	for _, k := range flowExp.FlowMonObj {
		s.deleteFlowMonitorRule(ctx, netMeta, k)

	}
	s.store.Delete(req)
	return nil
}

// GetFlowExportPolicy is the GET entry point
func (s *PolicyState) GetFlowExportPolicy(tx context.Context, p *tpmprotos.FlowExportPolicy) (*tpmprotos.FlowExportPolicy, error) {
	log.Infof("GET: %+v", p)

	if err := s.validateMeta(p); err != nil {
		return nil, err
	}

	req := &tpaprotos.FlowExportPolicyObj{}
	req.TypeMeta = p.TypeMeta
	req.ObjectMeta = p.ObjectMeta

	s.Lock()
	dbr, err := s.store.Read(req)
	s.Unlock()
	if err != nil || dbr == nil {
		log.Errorf("failed to read flow export policy, err:%s", err)
		return nil, fmt.Errorf("failed to read flow export policy")
	}

	flowExp, ok := dbr.(*tpaprotos.FlowExportPolicyObj)
	if !ok {
		log.Errorf("invalid type %v in getflow export, %+v", reflect.TypeOf(dbr))
		return nil, fmt.Errorf("failed to read data from flow export policy")
	}

	return flowExp.GetP(), nil
}

// ListFlowExportPolicy is the LIST all entry point
func (s *PolicyState) ListFlowExportPolicy(tx context.Context) ([]*tpmprotos.FlowExportPolicy, error) {
	log.Infof("LIST:")

	req := &tpaprotos.FlowExportPolicyObj{}
	req.Kind = "FlowExportPolicy"

	s.Lock()
	objList, err := s.store.List(req)
	s.Unlock()

	if err != nil {
		log.Errorf("failed to list flow export policy, err:%s", err)
		return []*tpmprotos.FlowExportPolicy{}, fmt.Errorf("failed to list flow export policy")
	}

	flowExpList := []*tpmprotos.FlowExportPolicy{}
	for _, obj := range objList {
		s.Lock()
		readObj, err := s.store.Read(obj)
		s.Unlock()
		if err != nil || readObj == nil {
			log.Errorf("failed to read flow export policy, err:%s", err)
			return []*tpmprotos.FlowExportPolicy{}, fmt.Errorf("failed to read flow export policy")
		}
		flowExp, ok := readObj.(*tpaprotos.FlowExportPolicyObj)
		if !ok {
			log.Errorf("invalid type %v in  export", reflect.TypeOf(readObj))
			return []*tpmprotos.FlowExportPolicy{}, fmt.Errorf("failed to read data from flow export policy")
		}
		flowExpList = append(flowExpList, flowExp.P)
	}

	return flowExpList, nil
}
