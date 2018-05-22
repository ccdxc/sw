package datapath

import (
	"context"
	"errors"
	"strings"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateSecurityGroup creates a security group
func (hd *Datapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	var secGroupPolicyRequests []*halproto.SecurityGroupPolicySpec

	// convert the rules
	for _, rl := range sg.Spec.Rules {
		policySpec, err := hd.convertRule(sg, &rl)
		if err != nil {
			return err
		}
		secGroupPolicyRequests = append(secGroupPolicyRequests, policySpec)
	}

	sgPolicyRequestMsg := halproto.SecurityGroupPolicyRequestMsg{
		Request: secGroupPolicyRequests,
	}

	// add security group policies
	_, err := hd.Hal.Sgclient.SecurityGroupPolicyCreate(context.Background(), &sgPolicyRequestMsg)
	if err != nil {
		log.Errorf("Could not create SG Policy. %v", err)
		return err
	}

	hd.Lock()
	hd.DB.SgPolicyDB[objectKey(&sg.ObjectMeta)] = &sgPolicyRequestMsg
	hd.Unlock()

	// build security group message
	sgs := halproto.SecurityGroupSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
	}
	sgmsg := halproto.SecurityGroupRequestMsg{
		Request: []*halproto.SecurityGroupSpec{&sgs},
	}

	// add security group
	_, err = hd.Hal.Sgclient.SecurityGroupCreate(context.Background(), &sgmsg)
	if err != nil {
		log.Errorf("Error creating security group. Err: %v", err)
		return err
	}

	// save the sg message
	hd.Lock()
	hd.DB.SgDB[objectKey(&sg.ObjectMeta)] = &sgmsg
	hd.Unlock()

	return nil
}

// UpdateSecurityGroup updates a security group
func (hd *Datapath) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	var secGroupPolicyRequests []*halproto.SecurityGroupPolicySpec
	// convert the rules
	for _, rl := range sg.Spec.Rules {
		policySpec, err := hd.convertRule(sg, &rl)
		if err != nil {
			return err
		}
		secGroupPolicyRequests = append(secGroupPolicyRequests, policySpec)
	}

	sgPolicyRequestMsg := halproto.SecurityGroupPolicyRequestMsg{
		Request: secGroupPolicyRequests,
	}
	// add security group policies
	_, err := hd.Hal.Sgclient.SecurityGroupPolicyUpdate(context.Background(), &sgPolicyRequestMsg)
	if err != nil {
		log.Errorf("Could not create SG Policy. %v", err)
		return err
	}

	hd.Lock()
	hd.DB.SgPolicyDB[objectKey(&sg.ObjectMeta)] = &sgPolicyRequestMsg
	hd.Unlock()

	// build security group message
	sgs := halproto.SecurityGroupSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
	}
	sgmsg := halproto.SecurityGroupRequestMsg{
		Request: []*halproto.SecurityGroupSpec{&sgs},
	}

	// update security group
	_, err = hd.Hal.Sgclient.SecurityGroupUpdate(context.Background(), &sgmsg)
	if err != nil {
		log.Errorf("Error updating security group. Err: %v", err)
		return err
	}

	// save the sg message
	hd.Lock()
	hd.DB.SgDB[objectKey(&sg.ObjectMeta)] = &sgmsg
	hd.Unlock()

	return nil
}

// DeleteSecurityGroup deletes a security group
func (hd *Datapath) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
	// build security group message
	sgdel := halproto.SecurityGroupDeleteRequest{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
	}
	sgmsg := halproto.SecurityGroupDeleteRequestMsg{
		Request: []*halproto.SecurityGroupDeleteRequest{&sgdel},
	}

	// delete security group
	_, err := hd.Hal.Sgclient.SecurityGroupDelete(context.Background(), &sgmsg)
	if err != nil {
		log.Errorf("Error deleting security group. Err: %v", err)
		return err
	}

	// delete the sg message
	hd.Lock()
	delete(hd.DB.SgDB, objectKey(&sg.ObjectMeta))
	hd.Unlock()

	return nil
}

func (hd *Datapath) convertRule(sg *netproto.SecurityGroup, rule *netproto.SecurityRule) (*halproto.SecurityGroupPolicySpec, error) {
	//var policyRules *halproto.SGPolicy
	// convert the action
	act := halproto.FirewallAction_FIREWALL_ACTION_NONE
	switch rule.Action {
	case "Allow":
		act = halproto.FirewallAction_FIREWALL_ACTION_ALLOW
	case "Deny":
		act = halproto.FirewallAction_FIREWALL_ACTION_DENY
	case "Reject":
		act = halproto.FirewallAction_FIREWALL_ACTION_REJECT
	default:
		log.Errorf("Unknown action %s in rule {%+v}", rule.Action, rule)
		return nil, errors.New("Unknown action")
	}

	// build service list
	srvs := []*halproto.Service{}
	for _, svc := range rule.Services {
		proto := halproto.IPProtocol_IPPROTO_NONE
		switch svc.Protocol {
		case "tcp":
			proto = halproto.IPProtocol_IPPROTO_TCP
		case "udp":
			proto = halproto.IPProtocol_IPPROTO_UDP
		case "icmp":
			proto = halproto.IPProtocol_IPPROTO_ICMP
		default:
			log.Errorf("Unknown protocol %s in rule {%+v}", svc.Protocol, rule)
			return nil, errors.New("Unknown protocol")
		}

		sr := halproto.Service{
			IpProtocol: proto,
			L4Info: &halproto.Service_DstPort{
				DstPort: svc.Port,
			},
		}

		srvs = append(srvs, &sr)
	}

	if strings.ToLower(rule.Direction) == "incoming" {
		var policyRules = halproto.SGPolicy{
			InFwRules: []*halproto.FirewallRuleSpec{
				{
					Svc:    srvs,
					Action: act,
					Log:    rule.Log,
				},
			},
		}
		sgPolicy := halproto.SecurityGroupPolicySpec{
			Meta: &halproto.ObjectMeta{},
			KeyOrHandle: &halproto.SecurityGroupPolicyKeyHandle{
				PolicyKeyOrHandle: &halproto.SecurityGroupPolicyKeyHandle_SecurityGroupPolicyId{
					SecurityGroupPolicyId: &halproto.SecurityGroupPolicyId{
						SecurityGroupId:     sg.Status.SecurityGroupID,
						PeerSecurityGroupId: rule.PeerGroupID,
					},
				},
			},
			PolicyRules: &policyRules,
		}
		return &sgPolicy, nil

	}
	var policyRules = halproto.SGPolicy{
		EgFwRules: []*halproto.FirewallRuleSpec{
			{
				Svc:    srvs,
				Action: act,
				Log:    rule.Log,
			},
		},
	}
	sgPolicy := halproto.SecurityGroupPolicySpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.SecurityGroupPolicyKeyHandle{
			PolicyKeyOrHandle: &halproto.SecurityGroupPolicyKeyHandle_SecurityGroupPolicyId{
				SecurityGroupPolicyId: &halproto.SecurityGroupPolicyId{
					SecurityGroupId:     sg.Status.SecurityGroupID,
					PeerSecurityGroupId: rule.PeerGroupID,
				},
			},
		},
		PolicyRules: &policyRules,
	}
	return &sgPolicy, nil
}
