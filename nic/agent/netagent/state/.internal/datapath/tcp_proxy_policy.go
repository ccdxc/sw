// +build ignore

package datapath

import (
	"context"
	"fmt"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ----------------------- TCPProxy Policy CRUDs ----------------------- //

// CreateTCPProxyPolicy creates an TCPProxy Policy in the datapath
func (hd *Datapath) CreateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	var tcpProxyRules []*halproto.TcpProxyRuleMatchSpec

	for _, r := range tcp.Spec.Rules {
		// Match source and dest attributes
		ruleMatches, err := hd.buildHALRuleMatches(r.Src, r.Dst, nil, nil)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}

		for _, match := range ruleMatches {
			rule := &halproto.TcpProxyRuleMatchSpec{
				RuleId:         r.ID,
				Match:          match,
				TcpProxyAction: convertProxyAction(r.Action),
			}
			tcpProxyRules = append(tcpProxyRules, rule)
		}

	}

	tcpProxyReqMsg := &halproto.TcpProxyRuleRequestMsg{
		Request: []*halproto.TcpProxyRuleSpec{
			{
				KeyOrHandle: &halproto.TcpProxyRuleKeyHandle{
					KeyOrHandle: &halproto.TcpProxyRuleKeyHandle_RuleKey{
						RuleKey: &halproto.TcpProxyRuleKey{
							TcpProxyRuleId: tcp.Status.TCPProxyPolicyID,
							VrfKeyOrHandle: vrfKey,
						},
					},
				},
				VrfKeyHandle: vrfKey,
				Rules:        tcpProxyRules,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.TCPProxyPolicyClient.TcpProxyRuleCreate(context.Background(), tcpProxyReqMsg)
		if err != nil {
			log.Errorf("Error creating TCPProxy Policy. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.TCPProxyPolicyClient.TcpProxyRuleCreate(context.Background(), tcpProxyReqMsg)
		if err != nil {
			log.Errorf("Error creating TCPProxy Policy. Err: %v", err)
			return err
		}
	}
	return nil
}

// UpdateTCPProxyPolicy updates an TCPProxy Policy in the datapath
func (hd *Datapath) UpdateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	var tcpProxyRules []*halproto.TcpProxyRuleMatchSpec

	for _, r := range tcp.Spec.Rules {
		// Match source and dest attributes
		ruleMatches, err := hd.buildHALRuleMatches(r.Src, r.Dst, nil, nil)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}

		for _, match := range ruleMatches {
			rule := &halproto.TcpProxyRuleMatchSpec{
				RuleId:         r.ID,
				Match:          match,
				TcpProxyAction: convertProxyAction(r.Action),
			}
			tcpProxyRules = append(tcpProxyRules, rule)
		}

	}

	tcpProxyUpdateReqMsg := &halproto.TcpProxyRuleRequestMsg{
		Request: []*halproto.TcpProxyRuleSpec{
			{
				KeyOrHandle: &halproto.TcpProxyRuleKeyHandle{
					KeyOrHandle: &halproto.TcpProxyRuleKeyHandle_RuleKey{
						RuleKey: &halproto.TcpProxyRuleKey{
							TcpProxyRuleId: tcp.Status.TCPProxyPolicyID,
							VrfKeyOrHandle: vrfKey,
						},
					},
				},
				VrfKeyHandle: vrfKey,
				Rules:        tcpProxyRules,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.TCPProxyPolicyClient.TcpProxyRuleUpdate(context.Background(), tcpProxyUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating TCPProxy Policy. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.TCPProxyPolicyClient.TcpProxyRuleUpdate(context.Background(), tcpProxyUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating  TCPProxy Policy. Err: %v", err)
			return err
		}
	}
	return nil
}

// DeleteTCPProxyPolicy deletes an TCPProxy Policy in the datapath
func (hd *Datapath) DeleteTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	tcpProxyPolicyDelReq := &halproto.TcpProxyRuleDeleteRequestMsg{
		Request: []*halproto.TcpProxyRuleDeleteRequest{
			{
				KeyOrHandle: &halproto.TcpProxyRuleKeyHandle{
					KeyOrHandle: &halproto.TcpProxyRuleKeyHandle_RuleKey{
						RuleKey: &halproto.TcpProxyRuleKey{
							TcpProxyRuleId: tcp.Status.TCPProxyPolicyID,
							VrfKeyOrHandle: vrfKey,
						},
					},
				},
			},
		},
	}
	if hd.Kind == "hal" {
		resp, err := hd.Hal.TCPProxyPolicyClient.TcpProxyRuleDelete(context.Background(), tcpProxyPolicyDelReq)
		if err != nil {
			log.Errorf("Error deleting TCPProxy Policy. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.TCPProxyPolicyClient.TcpProxyRuleDelete(context.Background(), tcpProxyPolicyDelReq)
		if err != nil {
			log.Errorf("Error deleting TCPProxy Policy. Err: %v", err)
			return err
		}
	}
	return nil
}

func convertProxyAction(action string) *halproto.TcpProxyAction {
	var proxyAction halproto.TcpProxyAction
	switch action {
	case "ENABLE":
		proxyAction.TcpProxyActionType = halproto.TcpProxyActionType_TCP_PROXY_ACTION_TYPE_ENABLE
	case "DISABLE":
		proxyAction.TcpProxyActionType = halproto.TcpProxyActionType_TCP_PROXY_ACTION_TYPE_DISABLE
	default:
		proxyAction.TcpProxyActionType = halproto.TcpProxyActionType_TCP_PROXY_ACTION_TYPE_NONE
		log.Errorf("invalid proxy policy action %v specified.", action)
	}
	return &proxyAction
}
