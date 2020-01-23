// +build iris

package iris

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"

	//utils2 "github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"net"
	"strconv"
	"syscall"
	"time"

	"github.com/pkg/errors"
	"golang.org/x/sys/unix"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/ipfix"
	"github.com/pensando/sw/venice/utils/log"
)

var netflowSessionToFlowMonitorRuleMapping = map[string][]*halapi.FlowMonitorRuleKeyHandle{}
var netflowSessionToCollectorIDMapping = map[string][]*halapi.CollectorKeyHandle{}

var templateContextMap = map[*halapi.CollectorKeyHandle]context.CancelFunc{}

// HandleFlowExportPolicy handles crud operations on netflow session
func HandleFlowExportPolicy(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, oper types.Operation, netflow netproto.FlowExportPolicy, vrfID uint64) error {
	switch oper {
	case types.Create:
		return createFlowExportPolicyHandler(infraAPI, telemetryClient, intfClient, epClient, netflow, vrfID)
	case types.Update:
		return updateFlowExportPolicyHandler(infraAPI, telemetryClient, intfClient, epClient, netflow, vrfID)
	case types.Delete:
		return deleteFlowExportPolicyHandler(infraAPI, telemetryClient, intfClient, epClient, netflow, vrfID)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createFlowExportPolicyHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, netflow netproto.FlowExportPolicy, vrfID uint64) error {
	var collectorKeys []*halapi.CollectorKeyHandle
	mgmtIP, _, _ := net.ParseCIDR(infraAPI.GetConfig().MgmtIP)
	for _, c := range netflow.Spec.Exports {
		var destPort int
		dstIP := c.Destination
		if err := CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, netflow.GetKey(), mgmtIP.String(), dstIP, false); err != nil {
			log.Error(errors.Wrapf(types.ErrNetflowCreateLateralObjects, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
			return errors.Wrapf(types.ErrMirrorCreateLateralObjects, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
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
		//// Lookup if existing collector EP is known
		//if knownEP, ok := isCollectorEPKnown(infraAPI, collectorEP); ok {
		//	if !reflect.DeepEqual(collectorEP.Spec.IPv4Addresses, knownEP.Spec.IPv4Addresses) {
		//		log.Infof("Netflow Pipeline Handler: %s", types.InfoKnownEPUpdateNeeded)
		//		knownEP.Spec.IPv4Addresses = append(knownEP.Spec.IPv4Addresses, dstIP)
		//		err := updateEndpointHandler(infraAPI, epClient, intfClient, knownEP, vrfID, types.UntaggedCollVLAN)
		//		if err != nil {
		//			log.Error(errors.Wrapf(types.ErrCollectorEPUpdateFailure, "FlowExportPolicy: %s | CollectorEP: %s | Err: %v", netflow.GetKey(), collectorEP.GetKey(), err))
		//			return errors.Wrapf(types.ErrCollectorEPUpdateFailure, "FlowExportPolicy: %s | CollectorEP: %s | Err: %v", netflow.GetKey(), collectorEP.GetKey(), err)
		//		}
		//	} else {
		//		log.Infof("Netflow Pipeline Handler: %s", types.InfoKnownEPNoUpdateNeeded)
		//	}
		//} else {
		//	log.Infof("Netflow Pipeline Handler: %s", types.InfoUnknownEPCreateNeeded)
		//	err := createEndpointHandler(infraAPI, epClient, intfClient, collectorEP, vrfID, types.UntaggedCollVLAN)
		//	if err != nil {
		//		log.Error(errors.Wrapf(types.ErrCollectorEPCreateFailure, "FlowExportPolicy: %s | CollectorEP: %s | Err: %v", netflow.GetKey(), collectorEP.GetKey(), err))
		//		return errors.Wrapf(types.ErrCollectorEPCreateFailure, "FlowExportPolicy: %s | CollectorEP: %s | Err: %v", netflow.GetKey(), collectorEP.GetKey(), err)
		//	}
		//}

		// Create HAL Collector
		l2SegID := getL2SegByCollectorIP(infraAPI, dstIP)
		collectorReqMsg := convertCollector(infraAPI, c, netflow, vrfID, l2SegID)
		resp, err := telemetryClient.CollectorCreate(context.Background(), collectorReqMsg)
		if resp != nil {
			if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Collector Create Failed for %s | %s", netflow.GetKind(), netflow.GetKey())); err != nil {
				return err
			}
		}
		collectorKeys = append(collectorKeys, collectorReqMsg.Request[0].GetKeyOrHandle())

		templateCtx, cancel := context.WithCancel(context.Background())
		templateContextMap[collectorReqMsg.Request[0].GetKeyOrHandle()] = cancel
		destIP := net.ParseIP(c.Destination)
		if c.Transport == nil {
			destPort = types.DefaultNetflowExportPort
		} else {
			destPort, _ = strconv.Atoi(c.Transport.Port)
		}
		go sendTemplate(templateCtx, infraAPI, destIP, destPort, netflow)

	}

	flowMonitorReqMsg, flowMonitorIDs := convertFlowMonitor(actionCollectFlowStats, infraAPI, netflow.Spec.MatchRules, nil, collectorKeys, vrfID)
	resp, err := telemetryClient.FlowMonitorRuleCreate(context.Background(), flowMonitorReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("FlowMonitorRule Create Failed for %s | %s", netflow.GetKind(), netflow.GetKey())); err != nil {
			return err
		}
	}

	netflowSessionToFlowMonitorRuleMapping[netflow.GetKey()] = flowMonitorIDs
	netflowSessionToCollectorIDMapping[netflow.GetKey()] = collectorKeys
	dat, _ := netflow.Marshal()

	if err := infraAPI.Store(netflow.Kind, netflow.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "FlowExportPolicy: %s | FlowExportPolicy: %v", netflow.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "FlowExportPolicy: %s | FlowExportPolicy: %v", netflow.GetKey(), err)
	}
	return nil
}

func updateFlowExportPolicyHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, netflow netproto.FlowExportPolicy, vrfID uint64) error {
	if err := deleteFlowExportPolicyHandler(infraAPI, telemetryClient, intfClient, epClient, netflow, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrFlowExportPolicyDeleteDuringUpdate, "FlowExportPolicy: %s | FlowExportPolicy: %v", netflow.GetKey(), err))
	}
	if err := createFlowExportPolicyHandler(infraAPI, telemetryClient, intfClient, epClient, netflow, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrFlowExportPolicyCreateDuringUpdate, "FlowExportPolicy: %s | FlowExportPolicy: %v", netflow.GetKey(), err))
		return errors.Wrapf(types.ErrFlowExportPolicyCreateDuringUpdate, "FlowExportPolicy: %s | FlowExportPolicy: %v", netflow.GetKey(), err)
	}
	return nil
}

func deleteFlowExportPolicyHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, netflow netproto.FlowExportPolicy, vrfID uint64) error {
	// Delete Flow Monitor rules
	var flowMonitorDeleteReq halapi.FlowMonitorRuleDeleteRequestMsg
	var collectorDeleteReq halapi.CollectorDeleteRequestMsg

	for _, flowMonitorKey := range netflowSessionToFlowMonitorRuleMapping[netflow.GetKey()] {
		req := &halapi.FlowMonitorRuleDeleteRequest{
			KeyOrHandle:  flowMonitorKey,
			VrfKeyHandle: convertVrfKeyHandle(vrfID),
		}
		flowMonitorDeleteReq.Request = append(flowMonitorDeleteReq.Request, req)
	}

	resp, err := telemetryClient.FlowMonitorRuleDelete(context.Background(), &flowMonitorDeleteReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.Response[0].ApiStatus, err, fmt.Sprintf("FlowMonitorRule Delete Failed for %s | %s", netflow.GetKind(), netflow.GetKey())); err != nil {
			return err
		}
	}

	// Clean up state. This is needed because Telemetry doesn't embed rules inside of the object like NetworkSecurityPolicy.
	// TODO Remove this hack once HAL side's telemetry code is cleaned up and DSCAgent must not maintain any internal state
	delete(netflowSessionToFlowMonitorRuleMapping, netflow.GetKey())

	var collectorKeys []*halapi.CollectorKeyHandle
	for _, c := range netflow.Spec.Exports {
		dstIP := c.Destination
		if err := DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, netflow.GetKey(), netflow.String(), dstIP, false); err != nil {
			log.Error(errors.Wrapf(types.ErrNetflowDeleteLateralObjects, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
			return errors.Wrapf(types.ErrNetflowDeleteLateralObjects, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
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
		//	log.Infof("Netflow Pipeline Handler: %s", types.InfoCollectorEPDeleteNeeded)
		//	err := deleteEndpointHandler(infraAPI, epClient, intfClient, knownEP, vrfID, types.UntaggedCollVLAN)
		//	if err != nil {
		//		log.Error(errors.Wrapf(types.ErrCollectorEPDeleteFailure, "FlowExportPolicy: %s | CollectorEP: %s | Err: %v", netflow.GetKey(), collectorEP.GetKey(), err))
		//		return errors.Wrapf(types.ErrCollectorEPDeleteFailure, "FlowExportPolicy: %s | CollectorEP: %s | Err: %v", netflow.GetKey(), collectorEP.GetKey(), err)
		//	}
		//}

		//collectorReqMsg := convertCollector(infraAPI, c, netflow, vrfID)
		//collectorKeys = append(collectorKeys, collectorReqMsg.Request[0].GetKeyOrHandle())
		//cancel := templateContextMap[collectorReqMsg.Request[0].GetKeyOrHandle()]
		//if cancel != nil {
		//	cancel()
		//}
	}

	collectorKeys, _ = netflowSessionToCollectorIDMapping[netflow.GetKey()]

	for _, collectorKey := range collectorKeys {
		cancel := templateContextMap[collectorKey]
		if cancel != nil {
			cancel()
		}
		req := &halapi.CollectorDeleteRequest{
			KeyOrHandle: collectorKey,
		}
		collectorDeleteReq.Request = append(collectorDeleteReq.Request, req)
	}

	cResp, err := telemetryClient.CollectorDelete(context.Background(), &collectorDeleteReq)
	if cResp != nil {
		if err := utils.HandleErr(types.Delete, cResp.Response[0].ApiStatus, err, fmt.Sprintf("FlowMonitorRule Delete Failed for %s | %s", netflow.GetKind(), netflow.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(netflow.Kind, netflow.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
	}
	return nil
}

func convertCollector(infraAPI types.InfraAPI, collector netproto.ExportConfig, netflow netproto.FlowExportPolicy, vrfID, l2SegID uint64) *halapi.CollectorRequestMsg {
	var port uint64
	var protocol halapi.IPProtocol
	mgmtIP, _, _ := net.ParseCIDR(infraAPI.GetConfig().MgmtIP)
	srcIP := utils.ConvertIPAddresses(mgmtIP.String())[0]
	dstIP := utils.ConvertIPAddresses(collector.Destination)[0]
	if collector.Transport != nil {
		port, _ = strconv.ParseUint(collector.Transport.GetPort(), 10, 64)
		protocol = halapi.IPProtocol(convertProtocol(collector.Transport.Protocol))
	}
	interval, _ := time.ParseDuration(netflow.Spec.Interval)

	// TODO fix collector destination to use the same protocol ports

	return &halapi.CollectorRequestMsg{
		Request: []*halapi.CollectorSpec{
			{
				KeyOrHandle: &halapi.CollectorKeyHandle{
					KeyOrHandle: &halapi.CollectorKeyHandle_CollectorId{
						CollectorId: infraAPI.AllocateID(types.CollectorID, 0),
					},
				},
				VrfKeyHandle: convertVrfKeyHandle(vrfID),
				Encap: &halapi.EncapInfo{
					EncapType:  halapi.EncapType_ENCAP_TYPE_DOT1Q,
					EncapValue: types.UntaggedCollVLAN,
				},
				L2SegKeyHandle: convertL2SegKeyHandle(l2SegID),
				SrcIp:          srcIP,
				DestIp:         dstIP,
				Protocol:       protocol,
				DestPort:       uint32(port),
				ExportInterval: uint32(interval.Seconds()),
				Format:         halapi.ExportFormat_IPFIX,
			},
		},
	}
}

//func (s *PolicyState) SendTemplates(ctx context.Context, ckey *types.CollectorKey) error {
//	dest := ckey.Destination
//	dport := int(ckey.Port)
//	interval := time.Duration(ckey.TemplateInterval) * time.Second
//
//	tick := time.NewTicker(interval)
//
//	tmplt, err := ipfix.CreateTemplateMsg()
//	if err != nil {
//		log.Errorf("failed to generate template, %v", err)
//		return err
//	}
//
//	nc := net.ListenConfig{Control: func(network, address string, c syscall.RawConn) error {
//		var sockErr error
//		if err := c.Control(func(fd uintptr) {
//			sockErr = unix.SetsockoptInt(int(fd), unix.SOL_SOCKET, unix.SO_REUSEPORT, 1)
//		}); err != nil {
//			return err
//		}
//		return sockErr
//	},
//	}
//
//	conn, err := nc.ListenPacket(ctx, "udp", fmt.Sprintf("%v:%v", s.getMgmtIPAddr(), ipfixSrcPort))
//	if err != nil {
//		log.Errorf("failed to bind %v, %v", fmt.Sprintf("%v:%v", s.getMgmtIPAddr(), ipfixSrcPort), err)
//		return err
//	}
//
//	tctx, cancel := context.WithCancel(context.Background())
//	templateCtx := &ipfixTemplateContext{
//		cancel: cancel,
//		tick:   tick,
//	}
//
//	s.ipfixCtx.Store(ckey.String(), templateCtx)
//
//	go func() {
//		defer conn.Close()
//		// send first template as soon as we start
//		if _, err := conn.WriteTo(tmplt, &net.UDPAddr{IP: net.ParseIP(dest), Port: dport}); err != nil {
//			log.Errorf("failed to send to %v:%v, %v", dest, dport, err)
//			atomic.AddUint64(&templateCtx.txErr, 1)
//		} else {
//			atomic.AddUint64(&templateCtx.txMsg, 1)
//		}
//
//		for {
//			select {
//			case <-tick.C:
//				if _, err := conn.WriteTo(tmplt, &net.UDPAddr{IP: net.ParseIP(dest), Port: dport}); err != nil {
//					log.Errorf("failed to send to %v", err)
//					atomic.AddUint64(&templateCtx.txErr, 1)
//				} else {
//					atomic.AddUint64(&templateCtx.txMsg, 1)
//				}
//			case <-tctx.Done():
//				log.Infof("timer stopped, stopping templates to %v:%v", dest, dport)
//				return
//			}
//		}
//	}()
//
//	return nil
//}

// TODO Remove this once the HAL side telemetry code is cleaned up. Agents must not be sending raw packets on sockets
func sendTemplate(ctx context.Context, infraAPI types.InfraAPI, destIP net.IP, destPort int, netflow netproto.FlowExportPolicy) {
	log.Infof("FlowExportPolicy | %s Collector: %s", types.InfoTemplateSendStart, destIP.String())
	var templateTicker *time.Ticker
	if len(netflow.Spec.TemplateInterval) > 0 {
		duration, _ := time.ParseDuration(netflow.Spec.TemplateInterval)
		templateTicker = time.NewTicker(duration)
	} else {
		templateTicker = time.NewTicker(types.DefaultTemplateDuration)
	}

	template, err := ipfix.CreateTemplateMsg()
	if err != nil {
		log.Error(errors.Wrapf(types.ErrIPFIXTemplateCreate, "FlowExportPolicy: %s  | Err: %v", netflow.GetKey(), err))
		return
	}

	nc := net.ListenConfig{
		Control: func(network, address string, c syscall.RawConn) error {
			var sockErr error
			if err := c.Control(func(fd uintptr) {
				sockErr = unix.SetsockoptInt(int(fd), unix.SOL_SOCKET, unix.SO_REUSEPORT, 1)
			}); err != nil {
				log.Error(errors.Wrapf(types.ErrIPFIXTemplateSockSend, "FlowExportPolicy: %s  | Err: %v", netflow.GetKey(), err))
			}
			return sockErr
		},
	}

	mgmtIP, _, _ := net.ParseCIDR(infraAPI.GetConfig().MgmtIP)

	conn, err := nc.ListenPacket(ctx, "udp", fmt.Sprintf("%v:%v", mgmtIP, types.IPFIXSrcPort))
	if err != nil {
		log.Error(errors.Wrapf(types.ErrIPFIXPacketListen, "FlowExportPolicy: %s  | Err: %v", netflow.GetKey(), err))
		return
	}
	defer conn.Close()

	for {
		select {
		case <-ctx.Done():
			log.Infof("FlowExportPolicy | Collector: %s", types.InfoTemplateSendStop)
			return
		case <-templateTicker.C:
			if _, err := conn.WriteTo(template, &net.UDPAddr{IP: destIP, Port: destPort}); err != nil {
				log.Error(errors.Wrapf(types.ErrIPFIXPacketSend, "FlowExportPolicy: %s  | Err: %v", netflow.GetKey(), err))
			}
		}
	}
}

func getL2SegByCollectorIP(i types.InfraAPI, destIP string) (l2SegID uint64) {
	eDat, err := i.List("Endpoint")
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Err: %v", types.ErrObjNotFound))
		l2SegID = types.UntaggedCollVLAN
		return
	}

	for _, o := range eDat {
		var endpoint netproto.Endpoint
		err := endpoint.Unmarshal(o)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
			continue
		}
		for _, address := range endpoint.Spec.IPv4Addresses {
			if address == destIP {
				var linkedNetwork netproto.Network
				obj := netproto.Network{
					TypeMeta: api.TypeMeta{
						Kind: "Network",
					},
					ObjectMeta: api.ObjectMeta{
						Name:      endpoint.Spec.NetworkName,
						Tenant:    "default",
						Namespace: "default",
					},
				}
				dat, err := i.Read(obj.Kind, obj.GetKey())
				if err != nil {
					log.Error(errors.Wrapf(types.ErrBadRequest, "Err: %v", types.ErrObjNotFound))
					l2SegID = types.UntaggedCollVLAN
					return
				}
				err = linkedNetwork.Unmarshal(dat)
				l2SegID = linkedNetwork.Status.NetworkID
				return
			}
		}
	}
	l2SegID = types.UntaggedCollVLAN
	return
}
