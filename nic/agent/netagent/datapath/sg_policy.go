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
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateSGPolicy creates a security group policy in the datapath
func (hd *Datapath) CreateSGPolicy(sgp *netproto.SGPolicy, vrfID uint64, sgs []*netproto.SecurityGroup, ruleIDAppLUT *sync.Map) error {
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

	for _, r := range sgp.Spec.Rules {
		ruleMatches, err := hd.buildHALRuleMatches(r.Src, r.Dst, ruleIDAppLUT, &r.ID)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}
		for _, match := range ruleMatches {
			ruleAction, err := convertRuleAction(r.ID, ruleIDAppLUT, r.Action)
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
							SecurityPolicyId: sgp.Status.SGPolicyID,
							VrfIdOrHandle:    vrfKey,
						},
					},
				},
				Rule: fwRules,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Sgclient.SecurityPolicyCreate(context.Background(), sgPolicyReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
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

// UpdateSGPolicy updates a security group policy in the datapath
func (hd *Datapath) UpdateSGPolicy(sgp *netproto.SGPolicy, vrfID uint64, ruleIDAppLUT *sync.Map) error {
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

	for _, r := range sgp.Spec.Rules {
		ruleMatches, err := hd.buildHALRuleMatches(r.Src, r.Dst, ruleIDAppLUT, &r.ID)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}

		for _, match := range ruleMatches {
			ruleAction, err := convertRuleAction(r.ID, ruleIDAppLUT, r.Action)
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
							SecurityPolicyId: sgp.Status.SGPolicyID,
							VrfIdOrHandle:    vrfKey,
						},
					},
				},
				Rule: fwRules,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Sgclient.SecurityPolicyUpdate(context.Background(), sgPolicyUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating SGPolicy. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Sgclient.SecurityPolicyUpdate(context.Background(), sgPolicyUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating SGPolicy. Err: %v", err)
			return err
		}
	}

	hd.DB.SgPolicyDB[objectKey(&sgp.ObjectMeta)] = sgPolicyUpdateReqMsg

	return nil
}

// DeleteSGPolicy deletes a security group policy in the datapath
func (hd *Datapath) DeleteSGPolicy(sgp *netproto.SGPolicy, vrfID uint64) error {
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
							SecurityPolicyId: sgp.Status.SGPolicyID,
							VrfIdOrHandle:    vrfKey,
						},
					},
				},
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Sgclient.SecurityPolicyDelete(context.Background(), sgPolicyDelReq)
		if err != nil {
			log.Errorf("Error deleting security policy. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
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

func convertRuleAction(ruleID uint64, ruleIDAppLUT *sync.Map, action string) (*halproto.SecurityRuleAction, error) {
	var ruleAction halproto.SecurityRuleAction
	var appData []*halproto.AppData

	obj, ok := ruleIDAppLUT.Load(ruleID)
	// Rule has a corresponding ALG information
	if ok {
		apps, ok := obj.([]*netproto.App)
		if !ok {
			log.Errorf("failed to cast App object. %v", obj)
			return nil, fmt.Errorf("failed to cast App object. %v", obj)
		}
		for _, app := range apps {
			alg, err := convertALG(app)
			if err != nil {
				log.Errorf("failed to convert alg data. Err: %v, App: %v", err, app)
			}
			appData = append(appData, alg)
		}

		ruleAction.AppData = appData
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
		if len(algSpec.DNS.QueryResponseTimeout) > 0 {
			dur, err := time.ParseDuration(algSpec.DNS.QueryResponseTimeout)
			if err != nil {
				return nil, fmt.Errorf("invalid time duration %s", algSpec.DNS.QueryResponseTimeout)
			}
			queryRespTimeout = uint32(dur.Seconds())
		} else {
			queryRespTimeout = constants.DefaultTimeout
		}

		appData.AppOptions = &halproto.AppData_DnsOptionInfo{
			DnsOptionInfo: &halproto.AppData_DNSOptions{
				DropMultiQuestionPackets:   algSpec.DNS.DropMultiQuestionPackets,
				DropLargeDomainNamePackets: algSpec.DNS.DropLargeDomainPackets,
				MaxMsgLength:               algSpec.DNS.MaxMessageLength,
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
		/*
				TBD: this needs to be coordinated with an array of (program-id, timeout) tuples
			appData.AppOptions = &halproto.AppData_MsrpcOptionInfo{
				MsrpcOptionInfo: &halproto.AppData_MSRPCOptions{
					Uuid: algSpec.MSRPC.ProgramID,
				},
			}
		*/
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

	// SunRPC
	if algSpec.SUNRPC != nil {
		appData.Alg = halproto.ALGName_APP_SVC_SUN_RPC
		/*
			TBD: this needs to be coordinated with an array of (program-id, timeout) tuples
			appData.AppOptions = &halproto.AppData_SunRpcOptionInfo{
				SunRpcOptionInfo: &halproto.AppData_SunRPCOptions{
					ProgramNumber: algSpec.SUNRPC.ProgramID,
				},
			}
		*/
		return &appData, nil
	}

	// TFTP
	if algSpec.TFTP != nil {
		appData.Alg = halproto.ALGName_APP_SVC_TFTP
		return &appData, nil
	}

	return nil, nil
}

// ------------------------ test utility functions -------------

// FindSGPolicy finds sg policy object in datapath
func (hd *Datapath) FindSGPolicy(meta api.ObjectMeta) (*halproto.SecurityPolicyRequestMsg, error) {
	hd.Lock()
	sgp, ok := hd.DB.SgPolicyDB[objectKey(&meta)]
	hd.Unlock()
	if !ok {
		return nil, errors.New("sgpolicy not found")
	}

	return sgp, nil
}
