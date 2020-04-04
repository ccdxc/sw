package impl

import (
	"context"
	"errors"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/venice/utils/featureflags"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/security"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// TODO Add this in the proto definition to easily validate
var supportedProtocols = []string{"tcp", "udp", "icmp", "any"}

type securityHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

const (
	ipNone = iota
	ipV4
	ipV6
)

// Following checks are enforced:
// Cannot specify both `AttachTenant` and `AttachGroups`
// `Apps` should have a valid proto/port. proto should either be tcp, udp or icmp. Port < 64K
// `FromIPAddress` should have individual IP Address, IP Mask or hyphen separated IP Range
// `ToIPAddress` should have individual IP Address, IP Mask or hyphen separated IP Range
// Specifying `AttachTenant` should mandatorily have `FromIPAddress` and `ToIPAddresses`
// For `FromIPAddresses` and `ToIPAddresses` cannot be empty. If the intent is to allow all. Enforce a mandatory `any` keyword.
// func (s *securityHooks) validateNetworkSecurityPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
func (s *securityHooks) validateNetworkSecurityPolicy(i interface{}, ver string, ignoreStatus, ignoreSpec bool) []error {
	if ignoreSpec {
		// only spec is validated in the hook
		return nil
	}
	sgp, ok := i.(security.NetworkSecurityPolicy)
	ret := []error{}
	if !ok {
		return []error{fmt.Errorf("invalid input type")}
	}

	if sgp.Spec.AttachTenant && len(sgp.Spec.AttachGroups) > 0 {
		ret = append(ret, fmt.Errorf("must specify a single attachment point"))
	}

	if sgp.Spec.AttachTenant == false && len(sgp.Spec.AttachGroups) == 0 {
		ret = append(ret, fmt.Errorf("must specify a single attachment point"))
	}
	err := s.validateProtoPort(sgp.Spec.Rules)
	if err != nil {
		ret = append(ret, err)
	}

	for _, r := range sgp.Spec.Rules {
		if sgp.Spec.AttachTenant {
			// AttachTenant must have To and From IPAddresses and groups
			if len(r.FromIPAddresses) == 0 && len(r.FromSecurityGroups) == 0 {
				ret = append(ret, fmt.Errorf("must specify either from-ip-addresses or from-security-groups when attaching the policy at the tenant level. %v", r))
			}
			if len(r.ToIPAddresses) == 0 && len(r.ToSecurityGroups) == 0 {
				ret = append(ret, fmt.Errorf("must specify either to-ip-addresses or to-security-groups when attaching the policy at the tenant level. %v", r))
			}

			toIPFamily, err := s.validateIPAddresses(r.FromIPAddresses)
			if err != nil {
				ret = append(ret, fmt.Errorf("could not validate one or more ip address in %v. Error: %v", r.FromIPAddresses, err))
			}

			fromIPFamily, err := s.validateIPAddresses(r.ToIPAddresses)
			if err != nil {
				ret = append(ret, fmt.Errorf("could not validate one or more ip addresses in %v. Error: %v", r.ToIPAddresses, err))
			}
			// ignore ipNone as to allow any to mean both v4 any or v6 any
			if toIPFamily != ipNone && fromIPFamily != ipNone && toIPFamily != fromIPFamily {
				ret = append(ret, fmt.Errorf("from and to ip addresses must belong the the same family. Error: %v", err))
			}
		} else {
			toIPFamily, err := s.validateIPAddresses(r.ToIPAddresses)
			if err != nil {
				ret = append(ret, fmt.Errorf("could not validate one or more ip addresses in to-ip-addresses %v. Error: %v", r.ToIPAddresses, err))
			}
			fromIPFamily, err := s.validateIPAddresses(r.FromIPAddresses)
			if err != nil {
				ret = append(ret, fmt.Errorf("could not validate one or more ip addresses in from-ip-addresses %v. Error: %v", r.FromIPAddresses, err))
			}

			if toIPFamily != ipNone && fromIPFamily != ipNone && fromIPFamily != toIPFamily {
				ret = append(ret, fmt.Errorf("from and to ip addresses must belong the the same family. Error: %v", err))
			}
		}
	}

	return ret
}

// enforceMaxNetworkSecurityPolicyPreCommitHook ensures that SG Policies will not exceed globals.MaxAllowedSGPolicies
func (s *securityHooks) enforceMaxNetworkSecurityPolicyPreCommitHook(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	policy, ok := i.(security.NetworkSecurityPolicy)
	if !ok {
		return i, false, fmt.Errorf("invalid object type %T. Expecting NetworkSecurityPolicy", i)
	}

	if ctx == nil || kvs == nil {
		return i, false, fmt.Errorf("enforceMaxNetworkSecurityPolicyPreCommitHook called with NIL parameter, ctx: %p, kvs: %p", ctx, kvs)
	}

	if featureflags.IsNetworkSecPolicyEnabled() {
		return i, true, nil
	}

	switch oper {
	case apiintf.CreateOper:
		var sgPolicies security.NetworkSecurityPolicyList
		pol := security.NetworkSecurityPolicy{}
		pol.Tenant = policy.Tenant
		sgpKey := strings.TrimSuffix(pol.MakeKey(string(apiclient.GroupSecurity)), "/")

		if err := kvs.List(ctx, sgpKey, &sgPolicies); err != nil {
			return nil, true, fmt.Errorf("failed to list SGPolicies. Err: %v", err)
		}

		// filter out policy with same name
		for idx, p := range sgPolicies.Items {
			if p.Name == policy.Name {
				sgPolicies.Items = append(sgPolicies.Items[:idx], sgPolicies.Items[idx+1:]...)
			}
		}

		if len(sgPolicies.Items) == globals.MaxAllowedSGPolicies {
			log.Errorf("failed to create Network Security Policy: %s, exceeds max allowed polices %d", policy.Name, globals.MaxAllowedSGPolicies)
			return nil, true, fmt.Errorf("failed to create Network Security Policy: %s, exceeds max allowed polices %d", policy.Name, globals.MaxAllowedSGPolicies)
		}
		return i, true, nil
	default:
		return i, true, nil
	}

}

// enforceMaxSGRulePreCommitHook ensures that SG Policies will not exceed globals.MaxAllowedSGRules
func (s *securityHooks) enforceMaxSGRulePreCommitHook(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	policy, ok := i.(security.NetworkSecurityPolicy)
	if !ok {
		return i, false, fmt.Errorf("invalid object type %T. Expecting NetworkSecurityPolicy", i)
	}

	if ctx == nil || kvs == nil {
		return i, false, fmt.Errorf("enforceMaxNetworkSecurityPolicyPreCommitHook called with NIL parameter, ctx: %p, kvs: %p", ctx, kvs)
	}

	switch oper {
	case apiintf.UpdateOper:
		fallthrough
	case apiintf.CreateOper:
		// check if we are exceeding max limit
		numRules := len(policy.Spec.Rules)
		log.Infof("Found %d rules", numRules)
		if numRules > globals.MaxAllowedSGRules {
			log.Errorf("Failed to create Network Security Policy: %s with %d rules, exceeds max allowed rules of %d", policy.Name, numRules, globals.MaxAllowedSGRules)
			return nil, false, fmt.Errorf("failed to create Network Security Policy: %s with %d rules, exceeds max allowed rules of %d", policy.Name, numRules, globals.MaxAllowedSGRules)
		}
		return i, true, nil
	default:
		return i, true, nil
	}

}

// validateProtoPort will enforce a valid proto/port declaration.
// references to the named apps will be handled by the controller.
func (s *securityHooks) validateProtoPort(rules []security.SGRule) error {
	for i, r := range rules {
		// verify a rule cant have both proto port and app
		if len(r.Apps) != 0 && len(r.ProtoPorts) != 0 {
			return fmt.Errorf("Can not have both app and protocol/port in rule[%d]: %v", i, r)
		}
		// error out on a rule which is missing proto-ports and a valid app
		if len(r.ProtoPorts) == 0 && len(r.Apps) == 0 {
			return fmt.Errorf("proto-ports are mandatory, when the rules don't refer to an App in rule[%d]: %v", i, r)
		}
		for _, pp := range r.ProtoPorts {
			protoNum, err := strconv.Atoi(pp.Protocol)
			if err == nil {
				// protocol number specified, check its supported in the curent release
				if err := s.validateIANA(protoNum); err != nil {
					return fmt.Errorf("unsupported IANA Protocol number. Supported values: 1, 6 and 17. Found: %d", protoNum)
				}
			} else {
				found := false
				for _, p := range supportedProtocols {
					// check if any is specified in protocol
					if strings.ToLower(pp.Protocol) == "any" {
						if len(r.ProtoPorts) > 1 {
							return fmt.Errorf("cannot specify more than one proto-ports if any is specified since this supersedes other proto-ports within a rule. Found %d proto-ports", len(r.ProtoPorts))
						}
						if len(pp.Ports) != 0 {
							return fmt.Errorf("cannot specify ports when protocol is any. Ports: %v", pp.Ports)
						}
					}

					if p == strings.ToLower(pp.Protocol) {
						found = true
						break
					}
				}

				if !found {
					return fmt.Errorf("invalid protocol %v in rule[%d]: %v", pp.Protocol, i, r)
				}
			}

			if len(pp.Ports) != 0 {
				// you can not specify ports for icmp
				if strings.ToLower(pp.Protocol) == "icmp" || strings.ToLower(pp.Protocol) == "1" {
					return fmt.Errorf("Can not specify ports for ICMP protocol in rule[%d]: %v", i, r)
				}

				// parse port ranges
				portRanges := strings.Split(pp.Ports, ",")
				for _, prange := range portRanges {
					ports := strings.Split(prange, "-")
					for _, port := range ports {
						i, err := strconv.Atoi(port)
						if err != nil {
							return fmt.Errorf("port %v must be an integer value in the rule[%d]: %v", port, i, r)
						}
						if i < 0 || i > 65535 {
							return fmt.Errorf("port %v outside range in rule[%d]: %v", port, i, r)
						}
					}
					if len(ports) == 2 {
						first, _ := strconv.Atoi(ports[0])
						second, _ := strconv.Atoi(ports[1])
						if first == 0 && second == 0 {
							return fmt.Errorf("invalid port range %v. upper and lower port range bounds must not be 0", prange)
						}
						if first > second {
							return fmt.Errorf("Invalid port range %v. first number bigger than second in rule[%d]: %v", prange, i, r)
						}
					} else if len(ports) > 2 {
						return fmt.Errorf("Invalid port range format: %v in rule[%d]: %v", prange, i, r)
					}
				}
			} else if s.isProtocolTCPorUDP(pp.Protocol) {
				// Reject empty ports only on non icmp protocol
				return fmt.Errorf("ports are mandatory. Use 0-65535 if the intent is to allow all in rule[%d]: %v", i, r)
			}
		}
	}
	return nil
}

// validates the IP Addresses in SG rules. Each IP Address must either be a standard octet, ipmask, hyphen separated ip range, or must use any as a keyword
func (s *securityHooks) validateIPAddresses(addresses []string) (int, error) {
	var ipAddresses []*net.IP
	for _, a := range addresses {
		// special keyword to denote match all on ip addresses
		if strings.ToLower(a) == "any" {
			if len(addresses) > 1 {
				return ipNone, fmt.Errorf("cannot specify more than one IP addresses if any is specified since this supersedes other IP addresses within a rule. Found %d proto-ports", len(addresses))
			}
			continue
		}
		if strings.Contains(a, "-") {
			// Try Parsing as a range
			ipRange := strings.Split(a, "-")
			if len(ipRange) != 2 {
				return ipNone, fmt.Errorf("ip range doesn't doesn't contain begin and end ip addresses, specified %v", a)
			}

			startIP := net.ParseIP(strings.TrimSpace(ipRange[0]))
			endIP := net.ParseIP(strings.TrimSpace(ipRange[1]))
			if len(startIP) > 0 && len(endIP) > 0 {
				ipAddresses = append(ipAddresses, &startIP, &endIP)
				continue
			} else {
				return ipNone, fmt.Errorf("could not parse IP from %v", a)
			}
		} else if strings.Contains(a, "/") {
			// try parsing as a CIDR block
			ip, _, err := net.ParseCIDR(strings.TrimSpace(a))
			if err == nil {
				// found a valid cidr block
				ipAddresses = append(ipAddresses, &ip)
				continue
			} else {
				return ipNone, fmt.Errorf("could not parse IP CIDR block from %v", a)
			}
		} else {
			// try parsing as an IP
			ipAddress := net.ParseIP(strings.TrimSpace(a))
			if len(ipAddress) > 0 {
				// found valid ip address in octet form
				ipAddresses = append(ipAddresses, &ipAddress)
				continue
			} else {
				return ipNone, fmt.Errorf("could not parse IP Address from %v", a)
			}
		}
	}

	return s.validateIPAddressFamily(ipAddresses)
}

// validates that the IP Addresses are in the same family. For FCS release we are allowing only v4 IPs in rules.
func (s *securityHooks) validateIPAddressFamily(ipAddresses []*net.IP) (ipFamily int, err error) {
	// First IP Address determines the family
	for _, ip := range ipAddresses {
		if ip.To4() == nil {
			// fail
			err = errors.New("v6 ip addresses are not supported")
			return
		}
		ipFamily = ipV4
	}
	return
}

// validateApp validates contents of app
func (s *securityHooks) validateApp(in interface{}, ver string, ignoreStatus, ignoreSpec bool) []error {
	if ignoreSpec {
		// only spec is validated in this hook
		return nil
	}
	ret := []error{}
	protocolNameValidate := func(protoNames []string, checkProto string, matchType string) bool {
		switch matchType {
		case "mustMatchAll":
			for _, protoName := range protoNames {
				if protoName != checkProto {
					return false
				}
			}
		default:
		}
		return true
	}

	app, ok := in.(security.App)
	if !ok {
		return []error{fmt.Errorf("invalid input type")}
	}

	// disallow apps that don't have at least one of ProtoPorts and ALG
	if (app.Spec.ProtoPorts == nil || len(app.Spec.ProtoPorts) == 0) && (app.Spec.ALG == nil || app.Spec.ALG.Type == "") {
		return []error{fmt.Errorf("app doesn't have at least one of ProtoPorts and ALG")}
	}

	appProtos := []string{}
	for _, pp := range app.Spec.ProtoPorts {
		protoNum, err := strconv.Atoi(pp.Protocol)
		if err == nil {
			// protocol number specified, check its between 0-255 range (0 is valid proto)
			if err := s.validateIANA(protoNum); err != nil {
				ret = append(ret, fmt.Errorf("unsupported IANA Protocol number. Supported values: 1, 6 and 17. Found: %d", protoNum))
			}
			if protoNum < 0 || protoNum >= 255 {
				ret = append(ret, fmt.Errorf("Invalid protocol number %v", pp.Protocol))
			}
		} else {
			found := false
			for _, p := range supportedProtocols {
				if strings.ToLower(pp.Protocol) == "any" {
					if len(app.Spec.ProtoPorts) > 1 {
						ret = append(ret, fmt.Errorf("cannot specify more than one proto-ports if any is specified since this supersedes other proto-ports within a rule. Found %d proto-ports", len(app.Spec.ProtoPorts)))
					}
					if len(pp.Ports) != 0 {
						ret = append(ret, fmt.Errorf("cannot specify ports when protocol is any. Ports: %v", pp.Ports))
					}
				}

				if s.isProtocolTCPorUDP(pp.Protocol) && len(pp.Ports) == 0 {
					ret = append(ret, errors.New("ports are mandatory for tcp or udp"))
				}

				if p == strings.ToLower(pp.Protocol) {
					found = true
					break
				}
			}

			if !found {
				ret = append(ret, fmt.Errorf("invalid protocol %v", pp.Protocol))
			}
		}
		appProtos = append(appProtos, pp.Protocol)

		if len(pp.Ports) != 0 {
			// you can not specify ports for icmp
			if strings.ToLower(pp.Protocol) == "icmp" || strings.ToLower(pp.Protocol) == "1" {
				ret = append(ret, fmt.Errorf("Can not specify ports for ICMP protocol"))
			}

			portRanges := strings.Split(pp.Ports, ",")
			for _, prange := range portRanges {
				ports := strings.Split(prange, "-")
				for _, port := range ports {
					i, err := strconv.Atoi(port)
					if err != nil {
						ret = append(ret, fmt.Errorf("port %v must be an integer value", port))
					}
					if 0 > i || i > 65535 {
						ret = append(ret, fmt.Errorf("port %v outside range", port))
					}
				}
				if len(ports) == 2 {
					first, _ := strconv.Atoi(ports[0])
					second, _ := strconv.Atoi(ports[1])

					if first == 0 && second == 0 {
						ret = append(ret, fmt.Errorf("invalid port range %v. upper and lower port range bounds must not be 0", prange))
					}

					if first > second {
						ret = append(ret, fmt.Errorf("Invalid port range %v. first number bigger than second", prange))
					}
				} else if len(ports) > 2 {
					ret = append(ret, fmt.Errorf("Invalid port range format: %v", prange))
				}
			}
		}
	}

	if !featureflags.AreALGsEnabled() && app.Spec.ALG != nil && app.Spec.ALG.Type != security.ALG_ICMP.String() {
		ret = append(ret, fmt.Errorf("ALGs other ICMP are not supported"))
	}

	// validate ALG types
	if featureflags.AreALGsEnabled() && app.Spec.ALG != nil && app.Spec.ALG.Type != "" {

		switch app.Spec.ALG.Type {
		case security.ALG_ICMP.String():
			if (app.Spec.ALG.Dns != nil) ||
				(app.Spec.ALG.Ftp != nil) ||
				(app.Spec.ALG.Sunrpc != nil) ||
				(app.Spec.ALG.Msrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}

			// validate ICMP type
			if app.Spec.ALG.Icmp != nil && app.Spec.ALG.Icmp.Type != "" {
				i, err := strconv.Atoi(app.Spec.ALG.Icmp.Type)
				if err != nil {
					ret = append(ret, fmt.Errorf("ICMP Type %v must be an integer value", app.Spec.ALG.Icmp.Type))
				}
				if 0 > i || i > 255 {
					ret = append(ret, fmt.Errorf("ICMP Type %v outside range", app.Spec.ALG.Icmp.Type))
				}
			}
			if app.Spec.ALG.Icmp != nil && app.Spec.ALG.Icmp.Code != "" {
				i, err := strconv.Atoi(app.Spec.ALG.Icmp.Code)
				if err != nil {
					ret = append(ret, fmt.Errorf("ICMP Code %v must be an integer value", app.Spec.ALG.Icmp.Code))
				}
				if 0 > i || i > 18 {
					ret = append(ret, fmt.Errorf("ICMP Code %v outside range", app.Spec.ALG.Icmp.Code))
				}
			}
		case security.ALG_DNS.String():
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Ftp != nil) ||
				(app.Spec.ALG.Sunrpc != nil) ||
				(app.Spec.ALG.Msrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
			if !protocolNameValidate(appProtos, "udp", "mustMatchAll") {
				ret = append(ret, fmt.Errorf("Protocol(s) %v is not allowed with DNS ALG", appProtos))
			}
		case security.ALG_FTP.String():
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Dns != nil) ||
				(app.Spec.ALG.Sunrpc != nil) ||
				(app.Spec.ALG.Msrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
		case security.ALG_SunRPC.String():
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Dns != nil) ||
				(app.Spec.ALG.Ftp != nil) ||
				(app.Spec.ALG.Msrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
		case security.ALG_MSRPC.String():
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Dns != nil) ||
				(app.Spec.ALG.Ftp != nil) ||
				(app.Spec.ALG.Sunrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
		case security.ALG_TFTP.String():
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Dns != nil) ||
				(app.Spec.ALG.Ftp != nil) ||
				(app.Spec.ALG.Sunrpc != nil) ||
				(app.Spec.ALG.Msrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
		case security.ALG_RTSP.String():
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Dns != nil) ||
				(app.Spec.ALG.Ftp != nil) ||
				(app.Spec.ALG.Sunrpc != nil) ||
				(app.Spec.ALG.Msrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
		default:
			ret = append(ret, fmt.Errorf("Invalid ALG type: %v", app.Spec.ALG.Type))
		}
	}
	return ret
}

func registerNetworkSecurityPolicyHooks(svc apiserver.Service, logger log.Logger) {
	r := securityHooks{
		svc:    svc,
		logger: logger.WithContext("Service", "SecurityV1"),
	}
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("NetworkSecurityPolicy", apiintf.CreateOper).GetRequestType().WithValidate(r.validateNetworkSecurityPolicy)
	svc.GetCrudService("NetworkSecurityPolicy", apiintf.CreateOper).WithPreCommitHook(r.enforceMaxNetworkSecurityPolicyPreCommitHook)
	svc.GetCrudService("NetworkSecurityPolicy", apiintf.CreateOper).WithPreCommitHook(r.enforceMaxSGRulePreCommitHook)
	svc.GetCrudService("NetworkSecurityPolicy", apiintf.UpdateOper).WithPreCommitHook(r.enforceMaxSGRulePreCommitHook)
	svc.GetCrudService("App", apiintf.CreateOper).GetRequestType().WithValidate(r.validateApp)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("security.SecurityV1", registerNetworkSecurityPolicyHooks)
}

func (s *securityHooks) isProtocolTCPorUDP(protocol string) bool {
	return strings.ToLower(protocol) == "6" || strings.ToLower(protocol) == "tcp" || strings.ToLower(protocol) == "17" || strings.ToLower(protocol) == "udp"
}

func (s *securityHooks) validateIANA(protocolNumber int) (err error) {
	switch protocolNumber {
	case 1:
		return
	case 6:
		return
	case 17:
		return
	default:
		err = fmt.Errorf("unsupported protocol number. Protocol: %d", protocolNumber)
		return
	}
}
