package impl

import (
	"context"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// TODO Add this in the proto definition to easily validate
var supportedProtocols = []string{"tcp", "udp", "icmp"}

type sgPolicyHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

// Following checks are enforced:
// Cannot specify both `AttachTenant` and `AttachGroups`
// `Apps` should have a valid proto/port. proto should either be tcp, udp or icmp. Port < 64K
// `FromIPAddress` should have individual IP Address, IP Mask or hyphen separated IP Range
// `ToIPAddress` should have individual IP Address, IP Mask or hyphen separated IP Range
// Specifying `AttachTenant` should mandatorily have `FromIPAddress` and `ToIPAddresses`
// For `FromIPAddresses` and `ToIPAddresses` cannot be empty. If the intent is to allow all. Enforce a mandatory `any` keyword.
func (s *sgPolicyHooks) validateSGPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, i interface{}) (interface{}, bool, error) {
	sgp, ok := i.(security.SGPolicy)
	if !ok {
		return i, false, fmt.Errorf("invalid input type")
	}

	if sgp.Spec.AttachTenant && len(sgp.Spec.AttachGroups) > 0 {
		return i, false, fmt.Errorf("must specify a single attachment point")
	}

	if sgp.Spec.AttachTenant == false && len(sgp.Spec.AttachGroups) == 0 {
		return i, false, fmt.Errorf("must specify atleast one of attach-tenant or attach-groups")
	}
	err := s.validateApps(sgp.Spec.Rules)
	if err != nil {
		return i, false, fmt.Errorf("app validation failed. Error: %v", err)
	}

	for _, r := range sgp.Spec.Rules {
		if sgp.Spec.AttachTenant {
			// AttachTenant must have To and From IPAddresses and groups
			if len(r.FromIPAddresses) == 0 && len(r.FromSecurityGroups) == 0 {
				return i, false, fmt.Errorf("must specify either from-ip-addresses or from-security-groups when attaching the policy at the tenant level. %v", r)
			}
			if len(r.ToIPAddresses) == 0 && len(r.ToSecurityGroups) == 0 {
				return i, false, fmt.Errorf("must specify either to-ip-addresses or to-security-groups when attaching the policy at the tenant level. %v", r)
			}
			err := s.validateIPAddresses(r.FromIPAddresses)
			if err != nil {
				return i, false, fmt.Errorf("could not validate one or more ip address in %v. Error: %v", r.FromIPAddresses, err)
			}

			err = s.validateIPAddresses(r.ToIPAddresses)
			if err != nil {
				return i, false, fmt.Errorf("could not validate one or more ip addresses in %v. Error: %v", r.ToIPAddresses, err)
			}
		} else {
			err := s.validateIPAddresses(r.ToIPAddresses)
			if err != nil {
				return i, false, fmt.Errorf("could not validate one or more ip addresses in to-ip-addresses %v. Error: %v", r.ToIPAddresses, err)
			}
			err = s.validateIPAddresses(r.FromIPAddresses)
			if err != nil {
				return i, false, fmt.Errorf("could not validate one or more ip addresses in from-ip-addresses %v. Error: %v", r.FromIPAddresses, err)
			}
		}
	}

	return i, true, nil
}

// validateApps will enforce a valid proto/port declaration.
// references to the named apps will be handled by the controller.
func (s *sgPolicyHooks) validateApps(rules []*security.SGRule) error {
	for _, r := range rules {
		for _, a := range r.Apps {
			found := false
			components := strings.Split(a, "/")
			// defer named apps validation to the controller
			if len(components) == 0 {
				continue
			}
			if len(components) != 2 {
				return fmt.Errorf("must specify app as protocol/port format. Found %v", a)
			}
			proto, port := components[0], components[1]
			for _, p := range supportedProtocols {
				if p == proto {
					found = true
					break
				}
			}

			if !found {
				return fmt.Errorf("invalid protocol in SGRule. %v", proto)
			}

			if proto != "tcp" && proto != "udp" && proto != "icmp" {
				return fmt.Errorf("invalid protocol in SGRule. %v", a)
			}

			if len(port) == 0 {
				return fmt.Errorf("empty port field")
			}
			i, err := strconv.Atoi(port)
			if err != nil {
				return fmt.Errorf("port must be an integer value in the SGRule. %v", port)
			}
			if 0 > i || i > 65535 {
				return fmt.Errorf("port outside range. %v", a)
			}
		}
	}
	return nil
}

// validates the IP Addresses in SG rules. Each IP Address must either be a standard octet, ipmask, hyphen separated ip range, or must use any as a keyword
func (s *sgPolicyHooks) validateIPAddresses(addresses []string) error {
	for _, a := range addresses {
		// special keyword to denote match all on ip addresses
		if a == "any" {
			continue
		}
		if strings.Contains(a, "-") {
			// Try Parsing as a range
			ipRange := strings.Split(a, "-")
			if len(ipRange) != 2 {
				return fmt.Errorf("ip range doesn't doesn't contain begin and end ip addresses, specified %v", a)
			}

			startIP := net.ParseIP(strings.TrimSpace(ipRange[0]))
			endIP := net.ParseIP(strings.TrimSpace(ipRange[1]))
			if len(startIP) > 0 && len(endIP) > 0 {
				continue
			} else {
				return fmt.Errorf("could not parse IP from %v", a)
			}
		} else if strings.Contains(a, "/") {
			// try parsing as a CIDR block
			_, _, err := net.ParseCIDR(strings.TrimSpace(a))
			if err == nil {
				// found a valid cidr block
				continue
			} else {
				return fmt.Errorf("could not parse IP CIDR block from %v", a)
			}
		} else {
			// try parsing as an IP
			ipAddress := net.ParseIP(strings.TrimSpace(a))
			if len(ipAddress) > 0 {
				// found valid ip address in octet form
				continue
			} else {
				return fmt.Errorf("could not parse IP Address from %v", a)
			}

		}
	}

	return nil
}

func registerSGPolicyHooks(svc apiserver.Service, logger log.Logger) {
	r := sgPolicyHooks{
		svc:    svc,
		logger: logger.WithContext("Service", "SGPolicy"),
	}
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("SGPolicy", apiserver.CreateOper).WithPreCommitHook(r.validateSGPolicy)
	svc.GetCrudService("SGPolicy", apiserver.UpdateOper).WithPreCommitHook(r.validateSGPolicy)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("security.SecurityV1", registerSGPolicyHooks)
}
