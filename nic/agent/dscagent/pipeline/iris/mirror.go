// +build iris

package iris

import (
	"context"
	"fmt"

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

// MirrorDestToIDMapping maps the key for each session to keys for sessions created in HAL
var MirrorDestToIDMapping = map[string][]uint64{}

var mirrorSessionToFlowMonitorRuleMapping = map[string][]*halapi.FlowMonitorRuleKeyHandle{}

// HandleMirrorSession handles crud operations on mirror session
func HandleMirrorSession(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, oper types.Operation, mirror netproto.MirrorSession, vrfID uint64) error {
	switch oper {
	case types.Create:
		return createMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID)
	case types.Update:
		return updateMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID)
	case types.Delete:
		return deleteMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createMirrorSessionHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, mirror netproto.MirrorSession, vrfID uint64) error {
	var mirrorKeys []*halapi.MirrorSessionKeyHandle
	mirrorKey := fmt.Sprintf("%s/%s", mirror.Kind, mirror.GetKey())
	for _, c := range mirror.Spec.Collectors {
		sessionID := infraAPI.AllocateID(types.MirrorSessionID, 0)
		colName := fmt.Sprintf("%s-%d", mirrorKey, sessionID)
		// Create collector
		col := buildCollector(colName, sessionID, c, mirror.Spec.PacketSize, mirror.Spec.SpanID)
		if err := HandleCol(infraAPI, telemetryClient, intfClient, epClient, types.Create, col, vrfID); err != nil {
			log.Error(errors.Wrapf(types.ErrCollectorCreate, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return errors.Wrapf(types.ErrCollectorCreate, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}

		// Populate the MirrorDestToIDMapping
		MirrorDestToIDMapping[mirrorKey] = append(MirrorDestToIDMapping[mirrorKey], sessionID)

		// Create MirrorSession handles
		mirrorKeys = append(mirrorKeys, convertMirrorSessionKeyHandle(sessionID))
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

func updateMirrorSessionHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, mirror netproto.MirrorSession, vrfID uint64) error {
	var existingMirror netproto.MirrorSession
	dat, err := infraAPI.Read(mirror.Kind, mirror.GetKey())
	if err != nil {
		return err
	}
	err = existingMirror.Unmarshal(dat)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrUnmarshal, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrUnmarshal, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
	}
	if err := deleteMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, existingMirror, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrMirrorSessionDeleteDuringUpdate, "MirrorSession: %s | MirrorSession: %v", existingMirror.GetKey(), err))
	}
	if err := createMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrMirrorSessionCreateDuringUpdate, "MirrorSession: %s | MirrorSession: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrMirrorSessionCreateDuringUpdate, "MirrorSession: %s | MirrorSession: %v", mirror.GetKey(), err)
	}
	return nil
}

func deleteMirrorSessionHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, mirror netproto.MirrorSession, vrfID uint64) error {
	// Delete Flow Monitor rules
	var flowMonitorDeleteReq halapi.FlowMonitorRuleDeleteRequestMsg

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

	mirrorKey := fmt.Sprintf("%s/%s", mirror.Kind, mirror.GetKey())
	sessionIDs := MirrorDestToIDMapping[mirrorKey]
	for idx, c := range mirror.Spec.Collectors {
		sessionID := sessionIDs[idx]
		colName := fmt.Sprintf("%s-%d", mirrorKey, sessionID)
		// Delete collector to HAL
		col := buildCollector(colName, sessionID, c, mirror.Spec.PacketSize, mirror.Spec.SpanID)
		if err := HandleCol(infraAPI, telemetryClient, intfClient, epClient, types.Delete, col, vrfID); err != nil {
			log.Error(errors.Wrapf(types.ErrCollectorDelete, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
		}
	}

	// Clean up MirrorDestToIDMapping for mirror key
	delete(MirrorDestToIDMapping, mirrorKey)
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
		if r.Dst == nil {
			m := &halapi.RuleMatch{
				SrcAddress: srcAddress,
				DstAddress: dstAddress,
				Protocol:   protocol,
				AppMatch:   appMatch,
			}
			ruleMatches = append(ruleMatches, m)
			continue
		}
		dstAddress = convertIPAddress(r.Dst.Addresses...)
		if r.Dst.ProtoPorts == nil || len(r.Dst.ProtoPorts) == 0 {
			m := &halapi.RuleMatch{
				SrcAddress: srcAddress,
				DstAddress: dstAddress,
				Protocol:   protocol,
				AppMatch:   appMatch,
			}
			ruleMatches = append(ruleMatches, m)
			continue
		}
		for _, pp := range r.Dst.ProtoPorts {
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
			m := &halapi.RuleMatch{
				SrcAddress: srcAddress,
				DstAddress: dstAddress,
				Protocol:   protocol,
				AppMatch:   appMatch,
			}
			ruleMatches = append(ruleMatches, m)
		}
	}

	return ruleMatches
}

func convertMirrorSessionKeyHandle(mirrorID uint64) *halapi.MirrorSessionKeyHandle {
	return &halapi.MirrorSessionKeyHandle{
		KeyOrHandle: &halapi.MirrorSessionKeyHandle_MirrorsessionId{
			MirrorsessionId: mirrorID,
		},
	}
}

func buildCollector(name string, sessionID uint64, mc netproto.MirrorCollector, packetSize, spanID uint32) Collector {
	return Collector{
		Name:         name,
		Destination:  mc.ExportCfg.Destination,
		PacketSize:   packetSize,
		Gateway:      mc.ExportCfg.Gateway,
		Type:         mc.Type,
		StripVlanHdr: mc.StripVlanHdr,
		SessionID:    sessionID,
		SpanID:       spanID,
	}
}
