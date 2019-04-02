package impl

import (
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
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
// func (s *securityHooks) validateSGPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
func (s *securityHooks) validateSGPolicy(i interface{}, ver string, ignoreStatus bool) []error {
	sgp, ok := i.(security.SGPolicy)
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
			err := s.validateIPAddresses(r.FromIPAddresses)
			if err != nil {
				ret = append(ret, fmt.Errorf("could not validate one or more ip address in %v. Error: %v", r.FromIPAddresses, err))
			}

			err = s.validateIPAddresses(r.ToIPAddresses)
			if err != nil {
				ret = append(ret, fmt.Errorf("could not validate one or more ip addresses in %v. Error: %v", r.ToIPAddresses, err))
			}
		} else {
			err := s.validateIPAddresses(r.ToIPAddresses)
			if err != nil {
				ret = append(ret, fmt.Errorf("could not validate one or more ip addresses in to-ip-addresses %v. Error: %v", r.ToIPAddresses, err))
			}
			err = s.validateIPAddresses(r.FromIPAddresses)
			if err != nil {
				ret = append(ret, fmt.Errorf("could not validate one or more ip addresses in from-ip-addresses %v. Error: %v", r.FromIPAddresses, err))
			}
		}
	}

	return ret
}

// validateProtoPort will enforce a valid proto/port declaration.
// references to the named apps will be handled by the controller.
func (s *securityHooks) validateProtoPort(rules []security.SGRule) error {
	for _, r := range rules {
		// verify a rule cant have both proto port and app
		if len(r.Apps) != 0 && len(r.ProtoPorts) != 0 {
			return fmt.Errorf("Can not have both app and protocol/port in rule: %v", r)
		}
		// error out on a rule which is missing proto-ports and a valid app
		if len(r.ProtoPorts) == 0 && len(r.Apps) == 0 {
			return fmt.Errorf("proto-ports are mandatory, when the rules don't refer to an App")
		}
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
					if p == strings.ToLower(pp.Protocol) {
						found = true
						break
					}
				}

				if !found {
					return fmt.Errorf("invalid protocol %v in SGRule: %v", pp.Protocol, r)
				}
			}

			if len(pp.Ports) != 0 {
				// you can not specify ports for icmp
				if strings.ToLower(pp.Protocol) == "icmp" {
					return fmt.Errorf("Can not specify ports for ICMP protocol in Rule: %v", r)
				}

				// parse port ranges
				portRanges := strings.Split(pp.Ports, ",")
				for _, prange := range portRanges {
					ports := strings.Split(prange, "-")
					for _, port := range ports {
						i, err := strconv.Atoi(port)
						if err != nil {
							return fmt.Errorf("port %v must be an integer value in the SGRule: %v", port, r)
						}
						if i < 0 || i > 65535 {
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
			} else if s.isProtocolTCPorUDP(pp.Protocol) {
				// Reject empty ports only on non icmp protocol
				return fmt.Errorf("ports are mandatory. Use 0-65535 if the intent is to allow all")
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
func (s *securityHooks) validateApp(in interface{}, ver string, ignoreStatus bool) []error {
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

	appProtos := []string{}
	for _, pp := range app.Spec.ProtoPorts {
		protoNum, err := strconv.Atoi(pp.Protocol)
		if err == nil {
			// protocol number specified, check its between 0-255 range (0 is valid proto)
			if protoNum < 0 || protoNum >= 255 {
				ret = append(ret, fmt.Errorf("Invalid protocol number %v", pp.Protocol))
			}
		} else {
			found := false
			for _, p := range supportedProtocols {
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
					if first > second {
						ret = append(ret, fmt.Errorf("Invalid port range %v. first number bigger than second", prange))
					}
				} else if len(ports) > 2 {
					ret = append(ret, fmt.Errorf("Invalid port range format: %v", prange))
				}
			}
		}
	}

	// validate ALG types
	if app.Spec.ALG != nil && app.Spec.ALG.Type != "" {

		switch app.Spec.ALG.Type {
		case "ICMP":
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
		case "DNS":
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Ftp != nil) ||
				(app.Spec.ALG.Sunrpc != nil) ||
				(app.Spec.ALG.Msrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
			if !protocolNameValidate(appProtos, "udp", "mustMatchAll") {
				ret = append(ret, fmt.Errorf("Protocol(s) %v is not allowed with DNS ALG", appProtos))
			}
		case "FTP":
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Dns != nil) ||
				(app.Spec.ALG.Sunrpc != nil) ||
				(app.Spec.ALG.Msrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
		case "SunRPC":
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Dns != nil) ||
				(app.Spec.ALG.Ftp != nil) ||
				(app.Spec.ALG.Msrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
		case "MSRPC":
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Dns != nil) ||
				(app.Spec.ALG.Ftp != nil) ||
				(app.Spec.ALG.Sunrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
		case "TFTP":
			if (app.Spec.ALG.Icmp != nil) ||
				(app.Spec.ALG.Dns != nil) ||
				(app.Spec.ALG.Ftp != nil) ||
				(app.Spec.ALG.Sunrpc != nil) ||
				(app.Spec.ALG.Msrpc != nil) {
				ret = append(ret, fmt.Errorf("Only %v params can be specified for ALG type: %v", app.Spec.ALG.Type, app.Spec.ALG.Type))
			}
		case "RTSP":
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

func registerSGPolicyHooks(svc apiserver.Service, logger log.Logger) {
	r := securityHooks{
		svc:    svc,
		logger: logger.WithContext("Service", "SecurityV1"),
	}
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("SGPolicy", apiintf.CreateOper).GetRequestType().WithValidate(r.validateSGPolicy)
	svc.GetCrudService("App", apiintf.CreateOper).GetRequestType().WithValidate(r.validateApp)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("security.SecurityV1", registerSGPolicyHooks)
}

func (s *securityHooks) isProtocolTCPorUDP(protocol string) bool {
	return strings.ToLower(protocol) == "6" || strings.ToLower(protocol) == "tcp" || strings.ToLower(protocol) == "17" || strings.ToLower(protocol) == "udp"
}
