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

type securityHooks struct {
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
func (s *securityHooks) validateSGPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
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
	err := s.validateProtoPort(sgp.Spec.Rules)
	if err != nil {
		return i, false, err
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

// validateProtoPort will enforce a valid proto/port declaration.
// references to the named apps will be handled by the controller.
func (s *securityHooks) validateProtoPort(rules []security.SGRule) error {
	for _, r := range rules {
		for _, pp := range r.ProtoPorts {
			protoNum, err := strconv.Atoi(pp.Protocol)
			if err == nil {
				// protocol number specified, check its between 0-255 range (0 is valid proto)
				if protoNum < 0 || protoNum >= 255 {
					return fmt.Errorf("Invalid protocol number %v in rule: %v", pp.Protocol, r)
				}
			} else {
				found := false
				for _, p := range supportedProtocols {
					if p == pp.Protocol {
						found = true
						break
					}
				}

				if !found {
					return fmt.Errorf("invalid protocol %v in SGRule: %v", pp.Protocol, r)
				}
			}

			if len(pp.Ports) != 0 {
				portRanges := strings.Split(pp.Ports, ",")
				for _, prange := range portRanges {
					ports := strings.Split(prange, "-")
					for _, port := range ports {
						i, err := strconv.Atoi(port)
						if err != nil {
							return fmt.Errorf("port %v must be an integer value in the SGRule: %v", port, r)
						}
						if 0 > i || i > 65535 {
							return fmt.Errorf("port %v outside range in rule: %v", port, r)
						}
					}
					if len(ports) == 2 {
						first, _ := strconv.Atoi(ports[0])
						second, _ := strconv.Atoi(ports[1])
						if first > second {
							return fmt.Errorf("Invalid port range %v. first number bigger than second", prange)
						}
					} else if len(ports) > 2 {
						return fmt.Errorf("Invalid port range format: %v", prange)
					}
				}
			}
		}
	}
	return nil
}

// validates the IP Addresses in SG rules. Each IP Address must either be a standard octet, ipmask, hyphen separated ip range, or must use any as a keyword
func (s *securityHooks) validateIPAddresses(addresses []string) error {
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

// validateApp validates contents of app
func (s *securityHooks) validateApp(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	app, ok := i.(security.App)
	if !ok {
		return i, false, fmt.Errorf("invalid input type")
	}

	for _, pp := range app.Spec.ProtoPorts {
		protoNum, err := strconv.Atoi(pp.Protocol)
		if err == nil {
			// protocol number specified, check its between 0-255 range (0 is valid proto)
			if protoNum < 0 || protoNum >= 255 {
				return i, false, fmt.Errorf("Invalid protocol number %v", pp.Protocol)
			}
		} else {
			found := false
			for _, p := range supportedProtocols {
				if p == pp.Protocol {
					found = true
					break
				}
			}

			if !found {
				return i, false, fmt.Errorf("invalid protocol %v", pp.Protocol)
			}
		}

		if len(pp.Ports) != 0 {
			portRanges := strings.Split(pp.Ports, ",")
			for _, prange := range portRanges {
				ports := strings.Split(prange, "-")
				for _, port := range ports {
					i, err := strconv.Atoi(port)
					if err != nil {
						return i, false, fmt.Errorf("port %v must be an integer value", port)
					}
					if 0 > i || i > 65535 {
						return i, false, fmt.Errorf("port %v outside range", port)
					}
				}
				if len(ports) == 2 {
					first, _ := strconv.Atoi(ports[0])
					second, _ := strconv.Atoi(ports[1])
					if first > second {
						return i, false, fmt.Errorf("Invalid port range %v. first number bigger than second", prange)
					}
				} else if len(ports) > 2 {
					return i, false, fmt.Errorf("Invalid port range format: %v", prange)
				}
			}
		}
	}

	// validate ALG types
	if app.Spec.ALG != nil && app.Spec.ALG.Type != "" {

		switch app.Spec.ALG.Type {
		case "ICMP":
			if (app.Spec.ALG.DnsAlg != nil) ||
				(app.Spec.ALG.FtpAlg != nil) ||
				(app.Spec.ALG.SunrpcAlg != nil) ||
				(app.Spec.ALG.MsrpcAlg != nil) {
				return i, false, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type)
			}

			// validate ICMP type
			if app.Spec.ALG.IcmpAlg.Type != "" {
				i, err := strconv.Atoi(app.Spec.ALG.IcmpAlg.Type)
				if err != nil {
					return i, false, fmt.Errorf("ICMP Type %v must be an integer value", app.Spec.ALG.IcmpAlg.Type)
				}
				if 0 > i || i > 255 {
					return i, false, fmt.Errorf("ICMP Type %v outside range", app.Spec.ALG.IcmpAlg.Type)
				}
			}
			if app.Spec.ALG.IcmpAlg.Code != "" {
				i, err := strconv.Atoi(app.Spec.ALG.IcmpAlg.Code)
				if err != nil {
					return i, false, fmt.Errorf("ICMP Code %v must be an integer value", app.Spec.ALG.IcmpAlg.Code)
				}
				if 0 > i || i > 18 {
					return i, false, fmt.Errorf("ICMP Code %v outside range", app.Spec.ALG.IcmpAlg.Code)
				}
			}
		case "DNS":
			if (app.Spec.ALG.IcmpAlg != nil) ||
				(app.Spec.ALG.FtpAlg != nil) ||
				(app.Spec.ALG.SunrpcAlg != nil) ||
				(app.Spec.ALG.MsrpcAlg != nil) {
				return i, false, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type)
			}
		case "FTP":
			if (app.Spec.ALG.IcmpAlg != nil) ||
				(app.Spec.ALG.DnsAlg != nil) ||
				(app.Spec.ALG.SunrpcAlg != nil) ||
				(app.Spec.ALG.MsrpcAlg != nil) {
				return i, false, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type)
			}
		case "SunRPC":
			if (app.Spec.ALG.IcmpAlg != nil) ||
				(app.Spec.ALG.DnsAlg != nil) ||
				(app.Spec.ALG.FtpAlg != nil) ||
				(app.Spec.ALG.MsrpcAlg != nil) {
				return i, false, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type)
			}
		case "MSRPC":
			if (app.Spec.ALG.IcmpAlg != nil) ||
				(app.Spec.ALG.DnsAlg != nil) ||
				(app.Spec.ALG.FtpAlg != nil) ||
				(app.Spec.ALG.SunrpcAlg != nil) {
				return i, false, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type)
			}
		case "TFTP":
			if (app.Spec.ALG.IcmpAlg != nil) ||
				(app.Spec.ALG.DnsAlg != nil) ||
				(app.Spec.ALG.FtpAlg != nil) ||
				(app.Spec.ALG.SunrpcAlg != nil) ||
				(app.Spec.ALG.MsrpcAlg != nil) {
				return i, false, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type)
			}
		case "RTSP":
			if (app.Spec.ALG.IcmpAlg != nil) ||
				(app.Spec.ALG.DnsAlg != nil) ||
				(app.Spec.ALG.FtpAlg != nil) ||
				(app.Spec.ALG.SunrpcAlg != nil) ||
				(app.Spec.ALG.MsrpcAlg != nil) {
				return i, false, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type)
			}
		default:
			return i, false, fmt.Errorf("Invalid ALG type: %v", app.Spec.ALG.Type)
		}
	}
	return i, true, nil
}

func registerSGPolicyHooks(svc apiserver.Service, logger log.Logger) {
	r := securityHooks{
		svc:    svc,
		logger: logger.WithContext("Service", "SecurityV1"),
	}
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("SGPolicy", apiserver.CreateOper).WithPreCommitHook(r.validateSGPolicy)
	svc.GetCrudService("SGPolicy", apiserver.UpdateOper).WithPreCommitHook(r.validateSGPolicy)
	svc.GetCrudService("App", apiserver.CreateOper).WithPreCommitHook(r.validateApp)
	svc.GetCrudService("App", apiserver.UpdateOper).WithPreCommitHook(r.validateApp)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("security.SecurityV1", registerSGPolicyHooks)
}
