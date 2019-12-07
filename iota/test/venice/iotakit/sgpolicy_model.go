// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"fmt"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/utils/log"
)

// NetworkSecurityPolicy represents security policy
type NetworkSecurityPolicy struct {
	venicePolicy *security.NetworkSecurityPolicy
	sm           *SysModel // pointer back to the model
}

// NetworkSecurityPolicyCollection is list of policies
type NetworkSecurityPolicyCollection struct {
	err      error
	policies []*NetworkSecurityPolicy
}

type sgRuleCtx struct {
	rule   *security.SGRule
	policy *NetworkSecurityPolicy
}

// SGRuleCollection is list of rules
type SGRuleCollection struct {
	err   error
	rules []*sgRuleCtx
}

// App represents app object
type App struct {
	veniceApp *security.App
	sm        *SysModel // pointer back to the model
}

// NewNetworkSecurityPolicy creates a new SG policy
func (sm *SysModel) NewNetworkSecurityPolicy(name string) *NetworkSecurityPolicyCollection {
	return &NetworkSecurityPolicyCollection{
		policies: []*NetworkSecurityPolicy{
			{
				venicePolicy: &security.NetworkSecurityPolicy{
					TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
					ObjectMeta: api.ObjectMeta{
						Tenant:    "default",
						Namespace: "default",
						Name:      name,
					},
					Spec: security.NetworkSecurityPolicySpec{
						AttachTenant: true,
					},
				},
				sm: sm,
			},
		},
	}
}

// NewVeniceNetworkSecurityPolicy creates a new SG policy
func (sm *SysModel) NewVeniceNetworkSecurityPolicy(policy *NetworkSecurityPolicy) *NetworkSecurityPolicyCollection {
	return &NetworkSecurityPolicyCollection{
		policies: []*NetworkSecurityPolicy{
			policy,
		},
	}
}

// NetworkSecurityPolicy finds an SG policy by name
func (sm *SysModel) NetworkSecurityPolicy(name string) *NetworkSecurityPolicyCollection {
	pol, ok := sm.sgpolicies[name]
	if !ok {
		return &NetworkSecurityPolicyCollection{
			err: fmt.Errorf("No policy found for %v", name),
		}
	}

	return &NetworkSecurityPolicyCollection{
		policies: []*NetworkSecurityPolicy{pol},
	}
}

// SGPolicies returns all SGPolicies in the model
func (sm *SysModel) SGPolicies() *NetworkSecurityPolicyCollection {
	spc := NetworkSecurityPolicyCollection{}
	for _, pol := range sm.sgpolicies {
		spc.policies = append(spc.policies, pol)
	}

	return &spc
}

// DefaultNetworkSecurityPolicy resturns default-policy that prevails across tests cases in the system
func (sm *SysModel) DefaultNetworkSecurityPolicy() *NetworkSecurityPolicyCollection {
	return sm.defaultSgPolicies[0]
}

// Restore is a very context specific function, which restores permit any any policy
func (spc *NetworkSecurityPolicyCollection) Restore() error {
	return spc.AddRule("any", "any", "any", "PERMIT").Commit()
}

// Status returns venice status of the policy
func (spc *NetworkSecurityPolicyCollection) Status() ([]*security.NetworkSecurityPolicyStatus, error) {
	if spc.err != nil {
		return nil, spc.err
	}

	var vsts []*security.NetworkSecurityPolicyStatus
	for _, pol := range spc.policies {
		sgp, err := pol.sm.GetNetworkSecurityPolicy(&pol.venicePolicy.ObjectMeta)
		if err != nil {
			return nil, err
		}
		vsts = append(vsts, &sgp.Status)
	}

	return vsts, nil
}

