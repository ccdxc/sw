// +build iris

package iris

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"

	//utils2 "github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"net"
	"strconv"
	"strings"
	"syscall"
	"time"

	"github.com/pkg/errors"
	"golang.org/x/sys/unix"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	commonUtils "github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/ipfix"
	"github.com/pensando/sw/venice/utils/log"
)

type netflowIDs struct {
	collectorID uint64
	NetflowKeys []string
}

var netflowSessionToFlowMonitorRuleMapping = map[string][]*halapi.FlowMonitorRuleKeyHandle{}

var templateContextMap = map[string]context.CancelFunc{}

// CollectorToNetflow maps the unique collector to netflowID
var CollectorToNetflow = map[string]*netflowIDs{}

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
	for _, c := range netflow.Spec.Exports {
		var destPort int
		var collectorID uint64
		var foundCollector bool
		dstIP := c.Destination
		collectorKey := commonUtils.BuildCollectorKey(netflow.Spec.VrfName, c)

		netflows, ok := CollectorToNetflow[collectorKey]
		if ok {
			netflows.NetflowKeys = append(netflows.NetflowKeys, netflow.GetKey())
			collectorID = netflows.collectorID
			foundCollector = true
		} else {
			collectorID = infraAPI.AllocateID(types.CollectorID, 0)
		}
		collectorKeys = append(collectorKeys, convertCollectorKeyHandle(collectorID))
		if foundCollector {
			continue
		}
		compositeKey := fmt.Sprintf("%s/%s", netflow.GetKind(), collectorKey)
		mgmtIP := commonUtils.GetMgmtIP(MgmtLink)
		if err := CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, compositeKey, mgmtIP, dstIP, false); err != nil {
			log.Error(errors.Wrapf(types.ErrNetflowCreateLateralObjects, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
			return errors.Wrapf(types.ErrMirrorCreateLateralObjects, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
		}
		// Create HAL Collector
		l2SegID := getL2SegByCollectorIP(infraAPI, dstIP)
		collectorReqMsg := convertCollector(infraAPI, c, netflow, vrfID, l2SegID, collectorID)
		resp, err := telemetryClient.CollectorCreate(context.Background(), collectorReqMsg)
		if resp != nil {
			if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Collector Create Failed for %s | %s", netflow.GetKind(), netflow.GetKey())); err != nil {
				return err
			}
		}

		CollectorToNetflow[collectorKey] = &netflowIDs{
			collectorID: collectorID,
			NetflowKeys: []string{netflow.GetKey()},
		}
		templateCtx, cancel := context.WithCancel(context.Background())
		templateContextMap[collectorKey] = cancel
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
	dat, _ := netflow.Marshal()

	if err := infraAPI.Store(netflow.Kind, netflow.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "FlowExportPolicy: %s | FlowExportPolicy: %v", netflow.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "FlowExportPolicy: %s | FlowExportPolicy: %v", netflow.GetKey(), err)
	}
	return nil
}

func updateFlowExportPolicyHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, netflow netproto.FlowExportPolicy, vrfID uint64) error {
	var existingNetflow netproto.FlowExportPolicy
	dat, err := infraAPI.Read(netflow.Kind, netflow.GetKey())
	if err != nil {
		return err
	}
	err = existingNetflow.Unmarshal(dat)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrUnmarshal, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
		return errors.Wrapf(types.ErrUnmarshal, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
	}
	if err := deleteFlowExportPolicyHandler(infraAPI, telemetryClient, intfClient, epClient, existingNetflow, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrFlowExportPolicyDeleteDuringUpdate, "FlowExportPolicy: %s | FlowExportPolicy: %v", existingNetflow.GetKey(), err))
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

	for _, c := range netflow.Spec.Exports {
		dstIP := c.Destination
		cKey := commonUtils.BuildCollectorKey(netflow.Spec.VrfName, c)
		netflowKeys, ok := CollectorToNetflow[cKey]
		if !ok {
			log.Error(errors.Wrapf(types.ErrCollectorAlreadyDeleted, "FlowExportPolicy: %s | DestKey: %s", netflow.GetKey(), cKey))
			return errors.Wrapf(types.ErrCollectorAlreadyDeleted, "FlowExportPolicy: %s | DestKey: %s", netflow.GetKey(), cKey)
		}
		// Remove the mirror key from the map
		length := len(netflowKeys.NetflowKeys)
		index := -1
		for idx, n := range netflowKeys.NetflowKeys {
			if n == netflow.GetKey() {
				index = idx
				break
			}
		}
		if index != -1 {
			netflowKeys.NetflowKeys[index] = netflowKeys.NetflowKeys[length-1]
			netflowKeys.NetflowKeys = netflowKeys.NetflowKeys[:length-1]
		}
		if len(netflowKeys.NetflowKeys) == 0 {
			collectorKey := convertCollectorKeyHandle(netflowKeys.collectorID)
			cancel := templateContextMap[cKey]
			if cancel != nil {
				cancel()
			}
			collectorDeleteReq := &halapi.CollectorDeleteRequestMsg{
				Request: []*halapi.CollectorDeleteRequest{
					{
						KeyOrHandle: collectorKey,
					},
				},
			}
			cResp, err := telemetryClient.CollectorDelete(context.Background(), collectorDeleteReq)
			if cResp != nil {
				if err := utils.HandleErr(types.Delete, cResp.Response[0].ApiStatus, err, fmt.Sprintf("FlowMonitorRule Delete Failed for collector %s | %s", netflow.GetKind(), cKey)); err != nil {
					return err
				}
			}
			delete(CollectorToNetflow, cKey)
			compositeKey := fmt.Sprintf("%s/%s", netflow.GetKind(), cKey)
			if err := DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, vrfID, compositeKey, dstIP, false); err != nil {
				log.Error(errors.Wrapf(types.ErrNetflowDeleteLateralObjects, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
				return errors.Wrapf(types.ErrNetflowDeleteLateralObjects, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
			}
		} else {
			log.Infof("NetflowCollector: %s | DstIP: %s | VrfName: %s still referenced by %s", netflow.GetKey(), dstIP, netflow.Spec.VrfName, strings.Join(netflowKeys.NetflowKeys, " "))
		}
	}

	if err := infraAPI.Delete(netflow.Kind, netflow.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
	}
	return nil
}

func convertCollector(infraAPI types.InfraAPI, collector netproto.ExportConfig, netflow netproto.FlowExportPolicy, vrfID, l2SegID, collectorID uint64) *halapi.CollectorRequestMsg {
	var port uint64
	var protocol halapi.IPProtocol
	mgmtIP := commonUtils.GetMgmtIP(MgmtLink)
	srcIP := utils.ConvertIPAddresses(mgmtIP)[0]
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
				KeyOrHandle:  convertCollectorKeyHandle(collectorID),
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

	mgmtIP := commonUtils.GetMgmtIP(MgmtLink)
	conn, err := nc.ListenPacket(ctx, "udp", fmt.Sprintf("%v:%v", mgmtIP, types.IPFIXSrcPort))
	if err != nil {
		log.Error(errors.Wrapf(types.ErrIPFIXPacketListen, "FlowExportPolicy: %s  | Err: %v", netflow.GetKey(), err))
		return
	}
	defer conn.Close()

	for {
		select {
		case <-ctx.Done():
			log.Infof("FlowExportPolicy | %s Collector: %s", types.InfoTemplateSendStop, destIP.String())
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

	pip := net.ParseIP(destIP).String()
	for _, o := range eDat {
		var endpoint netproto.Endpoint
		err := endpoint.Unmarshal(o)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
			continue
		}
		for _, address := range endpoint.Spec.IPv4Addresses {
			epIP, _, _ := net.ParseCIDR(address)
			if epIP.String() == pip {
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

func convertCollectorKeyHandle(collectorID uint64) *halapi.CollectorKeyHandle {
	return &halapi.CollectorKeyHandle{
		KeyOrHandle: &halapi.CollectorKeyHandle_CollectorId{
			CollectorId: collectorID,
		},
	}
}
