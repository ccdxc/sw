package datapath

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/constants"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateNetworkSecurityPolicy creates a security group policy in the datapath
func (hd *Datapath) CreateNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy, vrfID uint64, ruleIDAppLUT *sync.Map) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var fwRules []*halproto.SecurityRule
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}

	for idx, r := range sgp.Spec.Rules {
		ruleMatches, err := hd.buildHALRuleMatches(r.Src, r.Dst, ruleIDAppLUT, &idx)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}
		for _, match := range ruleMatches {
			ruleAction, err := convertRuleAction(idx, ruleIDAppLUT, r.Action)
			if err != nil {
				log.Errorf("Failed to convert rule action. Err: %v", err)
				return err
			}
			rule := &halproto.SecurityRule{
				RuleId: r.ID,
				Match:  match,
				Action: ruleAction,
			}
			fwRules = append(fwRules, rule)
		}

	}

	sgPolicyReqMsg := &halproto.SecurityPolicyRequestMsg{
		Request: []*halproto.SecurityPolicySpec{
			{
				KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
					PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
						SecurityPolicyKey: &halproto.SecurityPolicyKey{
							SecurityPolicyId: sgp.Status.NetworkSecurityPolicyID,
							VrfIdOrHandle:    vrfKey,
						},
					},
				},
				Rule: fwRules,
			},
		},
	}

	log.Infof("Sending NetworkSecurityPolicy Create to datapath: %+v", sgPolicyReqMsg.Request[0].KeyOrHandle)

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Sgclient.SecurityPolicyCreate(context.Background(), sgPolicyReqMsg)
		if err != nil {
			log.Errorf("Error creating security policy. Err: %v", err)
			return err
		}

		// Check if HAL already has the policy and make it idempotent
		if resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_ERR {
			sgpReq := &halproto.SecurityPolicyGetRequestMsg{
				Request: []*halproto.SecurityPolicyGetRequest{
					{
						KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
							PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
								SecurityPolicyKey: &halproto.SecurityPolicyKey{
									SecurityPolicyId: sgp.Status.NetworkSecurityPolicyID,
									VrfIdOrHandle:    vrfKey,
								},
							},
						},
					},
				},
			}
			getResp, _ := hd.Hal.Sgclient.SecurityPolicyGet(context.Background(), sgpReq)
			if getResp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK {
				log.Info("Network security policy exists in HAL")
				return nil
			}
		}

		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Sgclient.SecurityPolicyCreate(context.Background(), sgPolicyReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return err
		}
	}

	hd.DB.SgPolicyDB[objectKey(&sgp.ObjectMeta)] = sgPolicyReqMsg

	return nil
}

// UpdateNetworkSecurityPolicy updates a security group policy in the datapath
func (hd *Datapath) UpdateNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy, vrfID uint64, ruleIDAppLUT *sync.Map) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var fwRules []*halproto.SecurityRule
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}

	for idx, r := range sgp.Spec.Rules {
		ruleMatches, err := hd.buildHALRuleMatches(r.Src, r.Dst, ruleIDAppLUT, &idx)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}

		for _, match := range ruleMatches {
			ruleAction, err := convertRuleAction(idx, ruleIDAppLUT, r.Action)
			if err != nil {
				log.Errorf("Failed to convert rule action. Err: %v", err)
				return err
			}
			rule := &halproto.SecurityRule{
				RuleId: r.ID,
				Match:  match,
				Action: ruleAction,
			}
			fwRules = append(fwRules, rule)

		}
	}
	sgPolicyUpdateReqMsg := &halproto.SecurityPolicyRequestMsg{
		Request: []*halproto.SecurityPolicySpec{
			{
				KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
					PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
						SecurityPolicyKey: &halproto.SecurityPolicyKey{
							SecurityPolicyId: sgp.Status.NetworkSecurityPolicyID,
							VrfIdOrHandle:    vrfKey,
						},
					},
				},
				Rule: fwRules,
			},
		},
	}
	log.Infof("Sending NetworkSecurityPolicy Update to datapath: %+v", sgPolicyUpdateReqMsg.Request[0].KeyOrHandle)

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Sgclient.SecurityPolicyUpdate(context.Background(), sgPolicyUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating NetworkSecurityPolicy. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Sgclient.SecurityPolicyUpdate(context.Background(), sgPolicyUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating NetworkSecurityPolicy. Err: %v", err)
			return err
		}
	}

	hd.DB.SgPolicyDB[objectKey(&sgp.ObjectMeta)] = sgPolicyUpdateReqMsg

	return nil
}

