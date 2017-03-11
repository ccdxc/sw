package net

import (
	"net"
	"os"
	"strings"
)

// nameSliceFromMap return slice of DNS names from Map
func nameSliceFromMap(namesMap map[string]bool) []string {
	var names []string

	for k := range namesMap {
		names = append(names, k)
	}
	return names
}

//NameAndIPs returns slices of names and IP addresses of this host, including the FQDN
// names will have hostname and FQDN (if they are different).
// ips will have one or more if IPv4/IPv6 addresses which are result of DNS lookup for
// this hosts FQDN
func NameAndIPs() (names []string, ips []net.IP) {
	namesMap := make(map[string]bool)

	hostname, err := os.Hostname()
	if err != nil {
		return nil, nil
	}
	namesMap[strings.ToLower(hostname)] = true

	addrs, err := net.LookupIP(hostname)
	if err != nil {
		return nameSliceFromMap(namesMap), ips
	}

	for _, addr := range addrs {
		ip, err := addr.MarshalText()
		if err != nil {
			continue
		}
		hosts, err := net.LookupAddr(string(ip))
		if err != nil || len(hosts) == 0 {
			continue
		}
		name := strings.ToLower(strings.TrimSuffix(hosts[0], "."))
		namesMap[name] = true
		ips = append(ips, addr)
	}
	return nameSliceFromMap(namesMap), ips
}
