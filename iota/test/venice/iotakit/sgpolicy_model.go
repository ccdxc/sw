// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/utils/log"
)

// SGPolicy represents security policy
type SGPolicy struct {
	venicePolicy *security.SGPolicy
	sm           *SysModel // pointer back to the model
}

// SGPolicyCollection is list of policies
type SGPolicyCollection struct {
	err      error
	policies []*SGPolicy
}

type sgRuleCtx struct {
	rule   *security.SGRule
	policy *SGPolicy
}

// SGRuleCollection is list of rules
type SGRuleCollection struct {
	err   error
	rules []*sgRuleCtx
}

// NewSGPolicy creates a new SG policy
func (sm *SysModel) NewSGPolicy(name string) *SGPolicyCollection {
	return &SGPolicyCollection{
		policies: []*SGPolicy{
			{
				venicePolicy: &security.SGPolicy{
					TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
					ObjectMeta: api.ObjectMeta{
						Tenant:    "default",
						Namespace: "default",
						Name:      name,
					},
					Spec: security.SGPolicySpec{
						AttachTenant: true,
					},
				},
				sm: sm,
			},
		},
	}
}

// SGPolicy finds an SG policy by name
func (sm *SysModel) SGPolicy(name string) *SGPolicyCollection {
	pol, ok := sm.sgpolicies[name]
	if !ok {
		return &SGPolicyCollection{
			err: fmt.Errorf("No policy found for %v", name),
		}
	}

	return &SGPolicyCollection{
		policies: []*SGPolicy{pol},
	}
}

// AddRule adds a rule to the policy
func (spc *SGPolicyCollection) AddRule(fromIP, toIP, port, action string) *SGPolicyCollection {
	if spc.err != nil {
		return spc
	}

	var rproto, rport string
	// some basic checks
	switch action {
	case "PERMIT":
	case "DENY":
	case "REJECT":
	default:
		spc.err = fmt.Errorf("Invalid rule action: %s", action)
		return spc
	}

	var protoPort []security.ProtoPort
	// parse port in `<proto>/<port>` format. E.g: tcp/80
	if port != "" {
		params := strings.Split(port, "/")
		if len(params) == 1 {
			pstr := strings.ToLower(params[0])
			if pstr == "tcp" || pstr == "udp" || pstr == "icmp" {
				rproto = pstr
			} else {
				_, err := strconv.Atoi(pstr)
				if err != nil {
					spc.err = fmt.Errorf("Invalid port number: %v", port)
					return spc
				}
				rproto = "tcp"
				rport = pstr
			}
		} else if len(params) == 2 {
			protostr := strings.ToLower(params[0])
			if protostr == "tcp" || protostr == "udp" || protostr == "icmp" {
				rproto = protostr
			} else {
				spc.err = fmt.Errorf("Invalid protocol: %v", port)
				return spc
			}
			_, err := strconv.Atoi(params[1])
			if err != nil {
				spc.err = fmt.Errorf("Invalid port number: %v", port)
				return spc
			}
			rport = params[1]
		} else {
			spc.err = fmt.Errorf("Invalid port string: %v", port)
			return spc
		}

		protoPort = append(protoPort, security.ProtoPort{
			Protocol: rproto,
			Ports:    rport,
		})
	}

	// build the rule
	rule := security.SGRule{
		Action:          action,
		FromIPAddresses: []string{fromIP},
		ToIPAddresses:   []string{toIP},
		ProtoPorts:      protoPort,
	}

	for _, pol := range spc.policies {
		pol.venicePolicy.Spec.Rules = append(pol.venicePolicy.Spec.Rules, rule)
	}

	return spc
}