// DeleteNetworkSecurityPolicy deletes a security group policy in the datapath
func (hd *Datapath) DeleteNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy, vrfID uint64) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}

	sgPolicyDelReq := &halproto.SecurityPolicyDeleteRequestMsg{
		Request: []*halproto.SecurityPolicyDeleteRequest{
			{
				KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
					PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
						SecurityPolicyKey: &halproto.SecurityPolicyKey{
							SecurityPolicyId: sgp.Status.NetworkSecurityPolicyID,
							VrfIdOrHandle:    vrfKey,
						},
					},
				},
			},
		},
	}

	log.Infof("Sending NetworkSecurityPolicy Delete to datapath: %+v", sgPolicyDelReq.Request[0].KeyOrHandle)

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Sgclient.SecurityPolicyDelete(context.Background(), sgPolicyDelReq)
		if err != nil {
			log.Errorf("Error deleting security policy. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Sgclient.SecurityPolicyDelete(context.Background(), sgPolicyDelReq)
		if err != nil {
			log.Errorf("Error deleting security policy. Err: %v", err)
			return err
		}
	}

	delete(hd.DB.SgPolicyDB, objectKey(&sgp.ObjectMeta))

	return nil
}

func convertRuleAction(ruleIdx int, ruleIDAppLUT *sync.Map, action string) (*halproto.SecurityRuleAction, error) {
	var ruleAction halproto.SecurityRuleAction

	obj, ok := ruleIDAppLUT.Load(ruleIdx)
	// Rule has a corresponding ALG information
	if ok {
		app, ok := obj.(*netproto.App)
		if !ok {
			log.Errorf("failed to cast App object. %v", obj)
			return nil, fmt.Errorf("failed to cast App object. %v", obj)
		}
		alg, err := convertALG(app)
		if err != nil {
			log.Errorf("failed to convert alg data. Err: %v, App: %v", err, app)
		}
		ruleAction.AppData = alg
	}
	switch action {
	case "PERMIT":
		ruleAction.SecAction = halproto.SecurityAction_SECURITY_RULE_ACTION_ALLOW
	case "DENY":
		ruleAction.SecAction = halproto.SecurityAction_SECURITY_RULE_ACTION_DENY
	case "REJECT":
		ruleAction.SecAction = halproto.SecurityAction_SECURITY_RULE_ACTION_REJECT
	case "LOG":
		ruleAction.LogAction = halproto.LogAction_LOG_ON_SESSION_END
	default:
		ruleAction.SecAction = halproto.SecurityAction_SECURITY_RULE_ACTION_NONE
		ruleAction.LogAction = halproto.LogAction_LOG_NONE
		log.Errorf("invalid policy action %v specified.", action)
	}
	return &ruleAction, nil
}

