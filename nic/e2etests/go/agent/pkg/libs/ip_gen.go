package libs

import (
	"fmt"
	"math/rand"
	"net"

	log "github.com/sirupsen/logrus"
)

func GenSubnetsFixed(count int) (subnets []string) {
	// Fixed seed to generate repeatable IP Addresses
	rand.Seed(HEIMDALL_RANDOM_SEED)

	for len(subnets) != count {
		size := 4
		ip := make([]byte, size)
		ip[0] = 10
		for i := 1; i < size; i++ {
			ip[i] = byte(rand.Intn(255))
		}
		/* Ignore Experimental address to */
		if net.IP(ip).IsGlobalUnicast() && ip[0] <= 240 {
			cidr := fmt.Sprintf("%s/24", net.IP(ip).To4().String())
			subnets = append(subnets, cidr)
		}
	}
	return
}

func GenIPAddress(subnet string, numEp int, generateAll bool) (ipAddrs []string, gw string, err error) {
	ip, ipNet, err := net.ParseCIDR(subnet)
	if err != nil {
		return
	}

	for ip := ip.Mask(ipNet.Mask); ipNet.Contains(ip); nextIP(ip) {
		ipAddrs = append(ipAddrs, ip.String())
	}

	// validate if we have enough valid IPs. It should be atleast 3 more than the requested number.
	// 3 because, one is the network, one for broadcast and one for the gateway
	if len(ipAddrs) < numEp+3 {
		ipAddrs = nil
		err = ErrInsufficientIPAddressGenerated
		return
	}

	// Remove the network and broadcast addresses
	ipAddrs = ipAddrs[1 : len(ipAddrs)-1]

	// Assign the first valid IP as gw
	gw = ipAddrs[0]

	// If generate all, return all the IPs in a given subnet
	if generateAll {
		return
	}
	// Assign remaining IPs to be used as endpoints
	ipAddrs = ipAddrs[1 : numEp+1]

	// One final validation
	if len(ipAddrs) != numEp {
		ipAddrs = nil
		gw = ""
		err = ErrInsufficientIPAddressGenerated
	}

	return

}

// GenSubnets generates non overlapping subnets from a valid starting cidr block
func GenSubnets(networkStart string, count int) ([]string, error) {
	var subnets []string
	baseIP, baseNet, _ := net.ParseCIDR(networkStart)
	if baseNet.IP.Equal(net.IPv4zero) {
		log.Errorf("invalid network start: %v", networkStart)
		return nil, fmt.Errorf("invalid network start. %v", networkStart)
	}
	subnets = append(subnets, baseNet.String())
	for baseIP := baseIP.Mask(baseNet.Mask); len(subnets) != count; {
		nextIP(baseIP)
		prev := subnets[len(subnets)-1]
		_, prevSubnet, _ := net.ParseCIDR(prev)
		if !prevSubnet.Contains(baseIP) {
			nextSubnet := net.IPNet{
				IP:   baseIP,
				Mask: baseNet.Mask,
			}
			subnets = append(subnets, nextSubnet.String())
		}

	}

	return subnets, nil
}

//
//func uint32ToIPv4(intIP uint32) net.IP {
//	ip := make(net.IP, 4)
//	binary.BigEndian.PutUint32(ip, intIP)
//	return ip
//}

func nextIP(ip net.IP) {
	for j := len(ip) - 1; j >= 0; j-- {
		ip[j]++
		if ip[j] > 0 {
			break
		}
	}
}
