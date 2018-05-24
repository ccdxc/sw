package datapath

import (
	"context"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateSGPolicy creates a security group policy in the datapath
func (hd *Datapath) CreateSGPolicy(sgp *netproto.SGPolicy, vrfID uint64, sgs []uint64) error {
	var fwRules []*halproto.SecurityRule
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}

	for _, r := range sgp.Spec.Rules {
		ruleMatch, err := hd.convertMatchCriteria(r.Src, r.Dst)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}

		rule := &halproto.SecurityRule{
			RuleId: r.ID,
			Match:  ruleMatch,
			Action: convertRuleAction(r.Action),
		}

		fwRules = append(fwRules, rule)
	}
	sgPolicyReqMsg := &halproto.SecurityPolicyRequestMsg{
		Request: []*halproto.SecurityPolicySpec{
			{
				PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle{
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
func (hd *Datapath) UpdateSGPolicy(np *netproto.SGPolicy, ns *netproto.Namespace) error {
	return nil
}

// DeleteSGPolicy deletes a security group policy in the datapath
func (hd *Datapath) DeleteSGPolicy(np *netproto.SGPolicy, ns *netproto.Namespace) error {
	return nil
}

func convertRuleAction(actions []string) *halproto.SecurityRuleAction {
	var ruleAction halproto.SecurityRuleAction
	for _, a := range actions {
		switch a {
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
			log.Errorf("invalid policy action %v specified.", a)
		}
	}
	return &ruleAction
}
