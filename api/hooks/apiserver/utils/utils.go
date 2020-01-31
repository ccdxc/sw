package utils

import (
	"encoding/json"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/ctrler/tpm"
)

// ValidateFwLogPolicy validates policy, called from api-server for pre-commit
func ValidateFwLogPolicy(s *monitoring.FwlogPolicySpec) error {
	if _, ok := monitoring.MonitoringExportFormat_vvalue[s.Format]; !ok {
		return fmt.Errorf("invalid format %v", s.Format)
	}

	for _, f := range s.Filter {
		if _, ok := monitoring.FwlogFilter_vvalue[f]; !ok {
			return fmt.Errorf("invalid filter %v", f)
		}
	}

	if s.Config != nil {
		if _, ok := monitoring.SyslogFacility_vvalue[s.Config.FacilityOverride]; !ok {
			return fmt.Errorf("invalid facility override %v", s.Config.FacilityOverride)
		}

		if s.Config.Prefix != "" {
			return fmt.Errorf("prefix is not allowed in firewall log")
		}
	}

	if len(s.Targets) == 0 {
		return fmt.Errorf("no collectors configured")
	}

	if len(s.Targets) > tpm.MaxNumCollectorsPerPolicy {
		return fmt.Errorf("cannot configure more than %v collectors", tpm.MaxNumCollectorsPerPolicy)
	}

	collectors := map[string]bool{}
	for _, c := range s.Targets {
		if key, err := json.Marshal(c); err == nil {
			ks := string(key)
			if _, ok := collectors[ks]; ok {
				return fmt.Errorf("found duplicate collector %v %v", c.Destination, c.Transport)
			}
			collectors[ks] = true

		}

		if c.Destination == "" {
			return fmt.Errorf("cannot configure empty collector")
		}

		netIP, _, err := net.ParseCIDR(c.Destination)
		if err != nil {
			netIP = net.ParseIP(c.Destination)
		}

		if netIP == nil {
			// treat it as hostname and resolve
			if _, err := net.LookupHost(c.Destination); err != nil {
				return fmt.Errorf("failed to resolve name %s, error: %v", c.Destination, err)
			}
		}

		tr := strings.Split(c.Transport, "/")
		if len(tr) != 2 {
			return fmt.Errorf("transport should be in protocol/port format")
		}

		if _, ok := map[string]bool{
			"tcp": true,
			"udp": true,
		}[strings.ToLower(tr[0])]; !ok {
			return fmt.Errorf("invalid protocol %v\n Accepted protocols: TCP, UDP", tr[0])
		}

		port, err := strconv.Atoi(tr[1])
		if err != nil {
			return fmt.Errorf("invalid port %v", tr[1])
		}

		if uint(port) > uint(^uint16(0)) {
			return fmt.Errorf("invalid port %v (> %d)", port, ^uint16(0))
		}
	}

	return nil
}

//returns ipaddr type, is Range (only last byte is allowed to have range), is ip prefix
func getIPAddrDetails(ipAddr string) (bool, bool, bool) {
	return isIpv4(ipAddr), isRangeAddr(ipAddr), isSubnetAddr(ipAddr)
}

func isIpv4(ip string) bool {
	for i := 0; i < len(ip); i++ {
		switch ip[i] {
		case '.':
			return true
		case ':':
			return false
		}
	}
	return false
}
func isRangeAddr(ip string) bool {
	for i := 0; i < len(ip); i++ {
		switch ip[i] {
		case '-':
			return true
		}
	}
	return false
}
func isSubnetAddr(ip string) bool {
	for i := 0; i < len(ip); i++ {
		switch ip[i] {
		case '/':
			return true
		}
	}
	return false
}

// ValidateMatchRules validates match rules
func ValidateMatchRules(objMeta api.ObjectMeta, matchRules []netproto.MatchRule, findEndpoint func(api.ObjectMeta) (*netproto.Endpoint, error)) error {
	for _, rule := range matchRules {
		srcSelectors := rule.Src
		destSelectors := rule.Dst

		if srcSelectors != nil {

			if len(srcSelectors.Addresses) > 0 {
				for _, ipAddr := range srcSelectors.Addresses {
					if ipAddr == "any" {
						continue
					}
					_, isRange, isSubnet := getIPAddrDetails(ipAddr)
					if !isRange {
						if !isSubnet {
							if net.ParseIP(ipAddr) == nil {
								return fmt.Errorf("invalid ip address %v", ipAddr)
							}
						} else {
							_, _, err := net.ParseCIDR(ipAddr)
							if err != nil {
								return fmt.Errorf("invalid ip address %v", ipAddr)
							}
						}
					} else {
						// TODO: Handle IPaddr range
					}
				}
			}

		}
		if destSelectors != nil {

			if len(destSelectors.Addresses) > 0 {
				for _, ipAddr := range destSelectors.Addresses {
					if ipAddr == "any" {
						continue
					}
					_, isRange, isSubnet := getIPAddrDetails(ipAddr)
					if !isRange {
						if !isSubnet {
							if net.ParseIP(ipAddr) == nil {
								return fmt.Errorf("invalid ip address %v", ipAddr)
							}
						} else {
							_, _, err := net.ParseCIDR(ipAddr)
							if err != nil {
								return fmt.Errorf("invalid ip address %v", ipAddr)
							}
						}
					} else {
						// TODO: Handle IPaddr range
					}
				}
			}

		}
	}
	// TODO need to check if both srcIP and destIP are of same type (either both v4 or both v6)
	return nil
}
