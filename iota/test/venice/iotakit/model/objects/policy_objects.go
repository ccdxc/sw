package objects

import (
	"fmt"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

// NetworkSecurityPolicy represents security policy
type NetworkSecurityPolicy struct {
	VenicePolicy *security.NetworkSecurityPolicy
}

// App represents app object
type App struct {
	veniceApp *security.App
}

// NetworkSecurityPolicyCollection is list of policies
type NetworkSecurityPolicyCollection struct {
	CollectionCommon
	Policies []*NetworkSecurityPolicy
}

func NewNetworkSecurityPolicyCollection(policy *NetworkSecurityPolicy, client objClient.ObjClient, testbed *testbed.TestBed) *NetworkSecurityPolicyCollection {
	return &NetworkSecurityPolicyCollection{
		CollectionCommon: CollectionCommon{Client: client, Testbed: testbed},
		Policies: []*NetworkSecurityPolicy{
			policy,
		},
	}
}

// Delete deletes all policies in the collection
func (spc *NetworkSecurityPolicyCollection) Delete() error {
	if spc == nil {
		return nil
	}
	if spc.err != nil {
		return spc.err
	}

	// walk all policies and delete them
	for _, pol := range spc.Policies {
		err := spc.Client.DeleteNetworkSecurityPolicy(pol.VenicePolicy)
		if err != nil {
			return err
		}
		//delete(pol.sm.sgpolicies, pol.venicePolicy.Name)
	}

	// Added temporarily to avoid race conditions due to back-to-back delete followed by create
	time.Sleep(10 * time.Second)
	return nil
}

// DeleteAllRules deletes all rules in the policy
func (spc *NetworkSecurityPolicyCollection) DeleteAllRules() *NetworkSecurityPolicyCollection {
	if spc.err != nil {
		return spc
	}

	for _, pol := range spc.Policies {
		pol.VenicePolicy.Spec.Rules = []security.SGRule{}
	}

	return spc
}

// AddRulesForWorkloadPairs adds rule for each workload pair into the policies
func (spc *NetworkSecurityPolicyCollection) AddRulesForWorkloadPairs(wpc *WorkloadPairCollection, port, action string) *NetworkSecurityPolicyCollection {
	if spc.err != nil {
		return spc
	}
	if wpc.err != nil {
		spc := &NetworkSecurityPolicyCollection{}
		spc.SetError(wpc.Error())
		return spc
	}

	// walk each workload pair
	for _, wpair := range wpc.Pairs {
		fromIP := strings.Split(wpair.Second.iotaWorkload.IpPrefix, "/")[0]
		toIP := strings.Split(wpair.First.iotaWorkload.IpPrefix, "/")[0]
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
		spc := &NetworkSecurityPolicyCollection{}
		spc.SetError(wpc.Error())
		return spc
	}

	// walk each workload pair
	for _, wpair := range wpc.Pairs {
		fromIP := strings.Split(wpair.Second.iotaWorkload.IpPrefix, "/")[0]
		toIP := strings.Split(wpair.First.iotaWorkload.IpPrefix, "/")[0]
		// build the rule
		rule := security.SGRule{
			Action:          action,
			FromIPAddresses: []string{fromIP},
			ToIPAddresses:   []string{toIP},
			Apps:            []string{alg},
		}

		for _, pol := range spc.Policies {
			pol.VenicePolicy.Spec.Rules = append(pol.VenicePolicy.Spec.Rules, rule)
		}
	}

	return spc
}

// AddRuleForWorkloadCombo adds rule combinations
func (spc *NetworkSecurityPolicyCollection) AddRuleForWorkloadCombo(wpc *WorkloadPairCollection, fromIP, toIP, proto, port, action string) *NetworkSecurityPolicyCollection {
	for _, wpair := range wpc.Pairs {
		firstIP := strings.Split(wpair.First.iotaWorkload.IpPrefix, "/")[0]
		secondIP := strings.Split(wpair.Second.iotaWorkload.IpPrefix, "/")[0]
		firstSubnet := wpair.First.iotaWorkload.IpPrefix
		secondSubnet := wpair.Second.iotaWorkload.IpPrefix
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

		for _, pol := range spc.Policies {
			pol.VenicePolicy.Spec.Rules = append(pol.VenicePolicy.Spec.Rules, rule)
		}
	}

	return spc
}

// AddRule adds a rule to the policy
func (spc *NetworkSecurityPolicyCollection) AddRule(fromIP, toIP, port, action string) *NetworkSecurityPolicyCollection {
	if spc.HasError() {
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

	for _, pol := range spc.Policies {
		pol.VenicePolicy.Spec.Rules = append(pol.VenicePolicy.Spec.Rules, rule)
	}

	return spc
}

// Commit writes the policy to venice
func (spc *NetworkSecurityPolicyCollection) Commit() error {
	if spc.HasError() {
		return spc.err
	}

	for _, pol := range spc.Policies {
		err := spc.Client.CreateNetworkSecurityPolicy(pol.VenicePolicy)
		if err != nil {
			// try updating it
			err = spc.Client.UpdateNetworkSecurityPolicy(pol.VenicePolicy)
			if err != nil {
				spc.err = err
				return err
			}
		}

		log.Debugf("Created policy: %#v", pol.VenicePolicy)

	}

	return nil
}

// Restore is a very context specific function, which restores permit any any policy
func (spc *NetworkSecurityPolicyCollection) Restore() error {
	if spc == nil {
		return nil
	}
	return spc.AddRule("any", "any", "any", "PERMIT").Commit()
}

// Status returns venice status of the policy
func (spc *NetworkSecurityPolicyCollection) Status() ([]*security.NetworkSecurityPolicyStatus, error) {
	if spc.HasError() {
		return nil, spc.err
	}

	var vsts []*security.NetworkSecurityPolicyStatus
	for _, pol := range spc.Policies {
		sgp, err := spc.Client.GetNetworkSecurityPolicy(&pol.VenicePolicy.ObjectMeta)
		if err != nil {
			return nil, err
		}
		vsts = append(vsts, &sgp.Status)
	}

	return vsts, nil
}