func convertALG(app *netproto.App) (*halproto.AppData, error) {
	var appData halproto.AppData
	algSpec := app.Spec.ALG
	// Convert App Idle Timeout
	if len(app.Spec.AppIdleTimeout) > 0 {
		dur, err := time.ParseDuration(app.Spec.AppIdleTimeout)
		if err != nil {
			return nil, fmt.Errorf("invalid time duration %s", app.Spec.AppIdleTimeout)
		}
		appData.IdleTimeout = uint32(dur.Seconds())
	} else {
		appData.IdleTimeout = constants.DefaultTimeout
	}

	// Convert ALG Specific Information.

	// DNS
	if algSpec.DNS != nil {
		appData.Alg = halproto.ALGName_APP_SVC_DNS
		var queryRespTimeout uint32
		var maxMsgLength uint32
		if len(algSpec.DNS.QueryResponseTimeout) > 0 {
			dur, err := time.ParseDuration(algSpec.DNS.QueryResponseTimeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.DNS.QueryResponseTimeout)
			}
			queryRespTimeout = uint32(dur.Seconds())
		} else {
			queryRespTimeout = constants.DefaultTimeout
		}

		if algSpec.DNS.MaxMessageLength > 0 {
			maxMsgLength = algSpec.DNS.MaxMessageLength
		} else {
			maxMsgLength = constants.DefaultDNSMaxMessageLength
		}

		appData.AppOptions = &halproto.AppData_DnsOptionInfo{
			DnsOptionInfo: &halproto.AppData_DNSOptions{
				DropMultiQuestionPackets:   algSpec.DNS.DropMultiQuestionPackets,
				DropLargeDomainNamePackets: algSpec.DNS.DropLargeDomainPackets,
				MaxMsgLength:               maxMsgLength,
				QueryResponseTimeout:       queryRespTimeout,
				DropLongLabelPackets:       algSpec.DNS.DropLongLabelPackets,
				DropMultizonePackets:       algSpec.DNS.DropMultiZonePackets,
			},
		}
		return &appData, nil
	}

	// FTP
	if algSpec.FTP != nil {
		appData.Alg = halproto.ALGName_APP_SVC_FTP

		appData.AppOptions = &halproto.AppData_FtpOptionInfo{
			FtpOptionInfo: &halproto.AppData_FTPOptions{
				AllowMismatchIpAddress: algSpec.FTP.AllowMismatchIPAddresses,
			},
		}
		return &appData, nil
	}

	// MSRPC
	if algSpec.MSRPC != nil {
		appData.Alg = halproto.ALGName_APP_SVC_MSFT_RPC
		appData.AppOptions = &halproto.AppData_MsrpcOptionInfo{
			MsrpcOptionInfo: &halproto.AppData_MSRPCOptions{
				Data: convertRPCData(algSpec.MSRPC),
			},
		}

		return &appData, nil
	}

	// SunRPC
	if algSpec.SUNRPC != nil {
		appData.Alg = halproto.ALGName_APP_SVC_SUN_RPC

		appData.AppOptions = &halproto.AppData_SunRpcOptionInfo{
			SunRpcOptionInfo: &halproto.AppData_SunRPCOptions{
				Data: convertRPCData(algSpec.SUNRPC),
			},
		}
		return &appData, nil
	}

	// RTSP
	if algSpec.RTSP != nil {
		appData.Alg = halproto.ALGName_APP_SVC_RTSP
		return &appData, nil
	}

	// SIP
	if algSpec.SIP != nil {
		appData.Alg = halproto.ALGName_APP_SVC_SIP

		var mediaInactivityTimeout, maxCallDuration, cTimeout, t1Timeout, t4Timeout uint32

		if len(algSpec.SIP.MediaInactivityTimeout) > 0 {
			dur, err := time.ParseDuration(algSpec.SIP.MediaInactivityTimeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.SIP.MediaInactivityTimeout)
			}
			mediaInactivityTimeout = uint32(dur.Seconds())
		} else {
			mediaInactivityTimeout = constants.DefaultTimeout
		}

		if len(algSpec.SIP.MaxCallDuration) > 0 {
			dur, err := time.ParseDuration(algSpec.SIP.MaxCallDuration)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.SIP.MaxCallDuration)
			}
			maxCallDuration = uint32(dur.Seconds())
		} else {
			maxCallDuration = constants.DefaultTimeout
		}

		if len(algSpec.SIP.T4Timeout) > 0 {
			dur, err := time.ParseDuration(algSpec.SIP.T4Timeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.SIP.T4Timeout)
			}
			t4Timeout = uint32(dur.Seconds())
		} else {
			t4Timeout = constants.DefaultTimeout
		}

		if len(algSpec.SIP.T1Timeout) > 0 {
			dur, err := time.ParseDuration(algSpec.SIP.T1Timeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.SIP.T1Timeout)
			}
			t1Timeout = uint32(dur.Seconds())
		} else {
			t1Timeout = constants.DefaultTimeout
		}

		if len(algSpec.SIP.CTimeout) > 0 {
			dur, err := time.ParseDuration(algSpec.SIP.CTimeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.SIP.CTimeout)
			}
			cTimeout = uint32(dur.Seconds())
		} else {
			cTimeout = constants.DefaultTimeout
		}

		appData.AppOptions = &halproto.AppData_SipOptions{
			SipOptions: &halproto.AppData_SIPOptions{
				DscpCodePoint:          algSpec.SIP.DSCPCodePoint,
				MediaInactivityTimeout: mediaInactivityTimeout,
				MaxCallDuration:        maxCallDuration,
				T4TimerValue:           t4Timeout,
				T1TimerValue:           t1Timeout,
				Ctimeout:               cTimeout,
			},
		}
		return &appData, nil
	}

	// TFTP
	if algSpec.TFTP != nil {
		appData.Alg = halproto.ALGName_APP_SVC_TFTP
		return &appData, nil
	}

	return nil, nil
}

func convertRPCData(rpcData []*netproto.RPC) []*halproto.AppData_RPCData {
	var halRPC []*halproto.AppData_RPCData
	for _, r := range rpcData {
		var h halproto.AppData_RPCData
		h.ProgramId = r.ProgramID
		if len(r.ProgramIDTimeout) > 0 {
			dur, err := time.ParseDuration(r.ProgramIDTimeout)
			if err != nil {
				log.Errorf("invalid time duration %s", r.ProgramIDTimeout)
			}
			h.IdleTimeout = uint32(dur.Seconds())
		} else {
			h.IdleTimeout = constants.DefaultTimeout
		}
		halRPC = append(halRPC, &h)
	}
	return halRPC
}

// ------------------------ test utility functions -------------

// FindNetworkSecurityPolicy finds sg policy object in datapath
func (hd *Datapath) FindNetworkSecurityPolicy(meta api.ObjectMeta) (*halproto.SecurityPolicyRequestMsg, error) {
	hd.Lock()
	sgp, ok := hd.DB.SgPolicyDB[objectKey(&meta)]
	hd.Unlock()
	if !ok {
		return nil, errors.New("sgpolicy not found")
	}

	return sgp, nil
}
