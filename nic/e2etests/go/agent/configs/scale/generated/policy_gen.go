package main

import (
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"math/rand"
	"net"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
)

var ErrInsufficientIPAddressGenerated = errors.New("could not generate the required number of IP Addresses")

const fixedSeed = 42

func main() {
	var (
		numRules = flag.Int("numRules", 0, "Number of rules in a single policy")
		policies []*netproto.SGPolicy
		rules    []netproto.PolicyRule
	)
	flag.Parse()
	if *numRules == 0 {
		log.Fatal("Specify the number of rules needed with a -numRules flag")
	}

	ipaddrs, _, err := GenIPAddress("10.0.0.0/8", *numRules-1)
	fileName := fmt.Sprintf("%d_rules.json", *numRules)

	rand.Seed(fixedSeed)
	for idx, ipAddr := range ipaddrs {

		var action, protocol, port string
		port = fmt.Sprintf("%d", rand.Intn(65535-1)+1)
		switch idx % 3 {
		case 0:
			action = "PERMIT"
			protocol = "tcp"
		case 1:
			action = "DENY"
			protocol = "udp"
		case 2:
			action = "PERMIT"
			protocol = "icmp"
		}
		appConfig := []*netproto.AppConfig{
			{
				Protocol: protocol,
				Port:     port,
			},
		}
		rule := netproto.PolicyRule{
			Action: action,
			Src: &netproto.MatchSelector{
				Addresses: []string{"any"},
			},
			Dst: &netproto.MatchSelector{
				Addresses:  []string{ipAddr},
				AppConfigs: appConfig,
			},
		}
		rules = append(rules, rule)
	}

	permitAllRule := netproto.PolicyRule{
		Action: "PERMIT",
	}

	rules = append(rules, permitAllRule)

	sgp := &netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "scale-rules",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}
	policies = append(policies, sgp)
	b, _ := json.MarshalIndent(policies, "", "   ")

	err = ioutil.WriteFile(fileName, b, 0644)
	if err != nil {
		log.Fatalf("Could not write the generated policy. Err: %v", err)
	}
	fmt.Printf("Succesfully generated SGPolicy with %d rules to %s\n", *numRules, fileName)
}

func GenIPAddress(subnet string, count int) (ipAddrs []string, gw string, err error) {
	ip, ipNet, err := net.ParseCIDR(subnet)
	if err != nil {
		return
	}

	for ip := ip.Mask(ipNet.Mask); ipNet.Contains(ip); nextIP(ip) {
		ipAddrs = append(ipAddrs, ip.String())
	}

	// validate if we have enough valid IPs. It should be atleast 3 more than the requested number.
	// 3 because, one is the network, one for broadcast and one for the gateway
	if len(ipAddrs) < count+3 {
		ipAddrs = nil
		err = ErrInsufficientIPAddressGenerated
		return
	}

	// Remove the network and broadcast addresses
	ipAddrs = ipAddrs[1 : len(ipAddrs)-1]

	// Assign the first valid IP as gw
	gw = ipAddrs[0]

	// Assign remaining IPs to be used as endpoints
	ipAddrs = ipAddrs[1 : count+1]

	// One final validation
	if len(ipAddrs) != count {
		ipAddrs = nil
		gw = ""
		err = ErrInsufficientIPAddressGenerated
	}

	return

}

func nextIP(ip net.IP) {
	for j := len(ip) - 1; j >= 0; j-- {
		ip[j]++
		if ip[j] > 0 {
			break
		}
	}
}
