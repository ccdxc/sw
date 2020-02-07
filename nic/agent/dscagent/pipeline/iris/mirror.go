// +build iris

package iris

import (
	"context"
	"fmt"
	"net"
	"strings"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	actionMirror = iota
	actionCollectFlowStats
)

var mirrorSessionToFlowMonitorRuleMapping = map[string][]*halapi.FlowMonitorRuleKeyHandle{}
var MirrorSessionToInterfaceMapping = map[string][]string{}

// HandleMirrorSession handles crud operations on mirror session
func HandleMirrorSession(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, oper types.Operation, mirror netproto.MirrorSession, vrfID uint64) error {
	switch oper {
	case types.Create:
		return createMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID)
	case types.Update:
		return updateMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID, oper)
	case types.Delete:
		return deleteMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID, oper)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createMirrorSessionHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, mirror netproto.MirrorSession, vrfID uint64) error {
	var mirrorKeys []*halapi.MirrorSessionKeyHandle
	mgmtIP, _, _ := net.ParseCIDR(infraAPI.GetConfig().MgmtIP)
	for _, c := range mirror.Spec.Collectors {
		dstIP := c.ExportCfg.Destination
		if err := CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, mirror.GetKey(), mgmtIP.String(), dstIP, true); err != nil {
			log.Error(errors.Wrapf(types.ErrMirrorCreateLateralObjects, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return errors.Wrapf(types.ErrMirrorCreateLateralObjects, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}
		//dMAC, ok := arpCache[dstIP]
		//if !ok {
		//	log.Error(errors.Wrapf(types.ErrARPMissingDMAC, "IP: %s", dstIP))
		//}
		//
		//collectorEP := netproto.Endpoint{
		//	TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		//	ObjectMeta: api.ObjectMeta{
		//		Tenant:    types.DefaultVrf,
		//		Namespace: types.DefaultNamespace,
		//		Name:      fmt.Sprintf("_internal_collector_ep_%s", dMAC),
		//	},
		//	Spec: netproto.EndpointSpec{
		//		NetworkName:   types.InternalDefaultUntaggedNetwork,
		//		NodeUUID:      "REMOTE",
		//		IPv4Addresses: []string{dstIP},
		//		MacAddress:    dMAC,
		//	},
		//}
		//
		//// Lookup if existing Tunnel
		//mgmtIP, _, _ := net.ParseCIDR(infraAPI.GetConfig().MgmtIP)
		//collectorTunnel := netproto.Tunnel{
		//	TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		//	ObjectMeta: api.ObjectMeta{
		//		Tenant:    types.DefaultVrf,
		//		Namespace: types.DefaultNamespace,
		//		Name:      fmt.Sprintf("_internal_collector_tunnel_%s", dstIP),
		//	},
		//	Spec: netproto.TunnelSpec{
		//		Type:        "GRE",
		//		AdminStatus: "UP",
		//		Src:         mgmtIP.String(),
		//		Dst:         dstIP,
		//	},
		//	Status: netproto.TunnelStatus{
		//		TunnelID: infraAPI.AllocateID(types.TunnelID, types.TunnelOffset),
		//	},
		//}
		//
		//// Lookup if existing collector EP is known
		//if knownEP, ok := isCollectorEPKnown(infraAPI, collectorEP); ok {
		//	if !reflect.DeepEqual(collectorEP.Spec.IPv4Addresses, knownEP.Spec.IPv4Addresses) {
		//		log.Infof("Mirror Pipeline Handler: %s", types.InfoKnownEPUpdateNeeded)
		//		knownEP.Spec.IPv4Addresses = append(knownEP.Spec.IPv4Addresses, dstIP)
		//		err := updateEndpointHandler(infraAPI, epClient, intfClient, knownEP, vrfID, types.UntaggedCollVLAN)
		//		if err != nil {
		//			log.Error(errors.Wrapf(types.ErrCollectorEPUpdateFailure, "MirrorSession: %s | CollectorEP: %s | Err: %v", mirror.GetKey(), collectorEP.GetKey(), err))
		//			return errors.Wrapf(types.ErrCollectorEPUpdateFailure, "MirrorSession: %s | CollectorEP: %s | Err: %v", mirror.GetKey(), collectorEP.GetKey(), err)
		//		}
		//	} else {
		//		log.Infof("Mirror Pipeline Handler: %s", types.InfoKnownEPNoUpdateNeeded)
		//	}
		//} else {
		//	log.Infof("Mirror Pipeline Handler: %s", types.InfoUnknownEPCreateNeeded)
		//	err := createEndpointHandler(infraAPI, epClient, intfClient, collectorEP, vrfID, types.UntaggedCollVLAN)
		//	if err != nil {
		//		log.Error(errors.Wrapf(types.ErrCollectorEPCreateFailure, "MirrorSession: %s | CollectorEP: %s | Err: %v", mirror.GetKey(), collectorEP.GetKey(), err))
		//		return errors.Wrapf(types.ErrCollectorEPCreateFailure, "MirrorSession: %s | CollectorEP: %s | Err: %v", mirror.GetKey(), collectorEP.GetKey(), err)
		//	}
		//}
		//
		//// TODO Update EP Refcounts
		//
		//if _, ok := isCollectorTunnelKnown(infraAPI, collectorTunnel); !ok {
		//	log.Infof("Mirror Pipeline Handler: %s", types.InfoUnknownTunnelCreateNeeded)
		//	err := createTunnelHandler(infraAPI, intfClient, collectorTunnel, vrfID)
		//	if err != nil {
		//		log.Error(errors.Wrapf(types.ErrCollectorTunnelCreateFailure, "MirrorSession: %s | CollectorEP: %s | Err: %v", mirror.GetKey(), collectorTunnel.GetKey(), err))
		//		return errors.Wrapf(types.ErrCollectorTunnelCreateFailure, "MirrorSession: %s | CollectorEP: %s | Err: %v", mirror.GetKey(), collectorTunnel.GetKey(), err)
		//	}
		//}
		//
		//// TODO Update Tunnel Refcounts

		// Create MirrorSessions
		mirrorReqMsg := convertMirrorSession(mirror, dstIP, vrfID)
		resp, err := telemetryClient.MirrorSessionCreate(context.Background(), mirrorReqMsg)
		if resp != nil {
			if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("MirrorSession Create Failed for %s | %s", mirror.GetKind(), mirror.GetKey())); err != nil {
				return err
			}
		}
		mirrorKeys = append(mirrorKeys, mirrorReqMsg.Request[0].GetKeyOrHandle())
	}

	flowMonitorReqMsg, flowMonitorIDs := convertFlowMonitor(actionMirror, infraAPI, mirror.Spec.MatchRules, mirrorKeys, nil, vrfID)
	resp, err := telemetryClient.FlowMonitorRuleCreate(context.Background(), flowMonitorReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("FlowMonitorRule Create Failed for %s | %s", mirror.GetKind(), mirror.GetKey())); err != nil {
			return err
		}
	}

	mirrorSessionToFlowMonitorRuleMapping[mirror.GetKey()] = flowMonitorIDs
	dat, _ := mirror.Marshal()

	if err := infraAPI.Store(mirror.Kind, mirror.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "MirrorSession: %s | MirrorSession: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "MirrorSession: %s | MirrorSession: %v", mirror.GetKey(), err)
	}
	return nil
}

func updateMirrorSessionHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, mirror netproto.MirrorSession, vrfID uint64, oper types.Operation) error {
	if err := deleteMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID, oper); err != nil {
		log.Error(errors.Wrapf(types.ErrMirrorSessionDeleteDuringUpdate, "MirrorSession: %s | MirrorSession: %v", mirror.GetKey(), err))
	}
	if err := createMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrMirrorSessionCreateDuringUpdate, "MirrorSession: %s | MirrorSession: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrMirrorSessionCreateDuringUpdate, "MirrorSession: %s | MirrorSession: %v", mirror.GetKey(), err)
	}
	return nil
}

func deleteMirrorSessionHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, mirror netproto.MirrorSession, vrfID uint64, oper types.Operation) error {
	// Check if the mirror session is referenced by any interface, only if delete request comes
	if intfs, ok := MirrorSessionToInterfaceMapping[mirror.GetKey()]; ok && oper == types.Delete && len(intfs) != 0 {
		return errors.Wrapf(types.ErrMirrorSessionReferencedByInterface, "MirrorSession: %s referenced by interfaces : %s", mirror.GetKey(), strings.Join(intfs, " "))
	}
	delete(MirrorSessionToInterfaceMapping, mirror.GetKey())

	// Delete Flow Monitor rules
	var flowMonitorDeleteReq halapi.FlowMonitorRuleDeleteRequestMsg
	var mirrorSessionDeleteReq halapi.MirrorSessionDeleteRequestMsg
	mgmtIP, _, _ := net.ParseCIDR(infraAPI.GetConfig().MgmtIP)

	for _, flowMonitorKey := range mirrorSessionToFlowMonitorRuleMapping[mirror.GetKey()] {
		req := &halapi.FlowMonitorRuleDeleteRequest{
			KeyOrHandle:  flowMonitorKey,
			VrfKeyHandle: convertVrfKeyHandle(vrfID),
		}
		flowMonitorDeleteReq.Request = append(flowMonitorDeleteReq.Request, req)
	}

	resp, err := telemetryClient.FlowMonitorRuleDelete(context.Background(), &flowMonitorDeleteReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.Response[0].ApiStatus, err, fmt.Sprintf("FlowMonitorRule Delete Failed for %s | %s", mirror.GetKind(), mirror.GetKey())); err != nil {
			return err
		}
	}

	// Clean up state. This is needed because Telemetry doesn't embed rules inside of the object like NetworkSecurityPolicy.
	// TODO Remove this hack once HAL side's telemetry code is cleaned up and DSCAgent must not maintain any internal state
	delete(mirrorSessionToFlowMonitorRuleMapping, mirror.GetKey())

	var mirrorKeys []*halapi.MirrorSessionKeyHandle
	for _, c := range mirror.Spec.Collectors {
		dstIP := c.ExportCfg.Destination

		if err := DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, mirror.GetKey(), mgmtIP.String(), dstIP, true); err != nil {
			log.Error(errors.Wrapf(types.ErrMirrorDeleteLateralObjects, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return errors.Wrapf(types.ErrMirrorDeleteLateralObjects, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}
		//dMAC, ok := arpCache[dstIP]
		//if !ok {
		//	log.Error(errors.Wrapf(types.ErrARPMissingDMAC, "IP: %s", dstIP))
		//}
		//
		//collectorEP := netproto.Endpoint{
		//	TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		//	ObjectMeta: api.ObjectMeta{
		//		Tenant:    types.DefaultVrf,
		//		Namespace: types.DefaultNamespace,
		//		Name:      fmt.Sprintf("_internal_collector_ep_%s", dMAC),
		//	},
		//}
		//
		//if knownEP, ok := isCollectorEPKnown(infraAPI, collectorEP); ok {
		//	log.Infof("Mirror Pipeline Handler: %s", types.InfoCollectorEPDeleteNeeded)
		//	err := deleteEndpointHandler(infraAPI, epClient, intfClient, knownEP, vrfID, types.UntaggedCollVLAN)
		//	if err != nil {
		//		log.Error(errors.Wrapf(types.ErrCollectorEPDeleteFailure, "MirrorSession: %s | CollectorEP: %s | Err: %v", mirror.GetKey(), collectorEP.GetKey(), err))
		//		return errors.Wrapf(types.ErrCollectorEPDeleteFailure, "MirrorSession: %s | CollectorEP: %s | Err: %v", mirror.GetKey(), collectorEP.GetKey(), err)
		//	}
		//}
		//
		//collectorTunnel := netproto.Tunnel{
		//	TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		//	ObjectMeta: api.ObjectMeta{
		//		Tenant:    types.DefaultVrf,
		//		Namespace: types.DefaultNamespace,
		//		Name:      fmt.Sprintf("_internal_collector_tunnel_%s", dstIP),
		//	},
		//}
		//
		//if knownTunnel, ok := isCollectorTunnelKnown(infraAPI, collectorTunnel); ok {
		//	log.Infof("Mirror Pipeline Handler: %s", types.InfoCollectorTunnelDeleteNeeded)
		//	err := deleteTunnelHandler(infraAPI, intfClient, *knownTunnel)
		//	if err != nil {
		//		log.Error(errors.Wrapf(types.ErrCollectorTunnelDeleteFailure, "MirrorSession: %s | CollectorEP: %s | Err: %v", mirror.GetKey(), collectorTunnel.GetKey(), err))
		//		return errors.Wrapf(types.ErrCollectorTunnelDeleteFailure, "MirrorSession: %s | CollectorEP: %s | Err: %v", mirror.GetKey(), collectorTunnel.GetKey(), err)
		//	}
		//}

		mirrorReqMsg := convertMirrorSession(mirror, dstIP, vrfID)
		mirrorKeys = append(mirrorKeys, mirrorReqMsg.Request[0].GetKeyOrHandle())
	}

	for _, mirrorKey := range mirrorKeys {
		req := &halapi.MirrorSessionDeleteRequest{
			KeyOrHandle: mirrorKey,
		}
		mirrorSessionDeleteReq.Request = append(mirrorSessionDeleteReq.Request, req)
	}

	mResp, err := telemetryClient.MirrorSessionDelete(context.Background(), &mirrorSessionDeleteReq)
	if mResp != nil {
		if err := utils.HandleErr(types.Delete, mResp.Response[0].ApiStatus, err, fmt.Sprintf("FlowMonitorRule Delete Failed for %s | %s", mirror.GetKind(), mirror.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(mirror.Kind, mirror.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
	}
	return nil
}

func convertFlowMonitor(action int, infraAPI types.InfraAPI, matchRules []netproto.MatchRule, mirrorSessionKeys []*halapi.MirrorSessionKeyHandle, collectorKeys []*halapi.CollectorKeyHandle, vrfID uint64) (*halapi.FlowMonitorRuleRequestMsg, []*halapi.FlowMonitorRuleKeyHandle) {
	var flowMonitorRequestMsg halapi.FlowMonitorRuleRequestMsg
	var flowMonitorIDs []*halapi.FlowMonitorRuleKeyHandle
	ruleMatches := convertTelemetryRuleMatches(matchRules)

	for _, m := range ruleMatches {
		var halAction *halapi.MonitorAction
		if action == actionMirror {
			halAction = &halapi.MonitorAction{
				Action: []halapi.RuleAction{
					halapi.RuleAction_MIRROR,
				},
				MsKeyHandle: mirrorSessionKeys,
			}
		} else {
			halAction = &halapi.MonitorAction{
				Action: []halapi.RuleAction{
					halapi.RuleAction_COLLECT_FLOW_STATS,
				},
			}
		}

		req := &halapi.FlowMonitorRuleSpec{
			KeyOrHandle: &halapi.FlowMonitorRuleKeyHandle{
				KeyOrHandle: &halapi.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
					FlowmonitorruleId: infraAPI.AllocateID(types.FlowMonitorRuleID, 0),
				},
			},
			VrfKeyHandle:       convertVrfKeyHandle(vrfID),
			CollectorKeyHandle: collectorKeys,
			Action:             halAction,
			Match:              m,
		}
		flowMonitorRequestMsg.Request = append(flowMonitorRequestMsg.Request, req)
		flowMonitorIDs = append(flowMonitorIDs, req.KeyOrHandle)
	}

	return &flowMonitorRequestMsg, flowMonitorIDs
}

func convertTelemetryRuleMatches(rules []netproto.MatchRule) []*halapi.RuleMatch {
	var ruleMatches []*halapi.RuleMatch

	for _, r := range rules {
		var srcAddress, dstAddress []*halapi.IPAddressObj
		var protocol int32
		var appMatch *halapi.RuleMatch_AppMatch
		if r.Src != nil {
			srcAddress = convertIPAddress(r.Src.Addresses...)
		}
		if r.Dst != nil {
			dstAddress = convertIPAddress(r.Dst.Addresses...)
			if r.Dst.ProtoPorts != nil && len(r.Dst.ProtoPorts) > 0 {
				// Repeated proto ports for Flow Matches is not supported. So accept the first proto port
				pp := r.Dst.ProtoPorts[0]
				protocol = convertProtocol(pp.Protocol)
				if protocol == int32(halapi.IPProtocol_IPPROTO_ICMP) {
					appMatch = &halapi.RuleMatch_AppMatch{
						App: &halapi.RuleMatch_AppMatch_IcmpInfo{
							IcmpInfo: &halapi.RuleMatch_ICMPAppInfo{
								IcmpCode: 0, // TODO Support valid code/type parsing once the App is integrated with mirror
								IcmpType: 0,
							},
						},
					}
				} else {
					appMatch = convertPort(pp.Port)
				}
			}
		}

		m := &halapi.RuleMatch{
			SrcAddress: srcAddress,
			DstAddress: dstAddress,
			Protocol:   protocol,
			AppMatch:   appMatch,
		}
		ruleMatches = append(ruleMatches, m)
	}

	return ruleMatches
}

func convertMirrorSession(mirror netproto.MirrorSession, destIP string, vrfID uint64) *halapi.MirrorSessionRequestMsg {
	return &halapi.MirrorSessionRequestMsg{
		Request: []*halapi.MirrorSessionSpec{
			{
				KeyOrHandle:  convertMirrorSessionKeyHandle(mirror.Status.MirrorSessionID),
				VrfKeyHandle: convertVrfKeyHandle(vrfID),
				Snaplen:      mirror.Spec.PacketSize,
				Destination: &halapi.MirrorSessionSpec_ErspanSpec{ // TODO Fix Destination when more than one collector per MirrorSession is supported
					ErspanSpec: &halapi.ERSpanSpec{
						DestIp: utils.ConvertIPAddresses(destIP)[0],
						SpanId: uint32(mirror.Status.MirrorSessionID),
					},
				},
			},
		},
	}
}

func convertMirrorSessionKeyHandle(mirrorID uint64) *halapi.MirrorSessionKeyHandle {
	return &halapi.MirrorSessionKeyHandle{
		KeyOrHandle: &halapi.MirrorSessionKeyHandle_MirrorsessionId{
			MirrorsessionId: mirrorID,
		},
	}
}