// AddRulesForWorkloadPairs adds rule for each workload pair into the policies
func (spc *SGPolicyCollection) AddRulesForWorkloadPairs(wpc *WorkloadPairCollection, port, action string) *SGPolicyCollection {
	if spc.err != nil {
		return spc
	}
	if wpc.err != nil {
		return &SGPolicyCollection{err: wpc.err}
	}

	// walk each workload pair
	var pairNames []string
	for _, wpair := range wpc.pairs {
		fromIP := strings.Split(wpair.second.iotaWorkload.IpPrefix, "/")[0]
		toIP := strings.Split(wpair.first.iotaWorkload.IpPrefix, "/")[0]
		nspc := spc.AddRule(fromIP, toIP, port, action)
		if nspc.err != nil {
			return nspc
		}
		pairNames = append(pairNames, fmt.Sprintf("%s %s %s -> %s", action, port, fromIP, toIP))
	}

	log.Infof("Adding rules for pairs: %v", pairNames)

	return spc
}

// Commit writes the policy to venice
func (spc *SGPolicyCollection) Commit() error {
	if spc.err != nil {
		return spc.err
	}
	for _, pol := range spc.policies {
		err := pol.sm.tb.CreateSGPolicy(pol.venicePolicy)
		if err != nil {
			// try updating it
			err = pol.sm.tb.UpdateSGPolicy(pol.venicePolicy)
			if err != nil {
				spc.err = err
				return err
			}
		}

		log.Debugf("Created policy: %+v", pol.venicePolicy)

		pol.sm.sgpolicies[pol.venicePolicy.Name] = pol
	}

	return nil
}

// Delete deletes all policies in the collection
func (spc *SGPolicyCollection) Delete() error {
	if spc.err != nil {
		return spc.err
	}

	// walk all policies and delete them
	for _, pol := range spc.policies {
		err := pol.sm.tb.DeleteSGPolicy(pol.venicePolicy)
		if err != nil {
			return err
		}
		delete(pol.sm.sgpolicies, pol.venicePolicy.Name)
	}

	return nil
}

// Rules returns a list of all rule in policy collection
func (spc *SGPolicyCollection) Rules() *SGRuleCollection {
	if spc.err != nil {
		return &SGRuleCollection{
			err: spc.err,
		}
	}

	var ruleCol SGRuleCollection
	for _, pol := range spc.policies {
		for i := 0; i < len(pol.venicePolicy.Spec.Rules); i++ {
			rule := &pol.venicePolicy.Spec.Rules[i]
			rctx := sgRuleCtx{
				rule:   rule,
				policy: pol,
			}
			ruleCol.rules = append(ruleCol.rules, &rctx)
		}
	}

	return &ruleCol

}

// Update will update a list of rules. It expects update in the format `field = value`
func (rc *SGRuleCollection) Update(updateCmd string) *SGRuleCollection {
	if rc.err != nil {
		return rc
	}

	// some syntax checking
	updateCmd = strings.Replace(updateCmd, " ", "", -1)
	words := strings.Split(updateCmd, "=")
	if len(words) != 2 {
		rc.err = fmt.Errorf("Invalid update operation: %v", updateCmd)
		return rc
	}
	field := strings.ToLower(words[0])
	for _, rctx := range rc.rules {
		switch field {
		case "action":
			rctx.rule.Action = words[1]
		case "FromIP":
			rctx.rule.FromIPAddresses = strings.Split(words[1], ",")
		case "ToIP":
			rctx.rule.ToIPAddresses = strings.Split(words[1], ",")
		default:
			rc.err = fmt.Errorf("Invalid field name %s in update: %v", field, updateCmd)
			return rc
		}
	}

	return rc
}

// Commit writes all policies associates with these rules to venice
func (rc *SGRuleCollection) Commit() error {
	if rc.err != nil {
		return rc.err
	}

	// build a policy map from rules
	policyMap := make(map[string]*SGPolicy)
	for _, rctx := range rc.rules {
		policyMap[rctx.policy.venicePolicy.Name] = rctx.policy
	}

	for _, pol := range policyMap {
		err := pol.sm.tb.UpdateSGPolicy(pol.venicePolicy)
		if err != nil {
			return err
		}

		log.Debugf("Updated policy: %+v", pol.venicePolicy)
	}

	return nil
}
