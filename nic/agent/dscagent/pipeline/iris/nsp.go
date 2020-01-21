// +build iris

package iris

import (
	"context"
	"encoding/json"
	"fmt"
	"net"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"

	iristypes "github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/types"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleNetworkSecurityPolicy handles crud operations on nsp
func HandleNetworkSecurityPolicy(infraAPI types.InfraAPI, client halapi.NwSecurityClient, oper types.Operation, nsp netproto.NetworkSecurityPolicy, vrfID uint64, ruleIDToAppMapping *sync.Map) error {
	switch oper {
	case types.Create:
		return createNetworkSecurityPolicyHandler(infraAPI, client, nsp, vrfID, ruleIDToAppMapping)
	case types.Update:
		if err := deleteNetworkSecurityPolicyHandler(infraAPI, client, nsp, vrfID); err != nil {
			log.Infof("Delete failed during update: %v", err)
		}
		return createNetworkSecurityPolicyHandler(infraAPI, client, nsp, vrfID, ruleIDToAppMapping)
	case types.Delete:
		return deleteNetworkSecurityPolicyHandler(infraAPI, client, nsp, vrfID)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createNetworkSecurityPolicyHandler(infraAPI types.InfraAPI, client halapi.NwSecurityClient, nsp netproto.NetworkSecurityPolicy, vrfID uint64, ruleIDToAppMapping *sync.Map) error {
	c, _ := json.Marshal(nsp)
	log.Infof("Create SGP Req to Agent: %v", string(c))

	nspReqMsg := convertNetworkSecurityPolicy(nsp, vrfID, ruleIDToAppMapping)
	b, _ := json.Marshal(nspReqMsg)
	log.Infof("Create SGP Req to Datapath: %s", string(b))

	resp, err := client.SecurityPolicyCreate(context.Background(), nspReqMsg)
	log.Infof("Datapath Create SGP Response: %v", resp)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", nsp.GetKind(), nsp.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := nsp.Marshal()

	if err := infraAPI.Store(nsp.Kind, nsp.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "NetworkSecurityPolicy: %s | NetworkSecurityPolicy: %v", nsp.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "NetworkSecurityPolicy: %s | NetworkSecurityPolicy: %v", nsp.GetKey(), err)
	}
	return nil
}

func updateNetworkSecurityPolicyHandler(infraAPI types.InfraAPI, client halapi.NwSecurityClient, nsp netproto.NetworkSecurityPolicy, vrfID uint64, ruleIDToAppMapping *sync.Map) error {
	c, _ := json.Marshal(nsp)
	log.Infof("Update SGP Req to Agent: %s", string(c))

	nspReqMsg := convertNetworkSecurityPolicy(nsp, vrfID, ruleIDToAppMapping)
	b, _ := json.Marshal(nspReqMsg)
	log.Infof("Update SGP Req to Datapath: %s", string(b))
	resp, err := client.SecurityPolicyUpdate(context.Background(), nspReqMsg)
	log.Infof("Datapath Update SGP Response: %v", resp)

	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.Response[0].ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", nsp.GetKind(), nsp.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := nsp.Marshal()

	if err := infraAPI.Store(nsp.Kind, nsp.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "NetworkSecurityPolicy: %s | NetworkSecurityPolicy: %v", nsp.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "NetworkSecurityPolicy: %s | NetworkSecurityPolicy: %v", nsp.GetKey(), err)
	}
	return nil
}

func deleteNetworkSecurityPolicyHandler(infraAPI types.InfraAPI, client halapi.NwSecurityClient, nsp netproto.NetworkSecurityPolicy, vrfID uint64) error {

	nspDelReq := &halapi.SecurityPolicyDeleteRequestMsg{
		Request: []*halapi.SecurityPolicyDeleteRequest{
			{
				KeyOrHandle: convertNSPKeyHandle(vrfID, nsp.Status.NetworkSecurityPolicyID),
			},
		},
	}

	resp, err := client.SecurityPolicyDelete(context.Background(), nspDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.Response[0].ApiStatus, err, fmt.Sprintf("NetworkSecurityPolicy: %s", nsp.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(nsp.Kind, nsp.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err)
	}
	return nil
}

func convertNetworkSecurityPolicy(nsp netproto.NetworkSecurityPolicy, vrfID uint64, ruleIDToAppMapping *sync.Map) *halapi.SecurityPolicyRequestMsg {
	fwRules := convertHALFirewallRules(nsp, vrfID, ruleIDToAppMapping)
	return &halapi.SecurityPolicyRequestMsg{
		Request: []*halapi.SecurityPolicySpec{
			{
				KeyOrHandle: convertNSPKeyHandle(vrfID, nsp.Status.NetworkSecurityPolicyID),
				Rule:        fwRules,
			},
		},
	}
}

func convertNSPKeyHandle(vrfID, policyID uint64) *halapi.SecurityPolicyKeyHandle {
	return &halapi.SecurityPolicyKeyHandle{
		PolicyKeyOrHandle: &halapi.SecurityPolicyKeyHandle_SecurityPolicyKey{
			SecurityPolicyKey: &halapi.SecurityPolicyKey{
				SecurityPolicyId: policyID,
				VrfIdOrHandle:    convertVrfKeyHandle(vrfID),
			},
		},
	}
}

// convertFWRules converts netproto.PolicyRule to halapi Security Rule. // TODO handle expansion for src ports
func convertFWRules(nsp *netproto.NetworkSecurityPolicy, ruleIDToAppMapping map[uint64]*netproto.App) (rules []*halapi.SecurityRule) {
	for _, r := range nsp.Spec.Rules {
		var (
			app        *netproto.App
			protoPorts []*netproto.ProtoPort
		)

		if len(r.AppName) > 0 {
			app, _ = ruleIDToAppMapping[r.ID]
			protoPorts = app.Spec.ProtoPorts
		} else {
			protoPorts = r.Dst.ProtoPorts
		}
		action := convertAction(r, app)

		// Handle rule match for ICMP App. TODO get hal to unify this
		if app != nil && app.Spec.ALG.ICMP != nil {
			rule := &halapi.SecurityRule{
				RuleId: r.ID,
				Match:  convertRuleMatch(r.Src.Addresses, r.Dst.Addresses, nil, app),
				Action: action,
			}
			rules = append(rules, rule)
		}

		if len(protoPorts) > 0 {
			for _, p := range protoPorts {
				rule := &halapi.SecurityRule{
					RuleId: r.ID,
					Match:  convertRuleMatch(r.Src.Addresses, r.Dst.Addresses, p, app),
					Action: action,
				}
				rules = append(rules, rule)
			}
		} else {
			rule := &halapi.SecurityRule{
				RuleId: r.ID,
				Match:  convertRuleMatch(r.Src.Addresses, r.Dst.Addresses, nil, nil),
				Action: action,
			}
			rules = append(rules, rule)

		}
	}
	return
}

func convertAction(rule netproto.PolicyRule, app *netproto.App) *halapi.SecurityRuleAction {
	var action halapi.SecurityRuleAction
	if app != nil {
		action.AppData = convertALG(app.Spec)
	}
	switch strings.ToLower(rule.Action) {
	case "permit":
		action.SecAction = halapi.SecurityAction_SECURITY_RULE_ACTION_ALLOW
	case "deny":
		action.SecAction = halapi.SecurityAction_SECURITY_RULE_ACTION_DENY
	case "reject":
		action.SecAction = halapi.SecurityAction_SECURITY_RULE_ACTION_REJECT
	case "log":
		action.LogAction = halapi.LogAction_LOG_ON_SESSION_END
	}
	return &action
}

func convertRuleMatch(srcAddresses, dstAddresses []string, pp *netproto.ProtoPort, app *netproto.App) *halapi.RuleMatch {
	halSrcAddresses := convertIPAddress(srcAddresses...)
	halDstAddresses := convertIPAddress(dstAddresses...)

	// Handle ICMP here
	if app != nil && app.Spec.ALG != nil && app.Spec.ALG.ICMP != nil {
		return &halapi.RuleMatch{
			SrcAddress: halSrcAddresses,
			DstAddress: halDstAddresses,
			Protocol:   int32(halapi.IPProtocol_IPPROTO_ICMP),
			AppMatch: &halapi.RuleMatch_AppMatch{
				App: &halapi.RuleMatch_AppMatch_IcmpInfo{
					IcmpInfo: &halapi.RuleMatch_ICMPAppInfo{
						IcmpCode: app.Spec.ALG.ICMP.Code,
						IcmpType: app.Spec.ALG.ICMP.Type,
					},
				},
			},
		}
	}

	// Blanket Permit or Deny Rules
	if pp == nil && app == nil {
		return &halapi.RuleMatch{
			SrcAddress: halSrcAddresses,
			DstAddress: halDstAddresses,
		}
	}

	return &halapi.RuleMatch{
		SrcAddress: halSrcAddresses,
		DstAddress: halDstAddresses,
		Protocol:   convertProtocol(pp.Protocol),
		AppMatch:   convertPort(pp.Port),
	}
}

func convertALG(algSpec netproto.AppSpec) *halapi.AppData {
	var appData halapi.AppData

	// Convert App Idle Timeout
	if len(algSpec.AppIdleTimeout) > 0 {
		dur, _ := time.ParseDuration(algSpec.AppIdleTimeout)
		appData.IdleTimeout = uint32(dur.Seconds())
	} else {
		appData.IdleTimeout = types.DefaultTimeout
	}

	// Convert ALG Specific Information.

	// DNS
	if algSpec.ALG.DNS != nil {
		appData.Alg = halapi.ALGName_APP_SVC_DNS
		var queryRespTimeout uint32
		var maxMsgLength uint32
		if len(algSpec.ALG.DNS.QueryResponseTimeout) > 0 {
			dur, _ := time.ParseDuration(algSpec.ALG.DNS.QueryResponseTimeout)
			queryRespTimeout = uint32(dur.Seconds())
		} else {
			queryRespTimeout = types.DefaultTimeout
		}

		if algSpec.ALG.DNS.MaxMessageLength > 0 {
			maxMsgLength = algSpec.ALG.DNS.MaxMessageLength
		} else {
			maxMsgLength = types.DefaultDNSMaxMessageLength
		}

		appData.AppOptions = &halapi.AppData_DnsOptionInfo{
			DnsOptionInfo: &halapi.AppData_DNSOptions{
				DropMultiQuestionPackets:   algSpec.ALG.DNS.DropMultiQuestionPackets,
				DropLargeDomainNamePackets: algSpec.ALG.DNS.DropLargeDomainPackets,
				MaxMsgLength:               maxMsgLength,
				QueryResponseTimeout:       queryRespTimeout,
				DropLongLabelPackets:       algSpec.ALG.DNS.DropLongLabelPackets,
				DropMultizonePackets:       algSpec.ALG.DNS.DropMultiZonePackets,
			},
		}
		return &appData
	}

	// FTP
	if algSpec.ALG.FTP != nil {
		appData.Alg = halapi.ALGName_APP_SVC_FTP

		appData.AppOptions = &halapi.AppData_FtpOptionInfo{
			FtpOptionInfo: &halapi.AppData_FTPOptions{
				AllowMismatchIpAddress: algSpec.ALG.FTP.AllowMismatchIPAddresses,
			},
		}
		return &appData
	}

	// MSRPC
	if algSpec.ALG.MSRPC != nil {
		appData.Alg = halapi.ALGName_APP_SVC_MSFT_RPC
		appData.AppOptions = &halapi.AppData_MsrpcOptionInfo{
			MsrpcOptionInfo: &halapi.AppData_MSRPCOptions{
				Data: convertRPCData(algSpec.ALG.MSRPC),
			},
		}

		return &appData
	}

	// SunRPC
	if algSpec.ALG.SUNRPC != nil {
		appData.Alg = halapi.ALGName_APP_SVC_SUN_RPC

		appData.AppOptions = &halapi.AppData_SunRpcOptionInfo{
			SunRpcOptionInfo: &halapi.AppData_SunRPCOptions{
				Data: convertRPCData(algSpec.ALG.SUNRPC),
			},
		}
		return &appData
	}

	// RTSP
	if algSpec.ALG.RTSP != nil {
		appData.Alg = halapi.ALGName_APP_SVC_RTSP
		return &appData
	}

	// SIP
	if algSpec.ALG.SIP != nil {
		appData.Alg = halapi.ALGName_APP_SVC_SIP

		var mediaInactivityTimeout, maxCallDuration, cTimeout, t1Timeout, t4Timeout uint32

		if len(algSpec.ALG.SIP.MediaInactivityTimeout) > 0 {
			dur, _ := time.ParseDuration(algSpec.ALG.SIP.MediaInactivityTimeout)
			mediaInactivityTimeout = uint32(dur.Seconds())
		} else {
			mediaInactivityTimeout = types.DefaultTimeout
		}

		if len(algSpec.ALG.SIP.MaxCallDuration) > 0 {
			dur, _ := time.ParseDuration(algSpec.ALG.SIP.MaxCallDuration)
			maxCallDuration = uint32(dur.Seconds())
		} else {
			maxCallDuration = types.DefaultTimeout
		}

		if len(algSpec.ALG.SIP.T4Timeout) > 0 {
			dur, _ := time.ParseDuration(algSpec.ALG.SIP.T4Timeout)
			t4Timeout = uint32(dur.Seconds())
		} else {
			t4Timeout = types.DefaultTimeout
		}

		if len(algSpec.ALG.SIP.T1Timeout) > 0 {
			dur, _ := time.ParseDuration(algSpec.ALG.SIP.T1Timeout)
			t1Timeout = uint32(dur.Seconds())
		} else {
			t1Timeout = types.DefaultTimeout
		}

		if len(algSpec.ALG.SIP.CTimeout) > 0 {
			dur, _ := time.ParseDuration(algSpec.ALG.SIP.CTimeout)
			cTimeout = uint32(dur.Seconds())
		} else {
			cTimeout = types.DefaultTimeout
		}

		appData.AppOptions = &halapi.AppData_SipOptions{
			SipOptions: &halapi.AppData_SIPOptions{
				DscpCodePoint:          algSpec.ALG.SIP.DSCPCodePoint,
				MediaInactivityTimeout: mediaInactivityTimeout,
				MaxCallDuration:        maxCallDuration,
				T4TimerValue:           t4Timeout,
				T1TimerValue:           t1Timeout,
				Ctimeout:               cTimeout,
			},
		}
		return &appData
	}

	// TFTP
	if algSpec.ALG.TFTP != nil {
		appData.Alg = halapi.ALGName_APP_SVC_TFTP
		return &appData
	}

	return nil
}

func convertRPCData(rpcData []*netproto.RPC) []*halapi.AppData_RPCData {
	var halRPC []*halapi.AppData_RPCData
	for _, r := range rpcData {
		var h halapi.AppData_RPCData
		h.ProgramId = r.ProgramID
		if len(r.ProgramIDTimeout) > 0 {
			dur, _ := time.ParseDuration(r.ProgramIDTimeout)
			h.IdleTimeout = uint32(dur.Seconds())
		} else {
			h.IdleTimeout = types.DefaultTimeout
		}
		halRPC = append(halRPC, &h)
	}
	return halRPC
}

func convertIPAddress(addresses ...string) []*halapi.IPAddressObj {
	var halAddresses []*halapi.IPAddressObj
	for _, a := range addresses {
		kind, IPHigh, IPLow, IPMask := convertIPAddressKind(a)
		switch kind {
		case types.IPAddressAny:
			halAddresses = append(halAddresses, iristypes.HalIPAddressAny)
		case types.IPAddressSingleton, types.IPAddressCIDR:
			addr := &halapi.IPAddressObj{
				Formats: &halapi.IPAddressObj_Address{
					Address: &halapi.Address{
						Address: &halapi.Address_Prefix{
							Prefix: &halapi.IPSubnet{
								Subnet: &halapi.IPSubnet_Ipv4Subnet{
									Ipv4Subnet: &halapi.IPPrefix{
										Address:   utils.ConvertIPAddresses(IPLow.String())[0],
										PrefixLen: IPMask,
									},
								},
							},
						},
					},
				},
			}
			halAddresses = append(halAddresses, addr)
		case types.IPAddressRange:
			addr := &halapi.IPAddressObj{
				Formats: &halapi.IPAddressObj_Address{
					Address: &halapi.Address{
						Address: &halapi.Address_Range{
							Range: &halapi.AddressRange{
								Range: &halapi.AddressRange_Ipv4Range{
									Ipv4Range: &halapi.IPRange{
										LowIpaddr:  utils.ConvertIPAddresses(IPLow.String())[0],
										HighIpaddr: utils.ConvertIPAddresses(IPHigh.String())[0],
									},
								},
							},
						},
					},
				},
			}
			halAddresses = append(halAddresses, addr)
		}
	}
	return halAddresses
}

func convertProtocol(protocol string) int32 {
	switch strings.ToLower(protocol) {
	case "tcp", "6":
		return int32(halapi.IPProtocol_IPPROTO_TCP)
	case "udp", "17":
		return int32(halapi.IPProtocol_IPPROTO_UDP)
	case "icmp", "1":
		return int32(halapi.IPProtocol_IPPROTO_ICMP)
	default:
		return int32(halapi.IPProtocol_IPPROTO_NONE)
	}
}

func convertPort(port string) *halapi.RuleMatch_AppMatch {
	return &halapi.RuleMatch_AppMatch{
		App: &halapi.RuleMatch_AppMatch_PortInfo{
			PortInfo: &halapi.RuleMatch_L4PortAppInfo{
				DstPortRange: convertPortRange(port),
			},
		},
	}

}

func convertIPAddressKind(addr string) (kind types.IPAddressKind, IPHigh, IPLow net.IP, IPMask uint32) {
	if addr == "any" {
		kind = types.IPAddressAny
		return
	}

	// Singleton IP Address
	IP := net.ParseIP(strings.TrimSpace(addr))
	if len(IP) != 0 {
		kind = types.IPAddressSingleton
		IPHigh = IP
		IPLow = IP
		IPMask = 32
		return
	}

	// CIDR Based IP Address
	IP, IPNet, err := net.ParseCIDR(addr)
	if err == nil {
		mask, _ := IPNet.Mask.Size()
		kind = types.IPAddressCIDR
		IPHigh = IP
		IPLow = IP
		IPMask = uint32(mask)
		return
	}

	// Hyphen separated range
	components := strings.Split(addr, "-")
	if len(components) == 2 {
		kind = types.IPAddressRange
		IPLow = net.ParseIP(strings.TrimSpace(components[0]))
		IPHigh = net.ParseIP(strings.TrimSpace(components[1]))
		IPMask = 32
	}
	return

}

func convertPortRange(ports ...string) []*halapi.L4PortRange {
	var portRanges []*halapi.L4PortRange
	for _, p := range ports {
		var portLow, portHigh int64
		components := strings.Split(p, "-")
		if len(components) == 2 {
			portLow, _ = strconv.ParseInt(strings.TrimSpace(components[0]), 10, 32)
			portHigh, _ = strconv.ParseInt(strings.TrimSpace(components[1]), 10, 32)

		} else {
			portLow, _ = strconv.ParseInt(p, 10, 32)
			portHigh = portLow
		}
		pRange := &halapi.L4PortRange{
			PortLow:  uint32(portLow),
			PortHigh: uint32(portHigh),
		}
		portRanges = append(portRanges, pRange)
	}
	return portRanges
}
