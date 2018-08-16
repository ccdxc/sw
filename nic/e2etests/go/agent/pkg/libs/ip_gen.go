package libs

import (
	"encoding/binary"
	"fmt"
	"math/rand"
	"net"
)

func GenSubnets(count int) (subnets []string) {
	// Fixed seed to generate repeatable IP Addresses
	rand.Seed(HEIMDALL_RANDOM_SEED)

	for len(subnets) != count {
		size := 4
		ip := make([]byte, size)
		for i := 0; i < size; i++ {
			ip[i] = byte(rand.Intn(256))
		}
		/* Ignore Experimental address to */
		if net.IP(ip).IsGlobalUnicast() && ip[0] <= 240 {
			cidr := fmt.Sprintf("%s/28", net.IP(ip).To4().String())
			subnets = append(subnets, cidr)
		}
	}
	return
}

func GenIPAddress(subnet string, numEp int) (ipAddrs []string, gw string, err error) {
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

func uint32ToIPv4(intIP uint32) net.IP {
	ip := make(net.IP, 4)
	binary.BigEndian.PutUint32(ip, intIP)
	return ip
}

func nextIP(ip net.IP) {
	for j := len(ip) - 1; j >= 0; j-- {
		ip[j]++
		if ip[j] > 0 {
			break
		}
	}
}
