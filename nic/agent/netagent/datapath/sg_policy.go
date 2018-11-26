package datapath

import (
	"context"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateSGPolicy creates a security group policy in the datapath
func (hd *Datapath) CreateSGPolicy(sgp *netproto.SGPolicy, vrfID uint64, sgs []*netproto.SecurityGroup) error {
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
		ruleMatches, err := hd.convertMatchCriteria(r.Src, r.Dst)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}
		for _, match := range ruleMatches {
			rule := &halproto.SecurityRule{
				RuleId: r.ID,
				Match:  match,
				Action: convertRuleAction(r.Action),
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
	return nil
}

// UpdateSGPolicy updates a security group policy in the datapath
func (hd *Datapath) UpdateSGPolicy(sgp *netproto.SGPolicy, vrfID uint64) error {
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
		ruleMatches, err := hd.convertMatchCriteria(r.Src, r.Dst)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}

		for _, match := range ruleMatches {
			rule := &halproto.SecurityRule{
				RuleId: r.ID,
				Match:  match,
				Action: convertRuleAction(r.Action),
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
	return nil
}

func convertRuleAction(action string) *halproto.SecurityRuleAction {
	var ruleAction halproto.SecurityRuleAction
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
	return &ruleAction
}