// AddRule adds a rule to the policy
func (spc *NetworkSecurityPolicyCollection) AddRule(fromIP, toIP, port, action string) *NetworkSecurityPolicyCollection {
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
	if port == "" || port == "any" {
		protoPort = append(protoPort, security.ProtoPort{
			Protocol: "any",
		})
	} else {
		params := strings.Split(port, "/")
		if len(params) == 1 {
			pstr := strings.ToLower(params[0])

			if pstr == "tcp" || pstr == "udp" || pstr == "icmp" {
				rproto = pstr
			} else {
				rproto = "tcp"
				// Check if port is a hypen separted range.
				components := strings.Split(pstr, "-")
				if len(components) == 2 {
					rport = pstr
				} else if _, err := strconv.Atoi(pstr); err != nil {
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
			// Check if port is a hypen separted range.
			components := strings.Split(params[1], "-")
			if len(components) == 2 {
				rport = params[1]
			} else if _, err := strconv.Atoi(params[1]); err != nil {
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
func (spc *NetworkSecurityPolicyCollection) AddRulesForWorkloadPairs(wpc *WorkloadPairCollection, port, action string) *NetworkSecurityPolicyCollection {
	if spc.err != nil {
		return spc
	}
	if wpc.err != nil {
		return &NetworkSecurityPolicyCollection{err: wpc.err}
	}

	// walk each workload pair
	for _, wpair := range wpc.pairs {
		fromIP := strings.Split(wpair.second.iotaWorkload.IpPrefix, "/")[0]
		toIP := strings.Split(wpair.first.iotaWorkload.IpPrefix, "/")[0]
		nspc := spc.AddRule(fromIP, toIP, port, action)
		if nspc.err != nil {
			return nspc
		}
	}

	return spc
}

// AddAlgRulesForWorkloadPairs adds ALG rule for each workload pair
func (spc *NetworkSecurityPolicyCollection) AddAlgRulesForWorkloadPairs(wpc *WorkloadPairCollection, alg, action string) *NetworkSecurityPolicyCollection {
	if spc.err != nil {
		return spc
	}
	if wpc.err != nil {
		return &NetworkSecurityPolicyCollection{err: wpc.err}
	}

	// walk each workload pair
	for _, wpair := range wpc.pairs {
		fromIP := strings.Split(wpair.second.iotaWorkload.IpPrefix, "/")[0]
		toIP := strings.Split(wpair.first.iotaWorkload.IpPrefix, "/")[0]
		// build the rule
		rule := security.SGRule{
			Action:          action,
			FromIPAddresses: []string{fromIP},
			ToIPAddresses:   []string{toIP},
			Apps:            []string{alg},
		}

		for _, pol := range spc.policies {
			pol.venicePolicy.Spec.Rules = append(pol.venicePolicy.Spec.Rules, rule)
		}
	}

	return spc
}

// AddRuleForWorkloadCombo adds rule combinations
func (spc *NetworkSecurityPolicyCollection) AddRuleForWorkloadCombo(wpc *WorkloadPairCollection, fromIP, toIP, proto, port, action string) *NetworkSecurityPolicyCollection {
	for _, wpair := range wpc.pairs {
		firstIP := strings.Split(wpair.first.iotaWorkload.IpPrefix, "/")[0]
		secondIP := strings.Split(wpair.second.iotaWorkload.IpPrefix, "/")[0]
		firstSubnet := wpair.first.iotaWorkload.IpPrefix
		secondSubnet := wpair.second.iotaWorkload.IpPrefix
		// build the rule
		rule := security.SGRule{
			Action: action,
		}

		// determine from ip
		switch fromIP {
		case "workload-ip":
			rule.FromIPAddresses = []string{secondIP}
			if proto == "icmp" || proto == "any" {
				rule.FromIPAddresses = []string{firstIP, secondIP}
			}
		case "workload-subnet":
			rule.FromIPAddresses = []string{secondSubnet}
		case "any":
			rule.FromIPAddresses = []string{"any"}
		default:
			log.Fatalf("Invalid fromIP: %s", fromIP)
		}

		// determine to ip
		switch toIP {
		case "workload-ip":
			rule.ToIPAddresses = []string{firstIP}
			if proto == "icmp" || proto == "any" {
				rule.ToIPAddresses = []string{firstIP, secondIP}
			}
		case "workload-subnet":
			rule.ToIPAddresses = []string{firstSubnet}
		case "any":
			rule.ToIPAddresses = []string{"any"}
		default:
			log.Fatalf("Invalid fromIP: %s", fromIP)
		}

		// determine protocol
		switch proto {
		case "any":
			fallthrough
		case "icmp":
			rule.ProtoPorts = append(rule.ProtoPorts, security.ProtoPort{Protocol: proto})
		default:

			// determine ports
			if port != "any" {
				pp := security.ProtoPort{
					Protocol: proto,
					Ports:    port,
				}
				rule.ProtoPorts = append(rule.ProtoPorts, pp)
			} else {
				rule.ProtoPorts = append(rule.ProtoPorts, security.ProtoPort{Protocol: proto, Ports: "0-65535"})
			}

		}

		log.Infof("Adding rule: %#v", rule)

		for _, pol := range spc.policies {
			pol.venicePolicy.Spec.Rules = append(pol.venicePolicy.Spec.Rules, rule)
		}
	}

	return spc
}

// DeleteAllRules deletes all rules in the policy
func (spc *NetworkSecurityPolicyCollection) DeleteAllRules() *NetworkSecurityPolicyCollection {
	if spc.err != nil {
		return spc
	}

	for _, pol := range spc.policies {
		pol.venicePolicy.Spec.Rules = []security.SGRule{}
	}

	return spc
}

// Commit writes the policy to venice
func (spc *NetworkSecurityPolicyCollection) Commit() error {
	if spc.err != nil {
		return spc.err
	}
	for _, pol := range spc.policies {
		err := pol.sm.CreateNetworkSecurityPolicy(pol.venicePolicy)
		if err != nil {
			// try updating it
			err = pol.sm.UpdateNetworkSecurityPolicy(pol.venicePolicy)
			if err != nil {
				spc.err = err
				return err
			}
		}

		log.Debugf("Created policy: %#v", pol.venicePolicy)

		pol.sm.sgpolicies[pol.venicePolicy.Name] = pol
	}

	return nil
}

// Delete deletes all policies in the collection
func (spc *NetworkSecurityPolicyCollection) Delete() error {
	if spc.err != nil {
		return spc.err
	}

	// walk all policies and delete them
	for _, pol := range spc.policies {
		err := pol.sm.DeleteNetworkSecurityPolicy(pol.venicePolicy)
		if err != nil {
			return err
		}
		delete(pol.sm.sgpolicies, pol.venicePolicy.Name)
	}

	// Added temporarily to avoid race conditions due to back-to-back delete followed by create
	time.Sleep(10 * time.Second)
	return nil
}

// Rules returns a list of all rule in policy collection
func (spc *NetworkSecurityPolicyCollection) Rules() *SGRuleCollection {
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
	policyMap := make(map[string]*NetworkSecurityPolicy)
	for _, rctx := range rc.rules {
		policyMap[rctx.policy.venicePolicy.Name] = rctx.policy
	}

	for _, pol := range policyMap {
		err := pol.sm.UpdateNetworkSecurityPolicy(pol.venicePolicy)
		if err != nil {
			return err
		}

		log.Debugf("Updated policy: %+v", pol.venicePolicy)
	}

	return nil
}
